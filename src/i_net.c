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

#include "i_net.h"

#define USE_RANGE_CODER 1
#define MAX_DOWNLOAD 0
#define MAX_UPLOAD 0

#define NET_CHANNEL_RELIABLE 0
#define NET_CHANNEL_UNRELIABLE 1

#define NET_THROTTLE_INTERVAL 300
#define NET_THROTTLE_ACCEL 2
#define NET_THROTTLE_DECEL 1

static ENetHost   *net_host = NULL;
static const char *previous_host = NULL;
static uint16_t    previous_port = 0;

static net_connection_handler_f handle_net_connection = NULL;
static net_disconnection_handler_f handle_net_disconnection = NULL;
static net_data_handler_f handle_net_data = NULL;

void CL_SetConnected(void) {
  N_PeerSetConnected(CL_GetServerPeer());
}

void CL_Disconnect(void) {
  N_PeerRemove(CL_GetServerPeer());
  I_NetDisconnect();
}

void I_NetInit(void) {
  if (enet_initialize() != 0) {
    I_Error("I_NetInit: Error initializing ENet");
  }

  atexit(I_NetShutdown);
}

void I_NetSetConnectionHandler(net_connection_handler_f handler) {
  net_connection_handler = handler;
}

void I_NetSetDisconnectionHandler(net_disconnection_handler_f handler) {
  net_disconnection_handler = handler;
}

void I_NetSetDataHandler(net_data_handler_f handler) {
  net_data_handler = handler;
}

static void remove_disconnecting_peer(base_net_peer_t *peer,
                                      disconnection_reason_e reason) {
  net_peer_t *np = I_NetPeerGetPeer(peer);

  if (!np) {
    return;
  }

  N_PeerRemove(np);
}

void SV_Disconnect(disconnection_reason_e reason) {
  /*
   * - Send a base disconnection to all connected peers and wait for them to
   *   disconnect.
   * - As they disconnect, remove them
   * - For each remaining peer:
   *   - `enet_peer_reset`
   *   - `N_PeerRemove`
   * - I_NetShutdown
   */

  I_NetSetConnectionHandler(NULL);
  I_NetSetDisconnectionHandler(remove_disconnecting_peer);
  I_NetSetDataHandler(NULL);

  NET_PEER_FOR_EACH(iter) {
    I_NetPeerDisconnect(iter.np->link.com.base_net_peer);
  }

  I_NetServiceNetworkTimeout(NET_DISCONNECT_TIMEOUT * 1000);

  I_NetDisconnect();
}

void I_NetDisconnect(void) {
  if (!net_host) {
    return;
  }

  enet_host_destroy(net_host);
  net_host = NULL;

  I_NetSetConnectionHandler(NULL);
  I_NetSetDisconnectionHandler(NULL);
  I_NetSetDataHandler(NULL);
}

void I_NetShutdown(void) {
  enet_deinitialize();
}

void N_Shutdown(void) {
  D_MsgLocalInfo("N_Shutdown: shutting down\n");
  N_Disconnect(DISCONNECT_REASON_MANUAL);
}

bool I_NetListen(const char *host,
                 uint16_t port,
                 net_connection_handler_f net_connection_handler,
                 net_disconnection_handler_f net_disconnection_handler,
                 net_data_handler_f net_data_handler) {
  ENetAddress address;

  if (host) {
    enet_address_set_host(&address, host);
  }
  else {
    address.host = ENET_HOST_ANY;
  }

  if (port) {
    address.port = port;
  }
  else {
    address.port = DEFAULT_PORT;
  }

  net_host = enet_host_create(&address, MAX_CLIENTS, 2, 0, 0);
  
  if (!net_host) {
    D_MsgLocalError("I_NetListen: Error creating host on %s:%u\n", host, port);
    return false;
  }

#if USE_RANGE_CODER
  if (enet_host_compress_with_range_coder(net_host) < 0) {
    D_MsgLocalError("I_NetListen: Error activating range coder\n");
    return false;
  }
#endif

  handle_net_connection = net_connection_handler;
  handle_net_disconnection = net_disconnection_handler;
  handle_net_data = net_data_handler;

  return true;
}

bool I_NetConnect(const char *host,
                  uint16_t port,
                  net_connection_handler_f net_connection_handler,
                  net_disconnection_handler_f net_disconnection_handler,
                  net_data_handler_f net_data_handler) {
  ENetPeer *server = NULL;
  ENetAddress address;

  net_host = enet_host_create(NULL, 1, 2, 0, 0);

  if (!net_host) {
    D_MsgLocalError("I_NetConnect: Error creating host\n");
    return false;
  }

#if USE_RANGE_CODER
  if (enet_host_compress_with_range_coder(net_host) < 0) {
    D_MsgLocalError("I_NetConnect: Error activating range coder\n");
    return false;
  }
#endif

  enet_address_set_host(&address, host);

  if (port) {
    address.port = port;
  }
  else {
    address.port = DEFAULT_PORT;
  }

  server = enet_host_connect(net_host, &address, 2, 0);

  if (!server) {
    D_MsgLocalError("I_NetConnect: Error connecting to server\n");
    N_Disconnect(DISCONNECT_REASON_CONNECTION_ERROR);
    return false;
  }

  N_PeersAdd(server);

  previous_host = host;
  previous_port = port;

  handle_net_connection = net_connection_handler;
  handle_net_disconnection = net_disconnection_handler;
  handle_net_data = net_data_handler;

  return true;
}

bool I_NetConnected(void) {
  return (net_host != NULL);
}

bool I_NetReconnect(void) {
  net_connection_handler_f connection_handler = handle_net_connection;
  net_disconnection_handler_f disconnection_handler = handle_net_disconnection;
  net_data_handler_f data_handler = handle_net_data;

  if ((!previous_host) || (previous_port == 0)) {
    D_MsgLocalInfo("No previous connection\n");
    return false;
  }

  if (I_NetConnected()) {
    I_NetDisconnect(DISCONNECT_REASON_MANUAL);
  }

  return I_NetConnect(
    previous_host,
    previous_port,
    connection_handler,
    disconnection_handler,
    data_handler
  );
}

void I_NetWaitForPacket(int ms) {
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

void N_PeersCheckTimeouts(void) {
  NETPEER_FOR_EACH(iter) {
    if (SERVER) {
      if (iter.np->type != PEER_TYPE_CLIENT) {
        continue;
      }

      if (!N_LinkCheckTimeout(&iter.np->as.client.link)) {
        continue;
      }

      D_MsgLocalInfo("Client %s:%u timed out.\n",
        N_ComGetIPAddress(&iter.np->as.client.link.com),
        N_ComGetPort(&iter.np->as.client.link.com),
      );
      N_ComSendReset(&iter.np->as.client.link.com);
      N_PeerIterateRemove(&iter);
    }

    if (CLIENT) {
      if (iter.np->type != PEER_TYPE_SERVER) {
        continue;
      }

      if (!N_LinkCheckTimeout(&iter.np->as.server.link)) {
        continue;
      }

      D_MsgLocalInfo("Server (%s:%u) timed out.\n"
        N_ComGetIPAddress(&iter.np->as.server.link.com),
        N_ComGetPort(&iter.np->as.server.link.com),
      );
      N_ComSendReset(&iter.np->as.server.link.com);
      N_PeerIterateRemove(&iter);
    }
  }
}

void N_ServiceNetworkTimeout(int timeout_ms) {
  N_PeersCheckTimeouts();
  NETPEER_FOR_EACH(iter) {
    N_ComFlushChannels(iter.np->as.server.link.com);
  }
  I_NetServiceNetworkTimeout(timeout_ms);
}

void I_NetServiceNetworkTimeout(int timeout_ms) {
  int status = 0;
  ENetEvent net_event;

  if (!net_host) {
    return;
  }

  while (net_host) {
    status = enet_host_service(net_host, &net_event, timeout_ms);

    if (status == 0) {
      break;
    }

    if (status < 0) {
      D_MsgLocalInfo(
        "I_NetServiceNetworkTimeout: Unknown error occurred while servicing "
        "host\n"
      );
      break;
    }

    switch (net_event.type) {
      case ENET_EVENT_TYPE_CONNECT:
        handle_net_connection(net_event.peer);
      break;
      case ENET_EVENT_TYPE_DISCONNECT:
        if (net_event->data >= DISCONNECT_REASON_MAX) {
          D_MsgLocalInfo("Peer disconnected: Reason unknown (%u)\n",
            net_event->data
          );
        }
        else {
          handle_net_disconnection(net_event.peer, net_event.data);
        }
      break;
      case ENET_EVENT_TYPE_RECEIVE:
        handle_net_data(
          net_event.peer, net_event.packet->data, net_event.packet->dataLength
        );
        enet_packet_destroy(net_event.packet);
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

void N_ServiceNetwork(void) {
  N_ServiceNetworkTimeout(0);
}

void I_NetPeerInit(base_netpeer_t *peer) {
  if (SERVER) {
    enet_peer_throttle_configure(
      (ENetPeer *)peer,
      NET_THROTTLE_INTERVAL,
      NET_THROTTLE_ACCEL,
      NET_THROTTLE_DECEL
    );
  }
}

void I_NetPeerSendPacket(base_netpeer_t *peer, netpacket_t *packet) {
  ENetPeer *epeer = (ENetPeer *)peer;
  ENetPacket *epacket = (ENetPacket *)packet;

  if (epacket->flags & ENET_PACKET_FLAG_RELIABLE) {
    enet_peer_send(peer, NET_CHANNEL_RELIABLE, epacket);
  }
  else {
    enet_peer_send(peer, NET_CHANNEL_UNRELIABLE, epacket);
  }
}

void I_NetPeerSendReset(base_netpeer_t *peer) {
  enet_peer_reset((ENetPeer *)peer);
}

uint32_t I_NetPeerGetIPAddress(base_netpeer_t *peer) {
  ENetPeer *epeer = (ENetPeer *)peer;

  return ENET_NET_TO_HOST_32(epeer->address.host);
}

uint16_t I_NetPeerGetPort(base_netpeer_t *peer) {
  ENetPeer *epeer = (ENetPeer *)peer;

  return epeer->address.port;
}

float I_NetPeerGetPacketLoss(base_netpeer_t *peer) {
  ENetPeer *epeer = (ENetPeer *)peer;

  return epeer->packetLoss / (float)ENET_PEER_PACKET_LOSS_SCALE;
}

float I_NetPeerGetPacketLossJitter(base_netpeer_t *peer) {
  ENetPeer *epeer = N_LinkGetENetPeer(nl);

  return epeer->packetLossVariance / (float)ENET_PEER_PACKET_LOSS_SCALE;
}

void I_NetPeerDisconnect(base_net_peer_t *peer,
                         disconnection_reason_e reason) {
  ENetPeer *epeer = (ENetPeer *)peer;

  enet_peer_disconnect(epeer, reason);
}

net_peer_t* I_NetPeerGetPeer(base_net_peer_t *peer) {
  ENetPeer *epeer = (ENetPeer *)peer;

  return (net_peer_t *)epeer->data;
}

netpacket_t* I_NetPacketNewReliable(size_t size) {
  netpacket_t *packet = (netpacket_t *)enet_packet_create(
    NULL, size, ENET_PACKET_FLAG_RELIABLE
  );

  if (!packet) {
    I_Error(
      "I_NetPacketNewReliable: Error allocating memory for new packet\n"
    );
  }

  return packet;
}

netpacket_t* I_NetPacketNewUnreliable(size_t size) {
  netpacket_t *packet = (netpacket_t *)enet_packet_create(
    NULL, size, ENET_PACKET_FLAG_UNSEQUENCED
  );

  if (!packet) {
    I_Error(
      "I_NetPacketNewUnreliable: Error allocating memory for new packet\n"
    );
  }

  return packet;
}

unsigned char* I_NetPacketGetData(netpacket_t *packet) {
  ENetPacket *epacket = (ENetPacket *)packet;

  return epacket->data;
}

base_net_peer_t* I_NetEventGetPeer(netevent_t *event) {
  ENetEvent *eevent = (ENetEvent *)event;

  return (base_net_peer_t *)eevent->peer;
}

uint32_t I_NetEventGetData(netevent_t *event) {
  ENetEvent *eevent = (ENetEvent *)event;

  return eevent->data;
}

netpacket_t* I_NetEventGetData(netevent_t *event) {
  ENetEvent *eevent = (ENetEvent *)event;

  return (netpacket_t *)eevent->packet;
}

bool I_NetEventIsConnection(netevent_t *event) {
  ENetEvent *eevent = (ENetEvent *)event;

  return eevent->type == ENET_EVENT_TYPE_CONNECT;
}

bool I_NetEventIsDisconnection(netevent_t *event) {
  ENetEvent *eevent = (ENetEvent *)event;

  return eevent->type == ENET_EVENT_TYPE_DISCONNECT;
}

bool I_NetEventIsData(netevent_t *event) {
  ENetEvent *eevent = (ENetEvent *)event;

  return eevent->type == ENET_EVENT_TYPE_RECEIVE;
}

/* vi: set et ts=2 sw=2: */
