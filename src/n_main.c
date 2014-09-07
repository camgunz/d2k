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
#include "p_checksum.h"
#include "p_cmd.h"
#include "p_user.h"
#include "r_fps.h"
#include "s_sound.h"
#include "e6y.h"

#include "n_net.h"
#include "n_main.h"
#include "n_state.h"
#include "n_peer.h"
#include "n_proto.h"

#define DEBUG_NET 0
#define DEBUG_SYNC 0
#define DEBUG_SAVE 0
#define ENABLE_PREDICTION 1
#define PRINT_BANDWIDTH_STATS 0

#define SERVER_NO_PEER_SLEEP_TIMEOUT 20
#define SERVER_SLEEP_TIMEOUT 1

/* CG: TODO: Add WAD fetching (waiting on libcurl) */

static dboolean is_extra_ddisplay = false;

/* CG: Client only */
static bool received_setup = false;
static auth_level_e authorization_level = AUTH_LEVEL_NONE;
static bool repredicting = false;
static bool loading_state = false;
static bool catching_up;

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

  if (DELTASERVER) {
    N_SaveState();

    NETPEER_FOR_EACH(entry) {
      if (entry.np->sync.initialized)
        entry.np->sync.outdated = true;
      else if (gametic > entry.np->sync.tic)
        SV_SendSetup(entry.np->playernum);
    }

    N_UpdateSync();
  }

  gametic++;
}

static int run_tics(int tic_count) {
  int out = tic_count;

  while (tic_count--) {
    P_BuildCommand();
    run_tic();
  }

  if (CLIENT)
    D_Log(LOG_SYNC, "\n");

  return out;
}

static int run_commandsync_tics(int command_count) {
  int tic_count = command_count;

  for (int i = 0; i < command_count; i++)
    P_BuildCommand();

  for (int i = 0; i < MAXPLAYERS; i++) {
    if (playeringame[i]) {
      tic_count = MIN(tic_count, P_GetPlayerCommandCount(&players[i]));
    }
  }

  if (tic_count)
    run_tics(tic_count);

  if (CMDCLIENT && (command_count > 0)) {
    netpeer_t *server = CL_GetServerPeer();

    if (server != NULL)
      server->sync.outdated = true;
  }

  return tic_count;
}

static int process_tics(int tics_elapsed) {
  if (tics_elapsed <= 0)
    return 0;

  if (!MULTINET)
    return run_tics(tics_elapsed);

  D_Log(LOG_SYNC, "(%d) %d tics elapsed\n", gametic, tics_elapsed);

  if (CMDSYNC)
    return run_commandsync_tics(tics_elapsed);

  return run_tics(tics_elapsed);
}

static void render_menu(int menu_renderer_calls) {
  while (menu_renderer_calls--)
    M_Ticker();
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

  if (DELTASERVER)
    SV_RemoveOldStates();
}

static void deltaclient_service_network(void) {
  netpeer_t *server;
  int latest_sync_tic;
  int latest_command_index;
  int latest_delta_from_tic;
  int latest_delta_to_tic;

  if (!DELTACLIENT)
    return;

  server = CL_GetServerPeer();

  if (gametic <= 0) {
    service_network();
    return;
  }

  if (server == NULL)
    I_Error("Server disconnected");

  latest_sync_tic = server->sync.tic;
  latest_command_index = server->sync.cmd;
  latest_delta_from_tic = server->sync.delta.from_tic;
  latest_delta_to_tic = server->sync.delta.to_tic;

  service_network();

  if (server->sync.delta.to_tic != latest_delta_to_tic) {
    if (CL_LoadState()) {
      server->sync.outdated = true;
    }
    else {
      server->sync.tic = latest_sync_tic;
      server->sync.cmd = latest_command_index;
      server->sync.delta.from_tic = latest_delta_from_tic;
      server->sync.delta.to_tic = latest_delta_to_tic;
    }
  }
}

static void render_extra_frame(void) {
  dboolean should_render = false;

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

static void cleanup_old_commands_and_states(void) {
  if (!DELTASERVER)
    return;

  SV_RemoveOldStates();
  SV_RemoveOldCommands();
}

void N_LogPlayerPosition(player_t *player) {
  D_Log(LOG_SYNC, "(%5d/%5d): %td: {%4d/%4d/%4d %4d/%4d/%4d %4d/%4d/%4d/%4d}\n", 
    gametic,
    leveltime,
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

  for (int i = 0; i < MAXPLAYERS; i++) {
    char *name;
    size_t name_length;

    P_InitPlayerCommands(&players[i]);

    players[i].messages = g_ptr_array_new_with_free_func(P_DestroyMessage);

    players[i].name = NULL;

    name_length = snprintf(NULL, 0, "Player %d", i);
    name = calloc(name_length + 1, sizeof(char));
    snprintf(name, name_length + 1, "Player %d", i);

    P_SetName(i, name);
  }

  P_InitLocalCommands();
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

void CL_SetReceivedSetup(dboolean new_received_setup) {
  received_setup = new_received_setup;
}

void CL_SetAuthorizationLevel(auth_level_e level) {
  if (level > authorization_level)
    authorization_level = level;
}

bool CL_LoadState(void) {
  netpeer_t *server = CL_GetServerPeer();
  game_state_delta_t *delta = NULL;

  if (server == NULL)
    return false;

  delta = &server->sync.delta;

  repredicting = true;
  S_MuteSound();

  if (!N_ApplyStateDelta(delta)) {
    P_Echo(consoleplayer, "Error applying state delta");
    repredicting = false;
    S_UnMuteSound();
    return false;
  }

  if (!N_LoadLatestState(false)) {
    P_Echo(consoleplayer, "Error loading state");
    repredicting = false;
    S_UnMuteSound();
    return false;
  }

  N_RemoveOldStates(delta->from_tic);

  server->sync.tic = delta->to_tic;

  P_RemoveSyncedCommands();

  gametic++;

  if (P_GetLocalCommandCount() <= 0) {
    repredicting = false;
    S_UnMuteSound();
    return true;
  }

  P_UpdateConsoleplayerCommands();

  is_extra_ddisplay = true;
  run_tic();
  is_extra_ddisplay = false;

  P_ClearPlayerCommands(&players[consoleplayer]);

  while (P_LoadLocalCommandForTic(gametic)) {
    is_extra_ddisplay = true;
    run_tic();
    is_extra_ddisplay = false;
  }

  repredicting = false;
  S_UnMuteSound();
  return true;
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

bool CL_GetServerSync(int *command_index, int *sync_tic) {
  netpeer_t *server;

  if (!CLIENT)
    return false;

  server = CL_GetServerPeer();

  if (server == NULL)
    return false;

  *command_index = server->sync.cmd;
  *sync_tic = server->sync.tic;

  return true;
}

bool CL_IsCatchingUp(void) {
  return catching_up;
}

void SV_RemoveOldCommands(void) {
  int oldest_gametic = gametic;

  if (DELTASERVER)
    return;

  P_ClearLocalCommands();

  if (!CMDSYNC)
    return;

  NETPEER_FOR_EACH(entry) {
    netpeer_t *client = entry.np;

    if (client->sync.tic != 0)
      oldest_gametic = MIN(oldest_gametic, client->sync.tic);
  }

  NETPEER_FOR_EACH(entry) {
    netpeer_t *client = entry.np;
    P_RemoveOldCommands(&players[client->playernum], oldest_gametic);
  }
}

void SV_RemoveOldStates(void) {
  int oldest_gametic = gametic;

  NETPEER_FOR_EACH(entry) {
    netpeer_t *np = entry.np;

    if (np->sync.tic > 0)
      oldest_gametic = MIN(oldest_gametic, np->sync.tic);
  }

  N_RemoveOldStates(oldest_gametic);
}

void N_TryRunTics(void) {
  static int tics_built = 0;

  int tics_elapsed = I_GetTime() - tics_built;
  int menu_renderer_calls = tics_elapsed * 3;

  int tics_run = 0;
  dboolean render_fast = false;
  
  if (!SERVER)
    render_fast = movement_smooth && window_focused && (gametic > 0);

#ifdef GL_DOOM
  if ((!SERVER) && (V_GetMode() == VID_MODEGL))
    render_fast = true;
#endif

  if ((!render_fast) && (tics_elapsed <= 0)) {
    D_Log(LOG_SYNC, "Sleeping\n");
    I_uSleep(ms_to_next_tick * 1000);
  }

  if (tics_elapsed > 0) {
    D_Log(LOG_SYNC, "%d tics elapsed (2)\n", tics_elapsed);
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
  else {
    D_Log(LOG_SYNC, "Zero tics elapsed\n");
  }

  C_Ticker();

  cleanup_old_commands_and_states();

  if (CLIENT && gametic > 0 && CL_GetServerPeer() == NULL)
    I_Error("Server disconnected.\n");

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

