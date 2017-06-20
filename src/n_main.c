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
#include "i_net.h"
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

#include "n_main.h"
#include "n_msg.h"
#include "n_peer.h"
#include "n_proto.h"
#include "cl_cmd.h"
#include "cl_main.h"
#include "cl_net.h"

#define SERVER_NO_PEER_SLEEP_TIMEOUT 20
#define SERVER_SLEEP_TIMEOUT 1

bool netgame   = false;
bool solonet   = false;
bool netserver = false;

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
        if (G_GetGameState() == gamestate_level) {
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

void N_InitNetGame(void) {
  int i;

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
    if (i >= myargc) {
      I_Error("-connect requires an address, i.e. 12.88.97.14:10666");
    }

    netgame = true;

    CL_Init();
    CL_Connect(myargv[i + 1]);
  }
  else if ((i = M_CheckParm("-serve"))) {
    netgame     = true;
    netserver   = true;
    nodrawers   = true;
    nosfxparm   = true;
    nomusicparm = true;

    if (i < (myargc - 1)) {
      SV_Listen(strdup(myargv[1 + 1]));
    }
    else {
      SV_Listen(strdup("0.0.0.0:10666"));
    }
  }
}

base_net_peer_t* N_Connect(const char *host, uint16_t port) {
  base_netpeer_t *bnp = I_NetConnect(host, port, net_connection_handler,
                                                 net_disconnection_handler,
                                                 net_data_handler);
  if (!bnp) {
    D_MsgLocalError("N_Connect: Connection failed\n");
    return NULL;
  }

  return bnp;
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

  if ((G_GetGameState() == gamestate_level) && SERVER && (gametic > 0)) {
    NET_PEER_FOR_EACH(iter) {
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

void N_ServiceNetworkTimeout(int timeout_ms) {
  NET_PEERS_FOR_EACH(iter) {
    N_ComFlushChannels(iter.np->as.server.link.com);
  }
  I_NetServiceNetworkTimeout(timeout_ms);
}

void N_ServiceNetwork(void) {
  N_ServiceNetworkTimeout(0);
}

static inline void should_short_circuit_main_loop(int tics_elapsed,
                                                  bool needs_rendering) {
  if (gametic < 0) {
    return false;
  }

  if (SERVER && N_PeersGetCount() == 0) {
    return true;
  }

  if (tics_elapsed > 0) {
    return false;
  }

  if (needs_rendering) {
    return false;
  }

  return true;
}

bool N_TryRunTics(void) {
  static int tics_built = 0;

  int tics_elapsed = I_GetTime() - tics_built;
  bool needs_rendering = should_render();

  if (should_short_circuit_main_loop(tics_elapsed, needs_rendering)) {
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

    if (CLIENT && (gametic > 0) && (!CL_GetServerPeer())) {
      D_MsgLocalInfo("Server disconnected\n");
      I_NetDisconnect();
    }
  }

  N_ServiceNetwork();
  N_PeersCheckTimeouts();
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
