/*****************************************************************************/
/* D2K: A Doom Source Port for the 21st Century                              */
/*                                                                           */
/* Copyright (C) 2014: See COPYRIGHT file                                    */
/*                                                                           */
/* This file is part of D2K.                                                 */
/*                                                                           */
/* D2K is free software: you can redistribute it and/or modify it under the  */
/* terms of the GNU General Public License as published by the Free Software */
/* Foundation, either version 2 of the License, or (at your option) any      */
/* later version.                                                            */
/*                                                                           */
/* D2K is distributed in the hope that it will be useful, but WITHOUT ANY    */
/* WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS */
/* FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more    */
/* details.                                                                  */
/*                                                                           */
/* You should have received a copy of the GNU General Public License along   */
/* with D2K.  If not, see <http://www.gnu.org/licenses/>.                    */
/*                                                                           */
/*****************************************************************************/


#include "z_zone.h"

#include <enet/enet.h>

#include "doomdef.h"
#include "d_event.h"
#include "p_user.h"
#include "g_game.h"
#include "n_net.h"
#include "n_main.h"
#include "n_state.h"
#include "n_peer.h"
#include "n_proto.h"
#include "cl_cmd.h"
#include "cl_main.h"
#include "r_defs.h"
#include "r_fps.h"
#include "r_state.h"
#include "s_sound.h"

/*
 * Marked true when the client has received the setup message from the server
 */
static bool cl_received_setup = false;

/*
 * Set when the client receives an authorization message from the server
 * NOTE: Currently unimplemented.
 */
static auth_level_e cl_authorization_level = AUTH_LEVEL_NONE;

/*
 * The current command index, used for generating new commands
 */
static int cl_local_command_index = 1;

/*
 * Index of the currently running command
 */
static int cl_current_command_index = 1;

/*
 * Marked true when the client is loading a state
 * NOTE: Currently used to avoid stopping orphaned sounds
 */
static bool cl_loading_state = false;

/*
 * Marked true when the client is running consoleplayer's commands
 */
static bool cl_running_consoleplayer_commands = false;

/*
 * Marked true when the client is running commands for a non-consoleplayer
 * player
 */
static bool cl_running_nonconsoleplayer_commands = false;

/*
 * Marked true when the client is running thinkers and specials
 */
static bool cl_running_thinkers = false;

/*
 * Marked true when the client is synchronizing
 */
static bool cl_synchronizing = false;

/*
 * Marked true when the client is re-predicting
 */
static bool cl_repredicting = false;

/*
 * The TIC at which reprediction started.
 */
static int cl_repredicting_start_tic = 0;

/*
 * The TIC at which reprediction will finish.
 */
static int cl_repredicting_end_tic = 0;

/*
 * The TIC of the last state received from the server
 */
static int cl_state_tic = -1;

/*
 * The start TIC of the latest delta received
 */
static int cl_delta_from_tic = -1;

/*
 * The end TIC of the latest delta received
 */
static int cl_delta_to_tic = -1;

int cl_extrapolate_player_positions = false;

static bool cl_load_new_state(netpeer_t *server) {
  game_state_delta_t *delta = &server->sync.delta;
  bool state_loaded;

  if (!N_ApplyStateDelta(delta)) {
    P_Echo(consoleplayer, "Error applying state delta");
    return false;
  }

  cl_loading_state = true;
  state_loaded = N_LoadLatestState(false);
  cl_loading_state = false;

  if (!state_loaded) {
    P_Echo(consoleplayer, "Error loading latest state");
    return false;
  }

  cl_loading_state = true;
  state_loaded = N_LoadState(delta->from_tic, false);
  cl_loading_state = false;

  if (!state_loaded) {
    P_Echo(consoleplayer, "Error loading previous state");
    return false;
  }

  for (int i = 0; i < MAXPLAYERS; i++) {
    player_t *player;
    
    if (!playeringame[i]) {
      continue;
    }

    player = &players[i];

    if (player->cmdq.commands->len) {
      D_Msg(MSG_SYNC, "(%d) (%d) [ ", gametic, i);

      for (unsigned int i = 0; i < player->cmdq.commands->len; i++) {
        netticcmd_t *ncmd = g_ptr_array_index(player->cmdq.commands, i);

        D_Msg(MSG_SYNC, "%d/%d/%d ", ncmd->index, ncmd->tic, ncmd->server_tic);
      }

      D_Msg(MSG_SYNC, "]\n");
    }
  }

  cl_synchronizing = true;
  D_Msg(MSG_SYNC, "Synchronizing %d => %d\n", gametic, server->sync.tic);
  R_ResetViewInterpolation();
  for (int i = gametic; i <= server->sync.tic; i++) {
    N_RunTic();
    if (players[displayplayer].mo != NULL) {
      R_InterpolateView(&players[displayplayer]);
      R_RestoreInterpolations();
    }
  }
  cl_synchronizing = false;

  if (gametic != server->sync.tic + 1) {
    D_Msg(MSG_WARN, "Synchronization incomplete: %d, %d\n",
      gametic, server->sync.tic
    );
  }

  D_Msg(MSG_CMD, "Ran sync'd commands, loading latest state\n");

  cl_loading_state = true;
  state_loaded = N_LoadLatestState(false);
  cl_loading_state = false;

  if (!state_loaded) {
    P_Echo(consoleplayer, "Error loading latest state");
    return false;
  }

  R_UpdateInterpolations();

  N_RemoveOldStates(delta->from_tic);

  return true;
}

static void cl_set_repredicting(int start_tic, int end_tic) {
  cl_repredicting_start_tic = start_tic;
  cl_repredicting_end_tic = end_tic;
  cl_repredicting = true;
}

static void cl_clear_repredicting(void) {
  cl_repredicting_start_tic = 0;
  cl_repredicting_end_tic = 0;
  cl_repredicting = false;
}

static void cl_repredict(int saved_gametic) {
  player_t *player = &players[consoleplayer];
  unsigned int latest_command_index = P_GetLatestCommandIndex(consoleplayer);
  
  if (gametic == -1) {
    return;
  }

  if (latest_command_index == 0) {
    return;
  }

  cl_set_repredicting(
    player->cmdq.latest_command_run_index, latest_command_index
  );
  D_Msg(MSG_SYNC, "Re-predicting %u => %u (%d)\n",
    player->cmdq.latest_command_run_index,
    latest_command_index,
    latest_command_index - player->cmdq.latest_command_run_index
  );
  while (player->cmdq.latest_command_run_index < latest_command_index) {
    N_RunTic();
    if (players[displayplayer].mo != NULL) {
      R_InterpolateView(&players[displayplayer]);
      R_RestoreInterpolations();
    }
  }
  cl_clear_repredicting();
}

bool CL_OccurredDuringRePrediction(int tic) {
  if (!cl_repredicting) {
    return false;
  }

  if (tic < cl_repredicting_start_tic) {
    return false;
  }

  if (tic > cl_repredicting_end_tic) {
    return false;
  }

  return true;
}

void CL_CheckForStateUpdates(void) {
  netpeer_t *server;
  bool state_loaded;
  int saved_gametic = gametic;

  if (!CLIENT)
    return;

  server = CL_GetServerPeer();

  if (server == NULL) {
    P_Echo(consoleplayer, "Server disconnected");
    N_Disconnect();
    return;
  }

  if (server->sync.tic == cl_state_tic) {
    return;
  }

  D_Msg(MSG_SYNC, "(%d) Loading new state [%d, %d => %d] (%d)\n",
    gametic,
    server->sync.command_index,
    server->sync.delta.from_tic,
    server->sync.delta.to_tic,
    players[consoleplayer].cmdq.latest_command_run_index
  );

  S_ResetSoundLog();

  state_loaded = cl_load_new_state(server);

  if (!state_loaded) {
    server->sync.tic = cl_state_tic;
    server->sync.delta.from_tic = cl_delta_from_tic;
    server->sync.delta.to_tic = cl_delta_to_tic;
    return;
  }

  gametic++;

  CL_TrimSynchronizedCommands();

  N_LogPlayerPosition(&players[consoleplayer]);

#ifdef LOG_SECTOR
  if (LOG_SECTOR < numsectors) {
    if (sectors[LOG_SECTOR].floorheight != (168 << FRACBITS) &&
        sectors[LOG_SECTOR].floorheight != (40 << FRACBITS)) {
      D_Msg(MSG_SYNC, "(%d) Sector %d: %d/%d\n",
        gametic,
        LOG_SECTOR,
        sectors[LOG_SECTOR].floorheight >> FRACBITS,
        sectors[LOG_SECTOR].ceilingheight >> FRACBITS
      );
    }
  }
#endif

  if (G_GetGameState() == GS_LEVEL) {
    cl_repredict(saved_gametic);
  }

  cl_state_tic = server->sync.tic;
  cl_delta_from_tic = server->sync.delta.from_tic;
  cl_delta_to_tic = server->sync.delta.to_tic;

  server->sync.outdated = true;

  S_TrimSoundLog(cl_delta_from_tic, server->sync.command_index);
}

bool CL_LoadingState(void) {
  return cl_loading_state;
}

bool CL_Synchronizing(void) {
  return (CLIENT && cl_synchronizing);
}

bool CL_RePredicting(void) {
  return (CLIENT && cl_repredicting);
}

bool CL_Predicting(void) {
  return (CLIENT && !(cl_synchronizing || cl_repredicting));
}

bool CL_RunningConsoleplayerCommands(void) {
  return cl_running_consoleplayer_commands;
}

bool CL_RunningNonConsoleplayerCommands(void) {
  return cl_running_nonconsoleplayer_commands;
}

void CL_SetRunningThinkers(bool running) {
  cl_running_thinkers = running;
}

bool CL_RunningThinkers(void) {
  return cl_running_thinkers;
}

void CL_SetupCommandState(int playernum, netticcmd_t *ncmd) {
  if (!CLIENT)
    return;

  if (playernum == consoleplayer) {
    cl_running_consoleplayer_commands = true;
    cl_running_nonconsoleplayer_commands = false;
  }
  else {
    cl_running_consoleplayer_commands = false;
    cl_running_nonconsoleplayer_commands = true;
  }

  if (cl_running_consoleplayer_commands)
    cl_current_command_index = ncmd->index;
}

void CL_ShutdownCommandState(void) {
  cl_running_consoleplayer_commands = false;
  cl_running_nonconsoleplayer_commands = false;
}

int CL_GetCurrentCommandIndex(void) {
  return cl_current_command_index;
}

int CL_GetNextCommandIndex(void) {
  int out = cl_local_command_index;

  cl_local_command_index++;

  return out;
}

bool CL_ReceivedSetup(void) {
  return cl_received_setup;
}

void CL_SetReceivedSetup(bool new_received_setup) {
  cl_received_setup = new_received_setup;
}

void CL_SetAuthorizationLevel(auth_level_e level) {
  if (level > cl_authorization_level)
    cl_authorization_level = level;
}

void CL_MarkServerOutdated(void) {
  netpeer_t *server;

  if (!CLIENT)
    return;
  
  server = CL_GetServerPeer();

  if (server != NULL)
    server->sync.outdated = true;

  N_UpdateSync();
}

void CL_UpdateReceivedCommandIndex(int command_index) {
  netpeer_t *server = CL_GetServerPeer();

  if (server == NULL)
    return;

  server->sync.command_index = MAX(server->sync.command_index, command_index);
}

int CL_StateTIC(void) {
  return cl_state_tic;
}

void CL_Init(void) {
}

/* vi: set et ts=2 sw=2: */

