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
#include "d_ticcmd.h"
#include "d_player.h"
#include "n_net.h"
#include "n_main.h"
#include "n_state.h"
#include "n_peer.h"
#include "n_proto.h"
#include "cl_cmd.h"
#include "cl_main.h"
#include "p_cmd.h"
#include "p_user.h"
#include "r_fps.h"
#include "r_state.h"
#include "s_sound.h"

// #define LOG_SECTOR 43

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
 * Marked true when the client is synchronizing
 */
static bool cl_synchronizing = false;

/*
 * Marked true when the client is re-predicting
 */
static bool cl_repredicting = false;

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

  // cl_current_command_index = start_command_index;

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

  cl_synchronizing = true;
  while (gametic <= server->sync.tic) {
    N_RunTic();
    if (players[displayplayer].mo != NULL) {
      R_InterpolateView(&players[displayplayer]);
      R_RestoreInterpolations();
    }
  }
  cl_synchronizing = false;

  D_Log(LOG_SYNC, "Ran sync'd commands, loading latest state\n");

  cl_loading_state = true;
  state_loaded = N_LoadLatestState(false);
  cl_loading_state = false;

  if (!state_loaded) {
    P_Echo(consoleplayer, "Error loading latest state");
    return false;
  }

  N_RemoveOldStates(delta->from_tic);
  return true;
}

static void cl_predict(int saved_gametic) {
  int latest_command_index;
  
  if (gametic == -1)
    return;

  latest_command_index = P_GetLatestCommandIndex(consoleplayer);

  if (latest_command_index == -1)
    return;

  while (players[consoleplayer].latest_command_run_index <
         latest_command_index) {
    cl_repredicting = true;
    N_RunTic();
    if (players[displayplayer].mo != NULL) {
      R_InterpolateView(&players[displayplayer]);
      R_RestoreInterpolations();
    }
    cl_repredicting = false;
  }
}

void CL_CheckForStateUpdates(void) {
  netpeer_t *server;
  bool state_loaded;
  fixed_t saved_prev_viewz = players[displayplayer].prev_viewz;
  fixed_t saved_viewz = players[displayplayer].viewz;
  int saved_gametic = gametic;

  if (!DELTACLIENT)
    return;

  server = CL_GetServerPeer();

  if (server == NULL) {
    P_Echo(consoleplayer, "Server disconnected");
    N_Disconnect();
    return;
  }

  if (server->sync.tic == cl_state_tic)
    return;

  D_Log(LOG_SYNC, "(%d) Loading new state [%d, %d => %d] (%d)\n",
    gametic,
    server->sync.command_index,
    server->sync.delta.from_tic,
    server->sync.delta.to_tic,
    players[consoleplayer].latest_command_run_index
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

  CL_ClearSynchronizedCommands(consoleplayer);

  N_LogPlayerPosition(&players[consoleplayer]);

  D_Log(LOG_SYNC, "(%d) Loaded new state [%d, %d => %d] (%d)\n",
    gametic,
    server->sync.command_index,
    server->sync.delta.from_tic,
    server->sync.delta.to_tic,
    players[consoleplayer].latest_command_run_index
  );

#ifdef LOG_SECTOR
  if (LOG_SECTOR < numsectors) {
    if (sectors[LOG_SECTOR].floorheight != (168 << FRACBITS) &&
        sectors[LOG_SECTOR].floorheight != (40 << FRACBITS)) {
      D_Log(LOG_SYNC, "(%d) Sector %d: %d/%d\n",
        gametic,
        LOG_SECTOR,
        sectors[LOG_SECTOR].floorheight >> FRACBITS,
        sectors[LOG_SECTOR].ceilingheight >> FRACBITS
      );
      printf("(%d) Sector %d: %d/%d\n",
        gametic,
        LOG_SECTOR,
        sectors[LOG_SECTOR].floorheight >> FRACBITS,
        sectors[LOG_SECTOR].ceilingheight >> FRACBITS
      );
    }
  }
#endif
  cl_predict(saved_gametic);

  cl_state_tic = server->sync.tic;
  cl_delta_from_tic = server->sync.delta.from_tic;
  cl_delta_to_tic = server->sync.delta.to_tic;

  players[displayplayer].prev_viewz = saved_prev_viewz;
  players[displayplayer].viewz = saved_viewz;

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

void CL_Init(void) {
}

/* vi: set et ts=2 sw=2: */

