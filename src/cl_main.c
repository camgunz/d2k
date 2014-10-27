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
#include "p_cmd.h"
#include "p_user.h"
#include "r_fps.h"

#define LOAD_PREVIOUS_STATE 1
#define PREDICT_LOST_TICS 1
#define USE_NEW_PREDICTION 1
#define PREDICT_IN_ONE_TIC 0

static bool         cl_received_setup = false;
static auth_level_e cl_authorization_level = AUTH_LEVEL_NONE;
static bool         cl_loading_state = false;
static int          cl_local_command_index = 0;
static int          cl_current_command_index = 0;
static int          cl_latest_command_run = 0;
static bool         cl_running_consoleplayer_commands = false;
static bool         cl_running_nonconsoleplayer_commands = false;
static int          cl_state_tic = -1;
static int          cl_synchronized_command_index = -1;
static int          cl_delta_from_tic = -1;
static int          cl_delta_to_tic = -1;
static bool         cl_synchronizing = false;
static bool         cl_repredicting = false;

static bool cl_load_new_state(netpeer_t *server,
                              int previous_synchronized_command_index,
                              int latest_synchronized_command_index,
                              GQueue *sync_commands,
                              GQueue *run_commands) {
  game_state_delta_t *delta = &server->sync.delta;
  unsigned int sync_command_count = g_queue_get_length(sync_commands);
  bool state_loaded;

  D_Log(LOG_SYNC, "(%d) Loading new state [%d => %d] [%d => %d]\n",
    gametic,
    delta->from_tic,
    delta->to_tic,
    previous_synchronized_command_index,
    latest_synchronized_command_index
  );

  P_PrintCommands(sync_commands);

  cl_current_command_index = previous_synchronized_command_index;

#if LOAD_PREVIOUS_STATE
  cl_loading_state = true;
  state_loaded = N_LoadLatestState(false);
  cl_loading_state = false;

  if (!state_loaded) {
    P_Echo(consoleplayer, "Error loading previous state");
    server->sync.tic = cl_state_tic;
    delta->from_tic = cl_delta_from_tic;
    delta->to_tic = cl_delta_to_tic;
    return false;
  }
#endif

  if (!N_ApplyStateDelta(delta)) {
    P_Echo(consoleplayer, "Error applying state delta");
    server->sync.tic = cl_state_tic;
    delta->from_tic = cl_delta_from_tic;
    delta->to_tic = cl_delta_to_tic;
    return false;
  }

#if LOAD_PREVIOUS_STATE
  for (unsigned int i = 0; i < sync_command_count; i++) {
    netticcmd_t *sync_ncmd = g_queue_peek_nth(sync_commands, i);

    if (sync_ncmd->index > previous_synchronized_command_index &&
        sync_ncmd->index <= latest_synchronized_command_index) {
      netticcmd_t *run_ncmd = P_GetNewBlankCommand();

      memcpy(run_ncmd, sync_ncmd, sizeof(netticcmd_t));
      g_queue_push_tail(run_commands, run_ncmd);
    }
  }

  D_Log(LOG_SYNC, "Loaded previous state, running %d sync'd commands\n",
    latest_synchronized_command_index - previous_synchronized_command_index
  );

  cl_synchronizing = true;
  while (gametic < server->sync.tic) {
    N_RunTic();
    if (players[displayplayer].mo != NULL) {
      R_InterpolateView(&players[displayplayer]);
      R_RestoreInterpolations();
    }
  }
  cl_synchronizing = false;
#endif

  D_Log(LOG_SYNC, "Ran sync'd commands, loading latest state\n");

  cl_loading_state = true;
  state_loaded = N_LoadLatestState(false);
  cl_loading_state = false;

  if (!state_loaded)
    P_Echo(consoleplayer, "Error loading latest state");
  else
    N_RemoveOldStates(delta->from_tic);

  return state_loaded;
}

static void cl_predict(int saved_gametic,
                       int latest_synchronized_command_index,
                       GQueue *sync_commands,
                       GQueue *run_commands) {
  int command_index = latest_synchronized_command_index;
  unsigned int sync_command_count = g_queue_get_length(sync_commands);
  unsigned int command_count = 0;
  int tic_count = saved_gametic - gametic;
  int extra_tics;

  for (unsigned int i = 0; i < sync_command_count; i++) {
    netticcmd_t *sync_ncmd = g_queue_peek_nth(sync_commands, i);

    if (sync_ncmd->index > latest_synchronized_command_index)
      command_count++;
  }

  extra_tics = tic_count - command_count;

  /*
   * CG: Server ran >= 1 TIC without a command from us, which means at some
   *     point it will run a TIC with > 1 commands from us, in other words, it
   *     will bunch > 1 commands together in a single TIC.  So we must bunch
   *     some commands here to maintain clientside prediction's accuracy.
   */
  while (extra_tics < 0) {
    int found_command = false;

    for (unsigned int i = 0; i < sync_command_count; i++) {
      netticcmd_t *sync_ncmd = g_queue_peek_nth(sync_commands, i);
      netticcmd_t *run_ncmd;

      if (sync_ncmd->index <= command_index)
        continue;

      run_ncmd = P_GetNewBlankCommand();
      memcpy(run_ncmd, sync_ncmd, sizeof(netticcmd_t));
      g_queue_push_tail(run_commands, run_ncmd);
      command_index = sync_ncmd->index;
      found_command = true;
    }

    if (!found_command)
      break;

    extra_tics++;
  }

  /*
   * CG: Server bunched > 1 command(s) together from us, which means at some
   *     point it will run > 1 TIC without a command from us.  So we must run
   *     these TICs here to maintain clientside prediction's accuracy.
   */
  while (extra_tics > 0) {
    cl_repredicting = true;
    N_RunTic();
    cl_repredicting = false;
    extra_tics--;
  }

  for (unsigned int i = 0; i < sync_command_count; i++) {
    netticcmd_t *sync_ncmd = g_queue_peek_nth(sync_commands, i);
    netticcmd_t *run_ncmd;

    if (sync_ncmd->index <= latest_synchronized_command_index)
      continue;

    run_ncmd = P_GetNewBlankCommand();
    memcpy(run_ncmd, sync_ncmd, sizeof(netticcmd_t));
    g_queue_push_tail(run_commands, run_ncmd);
    cl_repredicting = true;
    N_RunTic();
    if (players[displayplayer].mo != NULL) {
      R_InterpolateView(&players[displayplayer]);
      R_RestoreInterpolations();
    }
    cl_repredicting = false;
  }

#if 0
  while (gametic < saved_gametic) {
    cl_repredicting = true;
    N_RunTic();
    cl_repredicting = false;
  }
#endif
}

void CL_CheckForStateUpdates(void) {
  netpeer_t *server;
  int previous_synchronized_command_index = cl_synchronized_command_index;
  int latest_synchronized_command_index;
  GQueue *sync_commands;
  GQueue *run_commands;
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

  cl_state_tic = server->sync.tic;

  if (!CL_GetCommandSync(consoleplayer,
                         &latest_synchronized_command_index,
                         &sync_commands,
                         &run_commands)) {
    P_Echo(consoleplayer, "Server disconnected");
    N_Disconnect();
    return;
  }

  state_loaded = cl_load_new_state(
    server,
    previous_synchronized_command_index,
    latest_synchronized_command_index,
    sync_commands,
    run_commands
  );

  if (!state_loaded) {
    server->sync.tic = cl_state_tic;
    server->sync.delta.from_tic = cl_delta_from_tic;
    server->sync.delta.to_tic = cl_delta_to_tic;
    return;
  }

  N_LogPlayerPosition(&players[consoleplayer]);
  cl_state_tic = server->sync.tic;
  cl_delta_from_tic = server->sync.delta.from_tic;
  cl_delta_to_tic = server->sync.delta.to_tic;

  cl_predict(
    saved_gametic,
    latest_synchronized_command_index,
    sync_commands,
    run_commands
  );

  players[displayplayer].prev_viewz = saved_prev_viewz;
  players[displayplayer].viewz = saved_viewz;

  server->sync.outdated = true;

  if (latest_synchronized_command_index > previous_synchronized_command_index) {
    P_RemoveOldCommands(latest_synchronized_command_index, sync_commands);
    cl_synchronized_command_index = latest_synchronized_command_index;
  }
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

bool CL_SoundAllowed(void) {
  if (!MULTINET)
    return true;

  if (!CLIENT)
    return true;

  if (cl_running_nonconsoleplayer_commands)
    return true;

  if ((!cl_synchronizing) && (!cl_repredicting))
    return true;

  return false;
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

  if (cl_current_command_index > cl_latest_command_run)
    cl_latest_command_run = cl_current_command_index;
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

