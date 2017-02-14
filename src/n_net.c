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
#include "doomstat.h"
#include "d_event.h"

#include "c_main.h"
#include "g_game.h"
#include "g_state.h"
#include "i_main.h"
#include "i_system.h"
#include "m_swap.h"
#include "n_main.h"
#include "p_user.h"
#include "cl_main.h"
#include "cl_net.h"

#define USE_RANGE_CODER 1
#define MAX_DOWNLOAD 0
#define MAX_UPLOAD 0

static ENetHost   *net_host = NULL;
static const char *previous_host = NULL;
static uint16_t    previous_port = 0;

bool netgame   = false;
bool solonet   = false;
bool netserver = false;

void N_Init(void) {
  if (enet_initialize() != 0) {
    I_Error("Error initializing ENet");
  }

  N_InitPeers();
  G_InitStates();
  atexit(N_Shutdown);
}

void N_Disconnect(void) {
  ENetEvent net_event;

  if (!net_host) {
    return;
  }

  NETPEER_FOR_EACH(iter) {
    D_Msg(MSG_INFO, "Disconnecting peer %d\n", N_PeerGetPlayernum(iter.np));
    N_PeerDisconnect(iter.np);
  }

  while (true) {
    int res = enet_host_service(
      net_host, &net_event, NET_DISCONNECT_TIMEOUT * 1000
    );

    if (res > 0) {
      netpeer_t *np = N_PeerForPeer(net_event.peer);

      if (!np) {
        D_Msg(MSG_WARN,
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
        D_Msg(MSG_WARN, "N_Disconnect: Unknown error disconnecting\n");
      }
      break;
    }
  }

  NETPEER_FOR_EACH(iter) {
    N_PeerSendReset(iter.np);
    N_PeerIterRemove(iter.it, iter.np);
  }

  memset(&net_event, 0, sizeof(ENetEvent));

  enet_host_destroy(net_host);
  net_host = NULL;
}

void N_Shutdown(void) {
  D_Msg(MSG_INFO, "N_Shutdown: shutting down\n");
  N_Disconnect();

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
    &address, MAXPLAYERS, NET_CHANNEL_MAX, MAX_DOWNLOAD, MAX_UPLOAD
  );
  
  if (!net_host) {
    D_Msg(MSG_ERROR, "N_Listen: Error creating host on %s:%u\n", host, port);
    return false;
  }

#if USE_RANGE_CODER
  if (enet_host_compress_with_range_coder(net_host) < 0) {
    D_Msg(MSG_ERROR, "N_Listen: Error activating range coder\n");
    return false;
  }
#endif

  return true;
}

bool N_Connect(const char *host, uint16_t port) {
  netpeer_t *np = NULL;
  ENetPeer *server = NULL;
  ENetAddress address;

  net_host = enet_host_create(NULL, 1, NET_CHANNEL_MAX, 0, 0);

  if (!net_host) {
    D_Msg(MSG_ERROR, "N_Connect: Error creating host\n");
    return false;
  }

#if USE_RANGE_CODER
  if (enet_host_compress_with_range_coder(net_host) < 0) {
    D_Msg(MSG_ERROR, "N_Connect: Error activating range coder\n");
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

  np = N_PeerAdd(server);

  server = enet_host_connect(net_host, &address, NET_CHANNEL_MAX, 0);

  if (!server) {
    D_Msg(MSG_ERROR, "N_Connect: Error connecting to server\n");
    N_Disconnect();
    return false;
  }

  previous_host = host;
  previous_port = port;

  D_Msg(MSG_INFO, "N_Connect: connected");

  return true;
}

bool N_Reconnect(void) {
  if ((!previous_host) || (previous_port == 0)) {
    D_Msg(MSG_INFO, "No previous connection\n");
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

  D_Msg(MSG_INFO, "Connecting to server");
  connected = N_Connect(host, port);

  free(host);

  if (connected) {
    D_Msg(MSG_INFO, "Connected");
  }
  else {
    D_Msg(MSG_ERROR, "Connection failed");
  }

  return connected;
}

void N_DisconnectPeer(netpeer_t *np) {
  D_Msg(MSG_INFO, "N_DisconnectPeer: Disconnecting peer %d\n",
    N_PeerGetPlayernum(np)
  );
  N_PeerDisconnect(np);
}

void N_DisconnectPlayer(unsigned short playernum) {
  netpeer_t *np = N_PeerForPlayer(playernum);

  if (!np) {
    I_Error("N_DisconnectPlayer: Invalid player %d.\n", playernum);
  }

  D_Msg(MSG_INFO, "N_DisconnectPlayer: Disconnecting player %d\n", playernum);
  N_DisconnectPeer(np);
}

void N_FlushNetwork(void) {
  if (!net_host) {
    return;
  }

  enet_host_flush(net_host);
}

static void handle_enet_connection(ENetEvent *net_event) {
  netpeer_t *np;

  if (SERVER) {
    np = N_PeerAdd(net_event->peer);

    if (!np) {
      D_Msg(MSG_ERROR, "N_ServiceNetwork: Adding new peer failed\n");
      return;
    }
  }
  else {
    np = CL_GetServerPeer();

    if (!np) {
      D_Msg(MSG_WARN,
        "N_ServiceNetwork: Got 'connect' event but no connection "
        "was requested.\n"
      );
      return;
    }

    N_PeerSetConnected(np);
  }
}

static void handle_enet_disconnection(ENetEvent *net_event) {
  netpeer_t *np = N_PeerForPeer(net_event->peer);

  if (!np) {
    D_Msg(MSG_WARN,
      "N_ServiceNetwork: Got 'disconnect' event from unknown peer %s:%u.\n",
      N_IPToConstString(ENET_NET_TO_HOST_32(net_event->peer->address.host)),
      net_event->peer->address.port
    );
    return;
  }

  if (CLIENT) {
    N_Disconnect();
    I_SafeExit(0);
  }

  N_PeerRemove(np);
}

static void handle_enet_receive(ENetEvent *net_event) {
  netpeer_t *np = N_PeerForPeer(net_event->peer);

  if (!np) {
    D_Msg(MSG_WARN,
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
      D_Msg(MSG_INFO, "Peer %s:%u timed out.\n",
        N_PeerGetIPAddressConstString(iter.np),
        N_PeerGetPort(iter.np)
      );
      N_PeerSendReset(iter.np);
      N_PeerIterRemove(iter.it, iter.np);
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
      D_Msg(MSG_INFO,
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
        D_Msg(MSG_WARN,
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

void N_ServiceNetwork(void) {
  N_ServiceNetworkTimeout(0);
}

/* vi: set et ts=2 sw=2: */
