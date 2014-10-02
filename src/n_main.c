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

#include "doomstat.h"
#include "protocol.h"

#include "d_event.h"
#include "c_main.h"
#include "d_main.h"
#include "g_game.h"
#include "i_network.h"
#include "i_system.h"
#include "i_main.h"
#include "i_video.h"
#include "lprintf.h"
#include "m_argv.h"
#include "m_delta.h"
#include "m_menu.h"
#include "n_net.h"
#include "n_main.h"
#include "n_state.h"
#include "n_peer.h"
#include "n_proto.h"
#include "p_checksum.h"
#include "p_cmd.h"
#include "p_user.h"
#include "r_fps.h"
#include "s_sound.h"
#include "e6y.h"

#define DEBUG_NET 0
#define DEBUG_SYNC 1
#define DEBUG_SAVE 0
#define ENABLE_PREDICTION 1
#define PRINT_BANDWIDTH_STATS 0

#define SERVER_NO_PEER_SLEEP_TIMEOUT 20
#define SERVER_SLEEP_TIMEOUT 1

/* CG: TODO: Add WAD fetching (waiting on libcurl) */

/* CG: Client only */
/* CG: TODO: Prefix all these with cl_ */
static bool         received_setup = false;
static auth_level_e authorization_level = AUTH_LEVEL_NONE;
static bool         loading_state = false;
static int          local_command_index = 0;
static int          current_command_index = 0;
static int          latest_command_run = 0;
static bool         running_consoleplayer_commands = false;
static int          cl_state_tic = -1;
static int          cl_command_index = -1;
static int          cl_delta_from_tic = -1;
static int          cl_delta_to_tic = -1;

static void run_tic(void) {
  if (advancedemo)
    D_DoAdvanceDemo();

  I_GetTime_SaveMS();

#if ENABLE_PREDICTION
  G_Ticker();
#else
  if (SERVER)
    G_Ticker();
#endif
  P_Checksum(gametic);

  if (DELTASERVER && gametic > 0) {
    /* CG: TODO: Don't save states if there are no peers, saves resources */
    N_SaveState();

    NETPEER_FOR_EACH(iter) {
      if (iter.np->sync.initialized)
        iter.np->sync.outdated = true;
      else if (gametic > iter.np->sync.tic)
        SV_SendSetup(iter.np->playernum);
    }

    N_UpdateSync();
  }

  gametic++;
}

static int run_tics(int tic_count) {
  int saved_tic_count = tic_count;

  while (tic_count--) {
    if (MULTINET)
      P_BuildCommand();
    else
      G_BuildTiccmd(&players[consoleplayer].cmd);
    run_tic();
  }

  return saved_tic_count;
}

static int run_commandsync_tics(int command_count) {
  int tic_count = command_count;

  while (command_count--)
    P_BuildCommand();

  for (int i = 0; i < MAXPLAYERS; i++) {
    if (playeringame[i]) {
      tic_count = MIN(tic_count, P_GetPlayerCommandCount(i));
    }
  }

  if (tic_count > 0)
    run_tics(tic_count);

  NETPEER_FOR_EACH(iter) {
    iter.np->sync.outdated = true;
  }

  return tic_count;
}

static int process_tics(int tics_elapsed) {
  if (tics_elapsed <= 0)
    return 0;

  if (CMDSYNC)
    return run_commandsync_tics(tics_elapsed);

  return run_tics(tics_elapsed);
}

static void render_menu(int menu_renderer_calls) {
  while (menu_renderer_calls--)
    M_Ticker();
}

static void cl_load_latest_state(void) {
  netpeer_t *server;
  int state_tic;
  int command_index;
  GQueue *sync_commands;
  GQueue *run_commands;
  int saved_gametic;

  if (!DELTACLIENT)
    return;

  server = CL_GetServerPeer();

  if (server == NULL) {
    P_Echo(consoleplayer, "Server disconnected");
    N_Disconnect();
    return;
  }

  if (!CL_GetCommandSync(consoleplayer, &command_index, &sync_commands,
                                                        &run_commands)) {
    P_Echo(consoleplayer, "Server disconnected");
    N_Disconnect();
    return;
  }

  state_tic = server->sync.tic;

  if (command_index > cl_command_index) {
    P_RemoveOldCommands(command_index, sync_commands);
    cl_command_index = command_index;
    server->sync.outdated = true;
  }

  if (server->sync.tic != cl_state_tic) {
    game_state_delta_t *delta = &server->sync.delta;
    bool state_loaded;

    if (!N_ApplyStateDelta(delta)) {
      P_Echo(consoleplayer, "Error applying state delta");
      return;
    }

    current_command_index = command_index;

    D_Log(LOG_SYNC, "(%d) Loading new state [%d => %d] {%d (%d)}\n",
      gametic,
      delta->from_tic,
      delta->to_tic,
      command_index,
      local_command_index - 1
    );

    loading_state = true;
    saved_gametic = gametic;
    state_loaded = N_LoadLatestState(false);
    loading_state = false;

    if (!state_loaded) {
      P_Echo(consoleplayer, "Error loading state");
      server->sync.tic = cl_state_tic;
      delta->from_tic = cl_delta_from_tic;
      delta->to_tic = cl_delta_to_tic;
      return;
    }

    N_RemoveOldStates(delta->from_tic);

    cl_state_tic = server->sync.tic;
    cl_delta_from_tic = delta->from_tic;
    cl_delta_to_tic = delta->to_tic;

#if ENABLE_PREDICTION
    unsigned int sync_command_count = g_queue_get_length(sync_commands);
    unsigned int sync_command_index = 0;

    for (unsigned int i = 0; i < sync_command_count; i++) {
      netticcmd_t *sync_ncmd = g_queue_peek_nth(sync_commands, i);
      netticcmd_t *run_ncmd = P_GetNewBlankCommand();

      memcpy(run_ncmd, sync_ncmd, sizeof(netticcmd_t));
      g_queue_push_tail(run_commands, run_ncmd);
    }

    D_Log(LOG_SYNC, "(%d) Repredicting %d TICs...\n",
      gametic, saved_gametic - gametic
    );

    while (gametic < saved_gametic)
      run_tic();

    D_Log(LOG_SYNC, "(%d) Finished repredicting\n", gametic);
#endif

    server->sync.outdated = true;
  }
}

static void render_extra_frame(void) {
#ifdef GL_DOOM
  if (V_GetMode() == VID_MODEGL) {
    D_Display();
    WasRenderedInTryRunTics = true;
  }
#else
  if (movement_smooth && gamestate == wipegamestate) {
    D_Display();
    WasRenderedInTryRunTics = true;
  }
#endif
}

static void sv_remove_old_commands(void) {
  NETPEER_FOR_EACH(iter) {
    for (int i = 0; i < MAXPLAYERS; i++) {
      if (i != iter.np->playernum) {
        P_RemoveOldCommands(
          iter.np->sync.commands[i].sync_index,
          iter.np->sync.commands[i].sync_queue
        );
      }
    }
  }
}

static void sv_remove_old_states(void) {
  int oldest_gametic = gametic;

  NETPEER_FOR_EACH(entry) {
    netpeer_t *np = entry.np;

    if (np->sync.tic > 0)
      oldest_gametic = MIN(oldest_gametic, np->sync.tic);
  }

  N_RemoveOldStates(oldest_gametic);
}

static void sv_cleanup_old_commands_and_states(void) {
  if (!SERVER)
    return;

  sv_remove_old_commands();
  if (DELTASERVER)
    sv_remove_old_states();
}

void N_LogCommand(netticcmd_t *ncmd) {
  D_Log(LOG_SYNC, "(%d): {%d/%d %d %d %d %d %u %u}\n",
    gametic,
    ncmd->index,
    ncmd->tic,
    ncmd->cmd.forwardmove,
    ncmd->cmd.sidemove,
    ncmd->cmd.angleturn,
    ncmd->cmd.consistancy,
    ncmd->cmd.chatchar,
    ncmd->cmd.buttons
  );
}

void N_LogPlayerPosition(player_t *player) {
  D_Log(LOG_SYNC, "(%d): %td: {%4d/%4d/%4d %4d/%4d/%4d %4d/%4d/%4d/%4d}\n", 
    gametic,
    player - players,
    player->mo->x           >> FRACBITS,
    player->mo->y           >> FRACBITS,
    player->mo->z           >> FRACBITS,
    player->mo->momx        >> FRACBITS,
    player->mo->momy        >> FRACBITS,
    player->mo->momz        >> FRACBITS,
    player->viewz           >> FRACBITS,
    player->viewheight      >> FRACBITS,
    player->deltaviewheight >> FRACBITS,
    player->bob             >> FRACBITS
  );
}

void N_InitNetGame(void) {
  int i;

  netgame   = false;
  solonet   = false;
  netserver = false;
  netsync   = NET_SYNC_TYPE_NONE;

  displayplayer = consoleplayer = 0;
  playeringame[consoleplayer] = true;

  M_InitDeltas();

  for (i = 0; i < MAXPLAYERS; i++) {
    char *name;
    size_t name_length;

    players[i].messages = g_ptr_array_new_with_free_func(P_DestroyMessage);

    players[i].name = NULL;
    name_length = snprintf(NULL, 0, "Player %d", i);
    name = calloc(name_length + 1, sizeof(char));
    snprintf(name, name_length + 1, "Player %d", i);

    P_SetName(i, name);
  }

  P_InitCommands();

  if ((i = M_CheckParm("-solo-net"))) {
    netgame = true;
    solonet = true;
  }
  else if ((i = M_CheckParm("-net"))) {
    if (i < myargc - 1) {
      char *host = NULL;
      unsigned short port = 0;
      time_t connect_time;

      netgame = true;

      N_Init();

      if (DEBUG_NET && CLIENT)
        D_EnableLogChannel(LOG_NET, "client-net.log");

      if (DEBUG_SAVE && CLIENT)
        D_EnableLogChannel(LOG_SAVE, "client-save.log");

      if (DEBUG_SYNC && CLIENT)
        D_EnableLogChannel(LOG_SYNC, "client-sync.log");

      N_ParseAddressString(myargv[i + 1], &host, &port);

      P_Printf(consoleplayer,
        "N_InitNetGame: Connecting to server %s:%u...\n", host, port
      );

      if (!N_Connect(host, port))
        I_Error("N_InitNetGame: Connection refused");

      P_Echo(consoleplayer, "N_InitNetGame: Connected!");

      if (CL_GetServerPeer() == NULL)
        I_Error("N_InitNetGame: Server peer was NULL");

      connect_time = time(NULL);

      G_ReloadDefaults();

      P_Echo(consoleplayer, "N_InitNetGame: Waiting for setup information...");

      while (true) {
        time_t now = time(NULL);

        N_ServiceNetwork();

        if (received_setup)
          break;

        if (difftime(now, connect_time) > (CONNECT_TIMEOUT * 1000))
          I_Error("N_InitNetGame: Timed out waiting for setup information");
      }

      P_Echo(consoleplayer, "N_InitNetGame: Setup information received!");
    }
  }
  else {
    if (M_CheckParm("-commandserve") && M_CheckParm("-deltaserve"))
      I_Error("Cannot specify both '-commandserve' and '-deltaserve'");
    else if ((i = M_CheckParm("-commandserve")))
      netsync = NET_SYNC_TYPE_COMMAND;
    else if ((i = M_CheckParm("-deltaserve")))
      netsync = NET_SYNC_TYPE_DELTA;

    if (CMDSYNC || DELTASYNC) {
      char *host = NULL;
      unsigned short port = DEFAULT_PORT;

      netgame = true;
      netserver = true;

      nodrawers   = true;
      nosfxparm   = true;
      nomusicparm = true;

      N_Init();

      if (i < myargc - 1) {
        size_t host_length = N_ParseAddressString(myargv[i + 1], &host, &port);

        if (host_length == 0)
          host = strdup("0.0.0.0");
      }
      else {
        host = strdup("0.0.0.0");
      }

      if (!N_Listen(host, port))
        I_Error("Startup aborted");

      P_Printf(consoleplayer, "N_InitNetGame: Listening on %s:%u.\n",
        host, port
      );

      if (DEBUG_NET && SERVER)
        D_EnableLogChannel(LOG_NET, "server-net.log");

      if (DEBUG_SAVE && SERVER)
        D_EnableLogChannel(LOG_SAVE, "server-save.log");

      if (DEBUG_SYNC && SERVER)
        D_EnableLogChannel(LOG_SYNC, "server-sync.log");
    }
  }
}

bool N_GetWad(const char *name) {
  return false;
}

bool CL_LoadingState(void) {
  return loading_state;
}

bool CL_RePredicting(void) {
  if (CLIENT &&
      running_consoleplayer_commands &&
      current_command_index <= latest_command_run) {
    return true;
  }

  return false;
}

void CL_SetupCommandState(int playernum, netticcmd_t *ncmd) {
  if (!CLIENT)
    return;

  if (playernum == consoleplayer)
    running_consoleplayer_commands = true;
  else
    running_consoleplayer_commands = false;

  if (running_consoleplayer_commands)
    current_command_index = ncmd->index;
}

void CL_ShutdownCommandState(void) {
  if (current_command_index > latest_command_run)
    latest_command_run = current_command_index;
}

int CL_GetCurrentCommandIndex(void) {
  return current_command_index;
}

int CL_GetNextCommandIndex(void) {
  int out = local_command_index;

  local_command_index++;

  return out;
}

bool CL_ReceivedSetup(void) {
  return received_setup;
}

void CL_SetReceivedSetup(bool new_received_setup) {
  received_setup = new_received_setup;
}

void CL_SetAuthorizationLevel(auth_level_e level) {
  if (level > authorization_level)
    authorization_level = level;
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

bool N_TryRunTics(void) {
  static int tics_built = 0;

  int tics_elapsed = I_GetTime() - tics_built;
  int menu_renderer_calls = tics_elapsed * 3;

  int tics_run = 0;
  bool render_fast = false;
  
  if (!SERVER)
    render_fast = movement_smooth && window_focused && (gametic > 0);

#ifdef GL_DOOM
  if ((!SERVER) && (V_GetMode() == VID_MODEGL))
    render_fast = true;
#endif

  if (tics_elapsed <= 0 && !render_fast) {
    N_ServiceNetwork();
    I_Sleep(1);
    return false;
  }

  if (tics_elapsed > 0) {
    tics_built += tics_elapsed;

    if (tics_elapsed > 1)
      D_Log(LOG_SYNC, "%d TICs elapsed!!!\n", tics_elapsed);

    if (ffmap)
      tics_elapsed++;

    cl_load_latest_state();

    tics_run = process_tics(tics_elapsed);

    render_menu(MAX(menu_renderer_calls - tics_run, 0));
  }

  if (render_fast)
    render_extra_frame();

  C_Ticker();

  sv_cleanup_old_commands_and_states();

  if (CLIENT && gametic > 0 && CL_GetServerPeer() == NULL) {
    P_Echo(consoleplayer, "Server disconnected.");
    N_Disconnect();
  }

#if PRINT_BANDWIDTH_STATS
  static uint64_t loop_count = 0;
  if (((loop_count++ % 2000) == 0) && SERVER && N_PeerGetCount() > 0) {
    printf("(%d) U/D: %d/%d b/s\n",
      gametic, N_GetUploadBandwidth(), N_GetDownloadBandwidth()
    );
  }
#endif

  N_ServiceNetwork();

  return true;
}

/* vi: set et ts=2 sw=2: */

