/* Emacs style mode select   -*- C++ -*-
 *-----------------------------------------------------------------------------
 *
 *
 *  PrBoom: a Doom port merged with LxDoom and LSDLDoom
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 1999 by
 *  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
 *  Copyright (C) 1999-2000 by
 *  Jess Haas, Nicolas Kalkhof, Colin Phipps, Florian Schulze
 *  Copyright 2005, 2006 by
 *  Florian Schulze, Colin Phipps, Neil Stevens, Andrey Budko
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 *  02111-1307, USA.
 *
 * DESCRIPTION:
 *
 *
 *-----------------------------------------------------------------------------
 */

#include "z_zone.h"

#include <enet/enet.h>
#include "cmp.h"

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
#include "m_pbuf.h"
#include "m_delta.h"
#include "m_menu.h"
#include "p_checksum.h"
#include "p_user.h"
#include "r_fps.h"
#include "e6y.h"

#include "n_net.h"
#include "n_main.h"
#include "n_state.h"
#include "n_peer.h"
#include "n_proto.h"

#define DEBUG_NET 0
#define DEBUG_SYNC 1
#define PRINT_BANDWIDTH_STATS 0

#define SERVER_NO_PEER_SLEEP_TIMEOUT 20
#define SERVER_SLEEP_TIMEOUT 1

/* CG: TODO: Add WAD fetching (waiting on libcurl) */

static dboolean is_extra_ddisplay = false;

/* CG: Client only */
static bool received_setup = false;
static auth_level_e authorization_level = AUTH_LEVEL_NONE;
static cbuf_t local_commands;
static int local_command_index = 0;
static bool predicting = false;
static bool loading_state = false;

static void build_command(void) {
  cbuf_t *commands = NULL;
  netticcmd_t *ncmd = NULL;

  if (DELTASERVER)
    return;

  if (DELTASYNC)
    commands = &local_commands;
  else
    commands = &players[consoleplayer].commands;

  M_CBufConsolidate(commands);
  ncmd = M_CBufGetFirstFreeOrNewSlot(commands);

  I_StartTic();
  G_BuildTiccmd(ncmd);
  ncmd->index = local_command_index;
  if (DELTACLIENT)
    D_Log(LOG_SYNC, "Built command (%d, %d)\n", ncmd->index, ncmd->tic);
  local_command_index++;

  if (CLIENT) {
    netpeer_t *server = N_PeerGet(0);

    if (server != NULL)
      server->sync.outdated = true;

    N_UpdateSync();
  }

  if (DELTACLIENT)
    M_CBufAppend(&players[consoleplayer].commands, ncmd);
}

static void run_tic(void) {
  if (advancedemo)
    D_DoAdvanceDemo();

  I_GetTime_SaveMS();
  G_Ticker();
  P_Checksum(gametic);

  if (DELTASERVER) {
    N_SaveState();

    for (int i = 0; i < N_PeerGetCount(); i++) {
      netpeer_t *client = N_PeerGet(i);

      if (client == NULL)
        continue;

      if (client->sync.initialized)
        client->sync.outdated = true;
      else if (gametic > client->sync.tic)
        SV_SendSetup(client->playernum);
    }

    N_UpdateSync();
  }

  gametic++;
}

static int run_tics(int tic_count) {
  int out = tic_count;

  while (tic_count--) {
    build_command();
    run_tic();
  }

  if (CLIENT)
    D_Log(LOG_SYNC, "\n");

  return out;
}

static int run_commandsync_tics(int command_count) {
  int tic_count = command_count;

  for (int i = 0; i < command_count; i++)
    build_command();

  for (int i = 0; i < MAXPLAYERS; i++) {
    if (playeringame[i]) {
      tic_count = MIN(tic_count, M_CBufGetObjectCount(&players[i].commands));
    }
  }

  if (tic_count)
    run_tics(tic_count);

  if (CMDCLIENT && (command_count > 0)) {
    netpeer_t *server = N_PeerGet(0);

    if (server != NULL)
      server->sync.outdated = true;
  }

  return tic_count;
}

static int process_tics(int tics_elapsed) {
  netpeer_t *p;

  if (tics_elapsed <= 0)
    return 0;

  if (!MULTINET)
    return run_tics(tics_elapsed);

  p = N_PeerGet(0);

  if (p != NULL) {
    if (SERVER)
      D_Log(LOG_SYNC, "\n");

    D_Log(LOG_SYNC, "(%d) === Running %d TICs (%d/%d)\n",
      gametic, tics_elapsed, p->sync.tic, p->sync.cmd
    );
  }

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

  server = N_PeerGet(0);

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

void N_PrintPlayerCommands(cbuf_t *commands) {
  D_Log(LOG_SYNC, "[ ");

  CBUF_FOR_EACH(commands, entry) {
    netticcmd_t *ncmd = (netticcmd_t *)entry.obj;

    D_Log(LOG_SYNC, "%d/%d ", ncmd->index, ncmd->tic);
  }

  D_Log(LOG_SYNC, "]\n");
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

      if (DEBUG_SYNC && CLIENT)
        D_EnableLogChannel(LOG_SYNC, "client-sync.log");

      N_ParseAddressString(myargv[i + 1], &host, &port);

      P_Printf(consoleplayer,
        "N_InitNetGame: Connecting to server %s:%u...\n", host, port
      );

      if (!N_Connect(host, port))
        I_Error("N_InitNetGame: Connection refused");

      P_Echo(consoleplayer, "N_InitNetGame: Connected!");

      if (N_PeerGet(0) == NULL)
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
      if (DEBUG_SYNC && SERVER)
        D_EnableLogChannel(LOG_SYNC, "server-sync.log");
    }
  }

  for (int i = 0; i < MAXPLAYERS; i++) {
    M_CBufInitWithCapacity(
      &players[i].commands, sizeof(netticcmd_t), BACKUPTICS
    );
    M_OBufInit(&players[i].messages);
  }

  M_CBufInitWithCapacity(&local_commands, sizeof(netticcmd_t), BACKUPTICS);
}

bool N_GetWad(const char *name) {
  return false;
}

bool CL_LoadingState(void) {
  return loading_state;
}

bool CL_Predicting(void) {
  return predicting;
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
  netpeer_t *server = N_PeerGet(0);
  game_state_delta_t *delta = NULL;
  cbuf_t *player_commands = &players[consoleplayer].commands;

  if (server == NULL)
    return false;

  delta = &server->sync.delta;

  predicting = true;

  if (!N_ApplyStateDelta(delta)) {
    P_Echo(consoleplayer, "Error applying state delta");
    predicting = false;
    return false;
  }

  if (!N_LoadLatestState(false)) {
    P_Echo(consoleplayer, "Error loading state");
    predicting = false;
    return false;
  }

  N_RemoveOldStates(delta->from_tic);

  server->sync.tic = delta->to_tic;

  // D_Log(LOG_SYNC, "Local commands: ");
  // N_PrintPlayerCommands(&local_commands);

  CBUF_FOR_EACH(&local_commands, entry) {
    netticcmd_t *ncmd = (netticcmd_t *)entry.obj;

    if (ncmd->index <= server->sync.cmd && ncmd->tic <= server->sync.tic) {
      M_CBufRemove(&local_commands, entry.index);
      entry.index--;
    }
  }

  gametic++;

  if (M_CBufGetObjectCount(&local_commands) <= 0) {
    predicting = false;
    return true;
  }

  M_CBufClear(player_commands);
  M_CBufEnsureCapacity(player_commands, M_CBufGetObjectCount(&local_commands));
  CBUF_FOR_EACH(&local_commands, entry) {
    netticcmd_t *ncmd = (netticcmd_t *)entry.obj;

    if (ncmd->tic > gametic)
      break;

    M_CBufAppend(player_commands, entry.obj);
  }

  // D_Log(LOG_SYNC, "First command batch: ");
  // N_PrintPlayerCommands(player_commands);
  is_extra_ddisplay = true;
  run_tic();
  is_extra_ddisplay = false;

  M_CBufClear(player_commands);
  while (1) {
    dboolean found_command = false;

    CBUF_FOR_EACH(&local_commands, entry) {
      netticcmd_t *ncmd = (netticcmd_t *)entry.obj;

      if (ncmd->tic == gametic) {
        found_command = true;
        M_CBufAppend(player_commands, entry.obj);
      }
    }

    if (!found_command)
      break;

    // D_Log(LOG_SYNC, "Second command batch: ");
    // N_PrintPlayerCommands(player_commands);

    is_extra_ddisplay = true;
    run_tic();
    is_extra_ddisplay = false;
  }

  predicting = false;
  return true;
}

static bool catching_up;

bool CL_IsCatchingUp(void) {
  return catching_up;
}

void SV_RemoveOldCommands(void) {
  int oldest_gametic = gametic;

  if (DELTASERVER)
    return;

  M_CBufClear(&local_commands);

  if (!CMDSYNC)
    return;

  for (int i = 0; i < N_PeerGetCount(); i++) {
    netpeer_t *client = N_PeerGet(i);

    if (client != NULL && client->sync.tic)
      oldest_gametic = MIN(oldest_gametic, client->sync.tic);
  }

  for (int i = 0; i < N_PeerGetCount(); i++) {
    cbuf_t *commands = NULL;
    netpeer_t *client = N_PeerGet(i);

    if (client == NULL)
      continue;

    commands = &players[client->playernum].commands;

    CBUF_FOR_EACH(commands, entry) {
      netticcmd_t *ncmd = (netticcmd_t *)entry.obj;

      if (ncmd->tic < oldest_gametic) {
        D_Log(LOG_SYNC, 
          "SV_RemoveOldCommands: (%d: %d) Removing old command %d (< %d).\n",
          gametic, client->playernum, ncmd->tic, oldest_gametic
        );
        M_CBufRemove(commands, entry.index);
        entry.index--;
      }
    }
  }
}

void SV_RemoveOldStates(void) {
  int oldest_gametic = gametic;

  for (int i = 0; i < N_PeerGetCount(); i++) {
    netpeer_t *np = N_PeerGet(i);

    if (np != NULL && np->sync.tic > 0)
      oldest_gametic = MIN(oldest_gametic, np->sync.tic);
  }

  N_RemoveOldStates(oldest_gametic);
}

cbuf_t* N_GetLocalCommands(void) {
  M_CBufConsolidate(&local_commands);
  return &local_commands;
}

void N_ResetLocalCommandIndex(void) {
  local_command_index = 0;
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

  if ((!render_fast) && (tics_elapsed <= 0))
    I_uSleep(ms_to_next_tick * 1000);

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

  if (CLIENT && gametic > 0 && N_PeerGet(0) == NULL)
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

