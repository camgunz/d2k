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

#include "i_system.h"
#include "i_input.h"
#include "i_main.h"
#include "i_video.h"
#include "m_argv.h"
#include "m_delta.h"
#include "c_eci.h"
#include "c_main.h"
#include "d_main.h"
#include "d_msg.h"
#include "e6y.h"
#include "g_game.h"
#include "g_state.h"
#include "mn_main.h"
#include "p_checksum.h"
#include "p_mobj.h"
#include "pl_cmd.h"
#include "pl_main.h"
#include "pl_msg.h"
#include "r_defs.h"
#include "r_fps.h"
#include "s_sound.h"
#include "sv_main.h"
#include "v_video.h"
#include "x_main.h"

#include "hu_lib.h"
#include "hu_stuff.h"

#include "n_addr.h"
#include "n_main.h"
#include "n_msg.h"
#include "n_peer.h"
#include "n_proto.h"
#include "cl_cmd.h"
#include "cl_main.h"
#include "cl_net.h"

#define SERVER_NO_PEER_SLEEP_TIMEOUT 20
#define SERVER_SLEEP_TIMEOUT 1
#define MAX_SETUP_REQUEST_ATTEMPTS 10

bool netgame   = false;
bool solonet   = false;
bool netserver = false;

const char *disconnection_reasons[DISCONNECT_REASON_MAX] = {
  "Lost peer connection",
  "Peer disconnected",
  "Manual disconnection",
  "Connection error",
  "Excessive lag",
  "Malformed setup",
  "Server full",
};

void G_BuildTiccmd(ticcmd_t *cmd);

static void net_connection_handler(base_net_peer_t *peer) {
  if (SERVER) {
    net_peer_t *client = N_PeersAdd(peer);

    if (!client) {
      D_MsgLocalError("net_connection_handler: Adding new peer failed\n");
      I_NetPeerDisconnect(peer, DISCONNECT_REASON_SERVER_FULL);
      return;
    }
  }
  else {
    if (!CL_GetServer()) {
      D_MsgLocalWarn(
        "net_connection_handler: Got connection event but no connection was "
        "requested.\n"
      );
      return;
    }

    if (I_NetPeerGetPeer(peer) != CL_GetServer()) {
      D_MsgLocalWarn(
        "net_connection_handler: Got connection event from a non-server peer\n"
      );
      return;
    }

    CL_SetConnected();
  }
}

static void net_disconnection_handler(base_net_peer_t *peer,
                                      disconnection_reason_e reason) {
  net_peer_t *np = I_NetPeerGetPeer(peer);

  if (!np) {
    D_MsgLocalWarn(
      "net_disconnection_handler: Got disconnection event from unknown peer "
      "%s:%u.\n",
      I_NetPeerGetIPAddress(peer),
      I_NetPeerGetPort(peer)
    );
    return false;
  }

  if (SERVER) {
    D_MsgLocalInfo(
      "Client (%s:%u) disconnected: %s\n",
      I_NetPeerGetIPAddress(peer),
      I_NetPeerGetPort(peer),
      disconnection_reasons[reason]
    );

    N_PeerRemove(np);
  }
  else {
    if (!CL_GetServer()) {
      D_MsgLocalWarn(
        "net_disconnection_handler: Got disconnection event while not "
        "connected\n"
      );
      return;
    }

    if (I_NetPeerGetPeer(peer) != CL_GetServer()) {
      D_MsgLocalWarn(
        "net_disconnection_handler: Got disconnection event from a "
        "non-server peer\n"
      );
      return;
    }

    D_MsgLocalInfo(
      "Server (%s:%u) disconnected: %s\n",
      I_NetPeerGetIPAddress(peer),
      I_NetPeerGetPort(peer),
      disconnection_reasons[reason]
    );

    CL_Disconnect();

    /* [CG] [TODO] Set console or something */
  }

}

static void net_data_handler(base_net_peer_t *peer, unsigned char *data,
                                                    size_t size) {
  net_peer_t *np = I_NetPeerGetPeer(peer);

  if (!np) {
    D_MsgLocalWarn(
      "N_HandleNetData: Got data event from unknown peer %s:%u.\n",
      I_NetPeerGetIPAddress(peer),
      I_NetPeerGetPort(peer)
    );
    return;
  }

  N_HandlePacket(np, data, size);
}


static int run_tics(int tic_count) {
  int saved_tic_count = tic_count;

  while (tic_count--) {
    if (MULTINET) {
      if (!D_Wiping()) {
        if (G_GetGameState() == GS_LEVEL) {
          PL_BuildCommand();
        }
        else {
          I_InputHandle();
        }
      }
    }
    else {
      memset(&P_GetConsolePlayer()->cmd, 0, sizeof(ticcmd_t));
      G_BuildTiccmd(&P_GetConsolePlayer()->cmd);
    }
    N_RunTic();
  }

  return saved_tic_count;
}

static int process_tics(int tics_elapsed) {
  if (tics_elapsed <= 0)
    return 0;

  return run_tics(tics_elapsed);
}

static bool should_render(void) {
  if (nodrawers) {
    return false;
  }

  if (!window_focused) {
    return false;
  }

  if (gametic <= 0) {
    return false;
  }

#ifdef GL_DOOM
  if (V_GetMode() == VID_MODEGL) {
    return true;
  }
#endif

  if (!movement_smooth) {
    return false;
  }

  return true;
}

const char* N_GetDisconnectionReason(uint32_t reason) {
  if (reason >= DISCONNECT_REASON_MAX) {
    return "unknown";
  }

  return disconnection_reasons[reason];
}

void N_InitNetGame(void) {
  int i;
  time_t start_request_time;

  netgame   = false;
  solonet   = false;
  netserver = false;

  M_InitDeltas();

  N_MsgInit();
  PL_InitCommandQueues();

  if ((i = M_CheckParm("-solo-net"))) {
    netgame = true;
    solonet = true;
  }
  else if ((i = M_CheckParm("-connect"))) {
    char *host = NULL;
    unsigned short port = 0;

    if (i >= myargc) {
      I_Error("-connect requires an address");
    }

    netgame = true;

    N_Init();

    CL_Init();

    if (i < (myargc - 1)) {
      N_ParseAddressString(myargv[i + 1], &host, &port);
    }
    else {
      host = strdup("0.0.0.0");
      port = DEFAULT_PORT;
    }

    D_MsgLocalInfo(
      "N_InitNetGame: Connecting to server %s:%u...\n", host, port
    );

    for (int i = 0; i < MAX_SETUP_REQUEST_ATTEMPTS; i++) {
      if (!N_Connect(host, port)) {
        I_Error("N_InitNetGame: Connection refused");
      }

      D_MsgLocalInfo("N_InitNetGame: Connected!\n");

      if (!CL_GetServerPeer()) {
        I_Error("N_InitNetGame: Server peer was NULL");
      }

      G_ReloadDefaults();

      D_MsgLocalInfo("N_InitNetGame: Requesting setup information...\n");

      start_request_time = time(NULL);

      while (N_Connected()) {
        CL_SendSetupRequest();
        N_ServiceNetwork();

        if (CL_ReceivedSetup()) {
          break;
        }

        if (difftime(time(NULL), start_request_time) > 10.0) {
          break;
        }
      }

      if (CL_ReceivedSetup()) {
        break;
      }
    }

    if (!CL_ReceivedSetup()) {
      /*
       * [CG] [FIXME] This should just cancel the connection and print to the
       *              console.
       */
      I_Error("N_InitNetGame: Timed out waiting for setup information");
    }


    D_MsgLocalInfo("N_InitNetGame: Setup information received!\n");
  }
  else {
    if ((i = M_CheckParm("-serve"))) {
      netgame = true;
      netserver = true;
    }

    if (MULTINET) {
      char *host = NULL;
      unsigned short port = DEFAULT_PORT;

      nodrawers   = true;
      nosfxparm   = true;
      nomusicparm = true;

      N_Init();

      if (i < (myargc - 1)) {
        size_t host_length = N_ParseAddressString(myargv[i + 1], &host, &port);

        if (host_length == 0) {
          host = strdup("0.0.0.0");
        }
      }
      else {
        host = strdup("0.0.0.0");
      }

      if (!N_Listen(host, port)) {
        I_Error("Error listening on %s:%d\n", host, port);
      }

      D_MsgLocalInfo("N_InitNetGame: Listening on %s:%u.\n", host, port);
    }
  }
}

void N_RunTic(void) {
  if (advancedemo) {
    D_DoAdvanceDemo();
  }

  MN_Ticker();

  if ((!CL_RePredicting()) && (!CL_Synchronizing())) {
    I_GetTime_SaveMS();
  }

  G_Ticker();

  P_Checksum(gametic);

  if ((G_GetGameState() == GS_LEVEL) && SERVER && (gametic > 0)) {
    NETPEER_FOR_EACH(iter) {
      if (N_PeerSynchronized(iter.np)) {
        N_PeerSyncSetOutdated(iter.np);
      }
    }

    if (N_PeersGetCount() > 0) {
      G_SaveState();
    }

    N_UpdateSync();
  }

  gametic++;
}

void SV_DisconnectLaggedClients(void) {
  if (!SERVER) {
    return;
  }

  if (gamestate != GS_LEVEL) {
    return;
  }

  NETPEER_FOR_EACH(iter) {
    if (!N_PeerSynchronized(iter.np)) {
      continue;
    }

    if (N_PeerGetStatus(iter.np) != NETPEER_STATUS_PLAYER) {
      continue;
    }

    if (N_PeerTooLagged(iter.np)) {
      D_MsgLocalInfo("(%d) Player %u is too lagged\n",
        gametic,
        N_PeerGetPlayer(iter.np)->id
      );
      /*
       * [CG] [FIXME] Should demote this peer to spectator instead of
       *              disconnecting them
       */
      N_DisconnectPeer(iter.np, DISCONNECT_REASON_EXCESSIVE_LAG);
    }
  }
}

void SV_UpdatePings(void) {
  if (!SERVER) {
    return;
  }

  if ((gametic % (TICRATE / 2)) != 0) {
    return;
  }

  NETPEER_FOR_EACH(iter) {
    player_t *player = NULL;

    if (!N_PeerSynchronized(iter.np)) {
      continue;
    }

    player = N_PeerGetPlayer(iter.np);

    if (PL_IsConsolePlayer(player)) {
      continue;
    }

    SV_SendPing(iter.np);
  }
}

bool N_TryRunTics(void) {
  static int tics_built = 0;

  int tics_elapsed = I_GetTime() - tics_built;
  bool needs_rendering = should_render();

  if ((gametic > 0) &&
      (((tics_elapsed <= 0) && (!needs_rendering)) ||
       (SERVER && N_PeersGetCount() == 0))) {
    N_ServiceNetwork();
    C_ECIService();
    I_Sleep(1);
    return false;
  }

  if (tics_elapsed > 0) {
    tics_built += tics_elapsed;

    if (ffmap) {
      tics_elapsed++;
    }

    CL_CheckForStateUpdates();

    process_tics(tics_elapsed);

    SV_UpdatePings();
    SV_CleanupOldCommandsAndStates();
    SV_DisconnectLaggedClients();

    if (CLIENT && gametic > 0 && CL_GetServerPeer() == NULL) {
      PL_Echo(P_GetConsolePlayer(), "Server disconnected.");
      N_Disconnect(DISCONNECT_REASON_LOST_PEER_CONNECTION);
    }
  }

  N_ServiceNetwork();
  C_ECIService();

  if ((!SERVER) && (!nodrawers)) {
    if (!X_Call(X_GetState(), "console", "tick", 0, 0)) {
      I_Error("Error ticking console: %s\n", X_GetError(X_GetState()));
    }

    HU_Ticker();
    D_Display();
  }

  X_RunGC(X_GetState());

  return tics_elapsed > 0;
}

/* vi: set et ts=2 sw=2: */
