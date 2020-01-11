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

// #define LOG_SECTOR 43

#define USE_RANGE_CODER 1
#define MAX_DOWNLOAD 0
#define MAX_UPLOAD 0

#define SERVER_NO_PEER_SLEEP_TIMEOUT 20
#define SERVER_SLEEP_TIMEOUT 1
#define MAX_SETUP_REQUEST_ATTEMPTS 10

static ENetHost   *net_host = NULL;
static const char *previous_host = NULL;
static uint16_t    previous_port = 0;

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

  if (!movement_smooth) {
    return false;
  }

  return true;
}

void N_Init(void) {
  if (enet_initialize() != 0) {
    I_Error("Error initializing ENet");
  }

  N_PeersInit();
  G_InitStates();
  atexit(N_Shutdown);
}

void N_Disconnect(disconnection_reason_e reason) {
  ENetEvent net_event;

  if (!net_host) {
    return;
  }

  NETPEER_FOR_EACH(iter) {
    D_MsgLocalInfo("Disconnecting peer %d\n", N_PeerGetID(iter.np));
    N_PeerDisconnect(iter.np, reason);
  }

  while (true) {
    int res = enet_host_service(
      net_host, &net_event, NET_DISCONNECT_TIMEOUT * 1000
    );

    if (res > 0) {
      netpeer_t *np = N_PeersLookupByENetPeer(net_event.peer);

      if (!np) {
        D_MsgLocalWarn(
          "N_Disconnect: Received network event from unknown peer %s:%u\n",
          N_IPToConstString(ENET_NET_TO_HOST_32(net_event.peer->address.host)),
          net_event.peer->address.port
        );
        continue;
      }

      if (net_event.type == ENET_EVENT_TYPE_DISCONNECT) {
        N_PeerRemove(np);
      }
    }
    else {
      if (res < 0) {
        D_MsgLocalWarn("N_Disconnect: Unknown error disconnecting\n");
      }
      break;
    }
  }

  NETPEER_FOR_EACH(iter) {
    N_PeerSendReset(iter.np);
    N_PeerIterRemove(&iter);
  }

  memset(&net_event, 0, sizeof(ENetEvent));

  enet_host_destroy(net_host);
  net_host = NULL;
}

void N_Shutdown(void) {
  D_MsgLocalInfo("N_Shutdown: shutting down\n");
  N_Disconnect(DISCONNECT_REASON_MANUAL);

  enet_deinitialize();
}

bool N_Listen(const char *host, uint16_t port) {
  ENetAddress address;

  if (host) {
    enet_address_set_host(&address, host);
  }
  else {
    address.host = ENET_HOST_ANY;
  }

  if (port != 0) {
    address.port = port;
  }
  else {
    address.port = DEFAULT_PORT;
  }

  net_host = enet_host_create(
    &address, MAX_CLIENTS, 2, MAX_DOWNLOAD, MAX_UPLOAD
  );
  
  if (!net_host) {
    D_MsgLocalError("N_Listen: Error creating host on %s:%u\n", host, port);
    return false;
  }

#if USE_RANGE_CODER
  if (enet_host_compress_with_range_coder(net_host) < 0) {
    D_MsgLocalError("N_Listen: Error activating range coder\n");
    return false;
  }
#endif

  return true;
}

bool N_Connect(const char *host, uint16_t port) {
  ENetPeer *server = NULL;
  ENetAddress address;

  net_host = enet_host_create(NULL, 1, 2, 0, 0);

  if (!net_host) {
    D_MsgLocalError("N_Connect: Error creating host\n");
    return false;
  }

#if USE_RANGE_CODER
  if (enet_host_compress_with_range_coder(net_host) < 0) {
    D_MsgLocalError("N_Connect: Error activating range coder\n");
    return false;
  }
#endif

  enet_address_set_host(&address, host);

  if (port != 0) {
    address.port = port;
  }
  else {
    address.port = DEFAULT_PORT;
  }

  server = enet_host_connect(net_host, &address, 2, 0);

  if (!server) {
    D_MsgLocalError("N_Connect: Error connecting to server\n");
    N_Disconnect(DISCONNECT_REASON_CONNECTION_ERROR);
    return false;
  }

  N_PeersAdd(server);

  previous_host = host;
  previous_port = port;

  return true;
}

bool N_Connected(void) {
  return (net_host != NULL);
}

bool N_Reconnect(void) {
  if ((!previous_host) || (previous_port == 0)) {
    D_MsgLocalInfo("No previous connection\n");
    return false;
  }

  return N_Connect(previous_host, previous_port);
}

void N_WaitForPacket(int ms) {
  ENetEvent net_event;

  enet_host_service(net_host, &net_event, ms);
}

bool N_ConnectToServer(const char *address) {
  char *host = NULL;
  uint16_t port = 0;
  bool connected = false;

  N_ParseAddressString(address, &host, &port);

  if (port == 0) {
    port = DEFAULT_PORT;
  }

  D_MsgLocalInfo("Connecting to server");
  connected = N_Connect(host, port);

  free(host);

  if (connected) {
    D_MsgLocalInfo("Connected");
  }
  else {
    D_MsgLocalError("Connection failed");
  }

  return connected;
}

void N_DisconnectPeer(netpeer_t *np, disconnection_reason_e reason) {
  D_MsgLocalInfo("N_DisconnectPeer: Disconnecting peer %u\n", N_PeerGetID(np));
  N_PeerDisconnect(np, reason);
}

void N_DisconnectPlayerID(uint32_t player_id, disconnection_reason_e reason) {
  player_t *player = P_PlayersLookup(player_id);
  
  if (!player) {
    D_MsgLocalWarn("N_DisconnectPlayerID: No player for ID %u\n", player_id);
    return;
  }

  N_DisconnectPlayer(player, reason);
}

void N_DisconnectPlayer(player_t *player, disconnection_reason_e reason) {
  netpeer_t *np = N_PeersLookupByPlayer(player);

  if (!np) {
    D_MsgLocalWarn("N_DisconnectPlayer: No peer for player %u\n", player->id);
    return;
  }

  D_MsgLocalInfo("N_DisconnectPlayer: Disconnecting player %u\n", player->id);
  N_DisconnectPeer(np, reason);
}

static void handle_enet_connection(ENetEvent *net_event) {
  netpeer_t *np;

  if (SERVER) {
    np = N_PeersAdd(net_event->peer);

    if (!np) {
      D_MsgLocalError("N_ServiceNetwork: Adding new peer failed\n");
      enet_peer_disconnect(net_event->peer, DISCONNECT_REASON_SERVER_FULL);
      return;
    }
  }
  else {
    np = CL_GetServerPeer();

    if (!np) {
      D_MsgLocalWarn(
        "N_ServiceNetwork: Got 'connect' event but no connection "
        "was requested.\n"
      );
      return;
    }

    N_PeerSetConnected(np);
  }
}

static void handle_enet_disconnection(ENetEvent *net_event) {
  netpeer_t *np = N_PeersLookupByENetPeer(net_event->peer);

  if (!np) {
    D_MsgLocalWarn(
      "N_ServiceNetwork: Got 'disconnect' event from unknown peer %s:%u.\n",
      N_IPToConstString(ENET_NET_TO_HOST_32(net_event->peer->address.host)),
      net_event->peer->address.port
    );
    return;
  }

  if (CLIENT) {
    N_Disconnect(DISCONNECT_REASON_GOT_PEER_DISCONNECTION);
  }

  if (net_event->data >= DISCONNECT_REASON_MAX) {
    D_MsgLocalInfo("Peer disconnected: Reason unknown (%u)\n",
      net_event->data
    );
  }
  else {
    D_MsgLocalInfo("Peer disconnected: %s\n",
      disconnection_reasons[net_event->data]
    );
  }

  N_PeerRemove(np);
}

static void handle_enet_receive(ENetEvent *net_event) {
  netpeer_t *np = N_PeersLookupByENetPeer(net_event->peer);

  if (!np) {
    D_MsgLocalWarn(
      "N_ServiceNetwork: Got 'packet' event from unknown peer %s:%u.\n",
      N_IPToConstString(ENET_NET_TO_HOST_32(
        net_event->peer->address.host
      )),
      net_event->peer->address.port
    );
    return;
  }

  if ((net_event->packet->data[0] & 0x80) == 0x80) {
    N_HandlePacket(
      np, net_event->packet->data, net_event->packet->dataLength
    );
  }

  enet_packet_destroy(net_event->packet);
}

void N_ServiceNetworkTimeout(int timeout_ms) {
  int status = 0;
  ENetEvent net_event;

  if (!net_host) {
    return;
  }

  NETPEER_FOR_EACH(iter) {
    if (N_PeerCheckTimeout(iter.np)) {
      D_MsgLocalInfo("Peer %s:%u timed out.\n",
        N_PeerGetIPAddressConstString(iter.np),
        N_PeerGetPort(iter.np)
      );
      N_PeerSendReset(iter.np);
      N_PeerIterRemove(&iter);
      continue;
    }

    N_PeerFlushChannels(iter.np);
  }

  while (net_host) {
    status = enet_host_service(net_host, &net_event, timeout_ms);

    /*
     * [CG]: ENet says this must be cleared, so do it right away here at the
     *       top since we don't use it.
     */
    if (net_event.peer && net_event.peer->data) {
      net_event.peer->data = NULL;
    }

    if (status == 0) {
      break;
    }

    if (status < 0) {
      D_MsgLocalInfo(
        "N_ServiceNetwork: Unknown error occurred while servicing host\n"
      );
      break;
    }

    switch (net_event.type) {
      case ENET_EVENT_TYPE_CONNECT:
        handle_enet_connection(&net_event);
      break;
      case ENET_EVENT_TYPE_DISCONNECT:
        handle_enet_disconnection(&net_event);
      break;
      case ENET_EVENT_TYPE_RECEIVE:
        handle_enet_receive(&net_event);
      break;
      default:
        D_MsgLocalWarn(
          "N_ServiceNetwork: Got unknown event from peer %s:%u.\n",
          N_IPToConstString(ENET_NET_TO_HOST_32(net_event.peer->address.host)),
          net_event.peer->address.port
        );
      break;
    }

    if (timeout_ms != 0) {
      break;
    }
  }
}

const char* N_GetDisconnectionReason(uint32_t reason) {
  if (reason >= DISCONNECT_REASON_MAX) {
    return "unknown";
  }

  return disconnection_reasons[reason];
}

void N_ServiceNetwork(void) {
  N_ServiceNetworkTimeout(0);
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
