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
#include "cl_main.h"
#include "sv_main.h"
#include "p_checksum.h"
#include "p_cmd.h"
#include "p_user.h"
#include "r_fps.h"
#include "s_sound.h"
#include "e6y.h"

#define DEBUG_NET 0
#define DEBUG_SYNC 0
#define DEBUG_SAVE 0
#define PRINT_NETWORK_STATS 0

#define SERVER_NO_PEER_SLEEP_TIMEOUT 20
#define SERVER_SLEEP_TIMEOUT 1

static int run_tics(int tic_count) {
  int saved_tic_count = tic_count;

  if (CLIENT)
    D_Log(LOG_SYNC, "Building %d commands\n", tic_count);

  while (tic_count--) {
    if (MULTINET) {
      P_BuildCommand();
    }
    else {
      memset(&players[consoleplayer].cmd, 0, sizeof(ticcmd_t));
      G_BuildTiccmd(&players[consoleplayer].cmd);
    }
    N_RunTic();
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

static bool should_render(void) {
  if (nodrawers)
    return false;

  if (!window_focused)
    return false;

  if (gametic <= 0)
    return false;

#ifdef GL_DOOM
  if (V_GetMode() == VID_MODEGL)
    return true;
#endif

  if (!movement_smooth)
    return false;

  if (gamestate != wipegamestate)
    return false;

  return true;
}

#if PRINT_NETWORK_STATS
static void print_network_stats(void) {
  static float frequency_factor = .5;
  static uint64_t loop_count = 0;

  int frequency = TICRATE * frequency_factor;

  if ((loop_count++ % frequency) != 0)
    return;

  if (N_PeerGetCount() <= 0)
    return;

  NETPEER_FOR_EACH(iter) {
    puts("------------------------------------------------------------------------------");
    puts("|  TIC  |      D/U      | Max/Last/Avg RTT | Max/Last/Avg RTTv |  Commands   |");
    puts("------------------------------------------------------------------------------");
    printf("| %5d | %4d/%4d b/s | %3d/%3d/%3d ms   | %3d/%3d/%3d ms    |       %5d |\n",
      gametic,
      N_GetUploadBandwidth(),
      N_GetDownloadBandwidth(),
      iter.np->peer->lowestRoundTripTime,
      iter.np->peer->lastRoundTripTime,
      iter.np->peer->roundTripTime,
      iter.np->peer->highestRoundTripTimeVariance,
      iter.np->peer->lastRoundTripTimeVariance,
      iter.np->peer->roundTripTimeVariance,
      CLIENT ? P_GetPlayerSyncCommandCount(consoleplayer) : 0
    );
    puts("------------------------------------------------------------------------------");
    puts("| Packet Loss | Throttle | Accel | Counter | Decel | Interval | Limit |  #   |");
    puts("------------------------------------------------------------------------------");
    printf("| %4.1f%%/%4.1f%% |    %5d | %5d |   %5d | %5d |    %5d | %5d |   %2d |\n",
      (iter.np->peer->packetLoss / (float)ENET_PEER_PACKET_LOSS_SCALE) * 100.f,
      (iter.np->peer->packetLossVariance / (float)ENET_PEER_PACKET_LOSS_SCALE) * 100.f,
      iter.np->peer->packetThrottle,
      iter.np->peer->packetThrottleAcceleration,
      iter.np->peer->packetThrottleCounter,
      iter.np->peer->packetThrottleDeceleration,
      iter.np->peer->packetThrottleInterval,
      iter.np->peer->packetThrottleLimit,
      iter.np->playernum
    );
    puts("------------------------------------------------------------------------------");
  }
}
#endif

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
  if (player->mo == NULL)
    return;

  D_Log(LOG_SYNC,
    "(%d): %td: {%4d/%4d/%4d %4d/%4d/%4d %4d %4d/%4d/%4d/%4d %4d/%4u/%4u}\n", 
    gametic,
    player - players,
    player->mo->x           >> FRACBITS,
    player->mo->y           >> FRACBITS,
    player->mo->z           >> FRACBITS,
    player->mo->momx        >> FRACBITS,
    player->mo->momy        >> FRACBITS,
    player->mo->momz        >> FRACBITS,
    player->mo->angle       /  ANG1,
    player->viewz           >> FRACBITS,
    player->viewheight      >> FRACBITS,
    player->deltaviewheight >> FRACBITS,
    player->bob             >> FRACBITS,
    player->prev_viewz      >> FRACBITS,
    player->prev_viewangle  /  ANG1,
    player->prev_viewpitch  /  ANG1
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

        if (CL_ReceivedSetup())
          break;

        if (difftime(now, connect_time) > (NET_SETUP_TIMEOUT * 1000))
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

/* CG: TODO: Add WAD fetching (waiting on libcurl) */
bool N_GetWad(const char *name) {
  return false;
}

void N_RunTic(void) {
  if (advancedemo)
    D_DoAdvanceDemo();

  M_Ticker();

  if ((!CL_RePredicting()) && (!CL_Synchronizing()))
    I_GetTime_SaveMS();

  G_Ticker();

  P_Checksum(gametic);

  if (DELTASERVER && gametic > 0) {
    /* CG: TODO: Don't save states if there are no peers, saves resources */
    D_Log(LOG_SYNC, "(%d) Saving state\n", gametic);
    N_SaveState();

    NETPEER_FOR_EACH(iter) {
      if (iter.np->sync.initialized) {
        iter.np->sync.outdated = true;
        if (playeringame[iter.np->playernum])
          N_LogPlayerPosition(&players[iter.np->playernum]);
      }
      else if (gametic > iter.np->sync.tic) {
        SV_SendSetup(iter.np->playernum);
      }
    }

    N_UpdateSync();
  }

  gametic++;
}

bool N_TryRunTics(void) {
  static int tics_built = 0;

  int tics_elapsed = I_GetTime() - tics_built;
  bool needs_rendering = should_render();

  if (tics_elapsed <= 0 && !needs_rendering) {
    N_ServiceNetwork();
    I_Sleep(1);
    return false;
  }

  if (tics_elapsed > 0) {
    tics_built += tics_elapsed;

    if (ffmap)
      tics_elapsed++;

    CL_CheckForStateUpdates();

    process_tics(tics_elapsed);

    SV_CleanupOldCommandsAndStates();

    if (CLIENT && gametic > 0 && CL_GetServerPeer() == NULL) {
      P_Echo(consoleplayer, "Server disconnected.");
      N_Disconnect();
    }

#if PRINT_NETWORK_STATS
    print_network_stats();
#endif

    N_ServiceNetwork();
  }

  if ((tics_elapsed > 0) || needs_rendering)
    D_Display();

#ifdef ENABLE_OVERLAY
  C_Ticker();
#endif

  return tics_elapsed > 0;
}

/* vi: set et ts=2 sw=2: */

