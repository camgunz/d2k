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

static bool is_extra_ddisplay = false;

/* CG: Client only */
static bool received_setup = false;
static auth_level_e authorization_level = AUTH_LEVEL_NONE;
static bool repredicting = false;
static bool loading_state = false;

static void run_tic(void) {
  if (advancedemo)
    D_DoAdvanceDemo();

  I_GetTime_SaveMS();

#ifdef ENABLE_PREDICTION
  G_Ticker();
#else
  if (SERVER)
    G_Ticker();
#endif
  P_Checksum(gametic);

  if (DELTASERVER && gametic > 0) {
    /* CG: TODO: Don't save states if there are no peers; saves resources */
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

static void cl_remove_old_commands(void) {
  netpeer_t *server = CL_GetServerPeer();
  
  if (server == NULL) {
    P_Echo(consoleplayer, "Server disconnected");
    N_Disconnect();
    return;
  }

  P_RemoveOldCommands(
    server->sync.commands[consoleplayer].sync_index,
    server->sync.commands[consoleplayer].sync_queue
  );
}

static void log_consoleplayer_position(void) {
  player_t *player = &players[consoleplayer];

  D_Log(LOG_SYNC, "(%5d/%5d): %d: {%4d/%4d/%4d %4d/%4d/%4d %4d/%4d/%4d/%4d}\n", 
    gametic,
    leveltime,
    consoleplayer,
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

static bool cl_load_state(void) {
  netpeer_t *server = CL_GetServerPeer();
  game_state_delta_t *delta;
  GQueue *run_commands;
  GQueue *sync_commands;
  unsigned int sync_command_count;

  if (server == NULL)
    return false;

  D_Log(LOG_SYNC, "Loading state %d => %d (%d)\n",
    server->sync.delta.from_tic,
    server->sync.delta.to_tic,
    server->sync.tic
  );

  delta = &server->sync.delta;

  if (!N_ApplyStateDelta(delta)) {
    P_Echo(consoleplayer, "Error applying state delta");
    return false;
  }

  if (!N_LoadLatestState(false)) {
    P_Echo(consoleplayer, "Error loading state");
    return false;
  }

  N_RemoveOldStates(delta->from_tic);

  run_commands = server->sync.commands[consoleplayer].run_queue;
  sync_commands = server->sync.commands[consoleplayer].sync_queue;
  sync_command_count = g_queue_get_length(sync_commands);

  for (unsigned int i = 0; i < sync_command_count; i++) {
    netticcmd_t *sync_ncmd = g_queue_peek_nth(sync_commands, i);
    netticcmd_t *run_ncmd;

    if (sync_ncmd->index >= server->sync.commands[consoleplayer].sync_index)
      break;

    run_ncmd = P_GetNewBlankCommand();
    memcpy(run_ncmd, sync_ncmd, sizeof(netticcmd_t));
    g_queue_push_tail(run_commands, run_ncmd);
  }

  /*
  is_extra_ddisplay = true;
  run_tic();
  is_extra_ddisplay = false;

  log_consoleplayer_position();
  */

  cl_remove_old_commands();
  sync_command_count = g_queue_get_length(sync_commands);

  repredicting = true;
  S_MuteSound();
  D_Log(LOG_SYNC, "(%d) Repredicting %d TICs...\n",
    gametic, sync_command_count
  );

  for (unsigned int i = 0; i < sync_command_count; i++) {
    netticcmd_t *sync_ncmd = g_queue_peek_nth(sync_commands, i);
    netticcmd_t *run_ncmd = P_GetNewBlankCommand();

    memcpy(run_ncmd, sync_ncmd, sizeof(netticcmd_t));
    g_queue_push_tail(run_commands, run_ncmd);

    is_extra_ddisplay = true;
    run_tic();
    is_extra_ddisplay = false;
  }
  repredicting = false;
  S_UnMuteSound();

  D_Log(LOG_SYNC, "(%d) Finished repredicting\n", gametic);

  return true;
}

static void service_network(void) {
  if (!MULTINET)
    return;

  N_ServiceNetwork();

#if 0
  if (CLIENT)
    N_ServiceNetworkTimeout(SERVER_NO_PEER_SLEEP_TIMEOUT);
  else if (N_PeerGetCount() > 0)
    N_ServiceNetworkTimeout(SERVER_SLEEP_TIMEOUT);
  else
    N_ServiceNetworkTimeout(SERVER_NO_PEER_SLEEP_TIMEOUT);
#endif
}

static void deltaclient_service_network(void) {
  netpeer_t *server;
  int latest_sync_tic;
  int latest_delta_from_tic;
  int latest_delta_to_tic;
  
  if (!DELTACLIENT)
    return;

  server = CL_GetServerPeer();

  if (server == NULL) {
    P_Echo(consoleplayer, "Server disconnected");
    N_Disconnect();
    return;
  }

  latest_sync_tic = server->sync.tic;
  latest_delta_from_tic = server->sync.delta.from_tic;
  latest_delta_to_tic = server->sync.delta.to_tic;

  /* CG: [XXX] What is this here for? */
  /*
  if (gametic <= 0) {
    service_network();
    return;
  }
  */

  service_network();

  if (server->sync.tic != latest_sync_tic) {
    if (cl_load_state()) {
      server->sync.outdated = true;
    }
    else {
      server->sync.tic = latest_sync_tic;
      server->sync.delta.from_tic = latest_delta_from_tic;
      server->sync.delta.to_tic = latest_delta_to_tic;
    }
  }
}

static void render_extra_frame(void) {
  bool should_render = false;

  WasRenderedInTryRunTics = true;

  should_render = movement_smooth && gamestate == wipegamestate;

#ifdef GL_DOOM
  if (V_GetMode() == VID_MODEGL)
    should_render = true;
#endif

  if (should_render) {
    is_extra_ddisplay = true;
    D_Display();
    is_extra_ddisplay = false;
  }
}

static void sv_remove_old_commands(void) {
  NETPEER_FOR_EACH(iter) {
    netpeer_t *np = iter.np;

    for (int i = 0; i < MAXPLAYERS; i++) {
      if (i == np->playernum)
        continue;

      P_RemoveOldCommands(
        np->sync.commands[i].sync_index, np->sync.commands[i].sync_queue
      );
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

static void cleanup_old_commands_and_states(void) {
  if (!SERVER)
    return;

  sv_remove_old_commands();
  if (DELTASERVER)
    sv_remove_old_states();
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
  return repredicting;
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

void N_TryRunTics(void) {
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

  if ((!render_fast) && (tics_elapsed <= 0)) {
    I_uSleep(ms_to_next_tick * 1000);
  }

  if (tics_elapsed > 0) {
    if (DELTACLIENT) {
      loading_state = true;
      deltaclient_service_network();
      loading_state = false;
    }
    else {
      service_network();
    }

    tics_built += tics_elapsed;

    if (ffmap)
      tics_elapsed++;

    tics_run = process_tics(tics_elapsed);

    render_menu(MAX(menu_renderer_calls - tics_run, 0));
  }

  C_Ticker();

  cleanup_old_commands_and_states();

  if (CLIENT && gametic > 0 && CL_GetServerPeer() == NULL) {
    P_Echo(consoleplayer, "Server disconnected.");
    N_Disconnect();
  }

  if ((!SERVER) && render_fast)
    render_extra_frame();

#if PRINT_BANDWIDTH_STATS
  static uint64_t loop_count = 0;
  if (((loop_count++ % 2000) == 0) && SERVER && N_PeerGetCount() > 0) {
    printf("(%d) U/D: %d/%d b/s\n",
      gametic, N_GetUploadBandwidth(), N_GetDownloadBandwidth()
    );
  }
#endif
}

/* vi: set et ts=2 sw=2: */

