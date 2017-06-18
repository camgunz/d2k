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

struct net_peer_s;
typedef struct net_peer_s net_peer_t;

#define USE_RANGE_CODER 1
#define NET_MAX_DOWNLOAD 0
#define NET_MAX_UPLOAD 0

#define NET_CHANNEL_RELIABLE 0
#define NET_CHANNEL_UNRELIABLE 1

#define NET_THROTTLE_INTERVAL 300
#define NET_THROTTLE_ACCEL 2
#define NET_THROTTLE_DECEL 1

#define MAX_ADDRESS_LENGTH 500

#define to_uchar(x)         ((unsigned char)((x) & 0xFF))
#define to_ushort(x)        ((unsigned short)((x) & 0xFFFF))
#define string_to_ushort(x) (to_ushort(strtol(x, NULL, 10)))

static ENetHost   *net_host = NULL;
static const char *previous_host = NULL;
static uint16_t    previous_port = 0;

static net_connection_handler_f *handle_net_connection = NULL;
static net_disconnection_handler_f *handle_net_disconnection = NULL;
static net_data_handler_f *handle_net_data = NULL;

const char *disconnection_reasons[DISCONNECT_REASON_MAX] = {
  "Lost peer connection",
  "Peer disconnected",
  "Manual disconnection",
  "Connection error",
  "Excessive lag",
  "Malformed setup",
  "Server full",
};

void I_NetInit(void) {
  if (enet_initialize() != 0) {
    I_Error("I_NetInit: Error initializing ENet");
  }

  atexit(enet_deinitialize);
}

bool I_NetListen(const char *host,
                 uint16_t port,
                 net_connection_handler_f net_connection_handler,
                 net_disconnection_handler_f net_disconnection_handler,
                 net_data_handler_f net_data_handler) {
  ENetAddress address;

  enet_address_set_host(&address, host);
  address.port = port;

  net_host = enet_host_create(
    &address, NET_MAX_CLIENTS, 2, NET_MAX_DOWNLOAD, NET_MAX_UPLOAD
  );

  if (!net_host) {
    D_MsgLocalError("I_NetListen: Error creating host on %s:%u\n", host, port);
    return false;
  }

#if USE_RANGE_CODER
  if (enet_host_compress_with_range_coder(net_host) < 0) {
    D_MsgLocalError("I_NetListen: Error activating range coder\n");
    I_NetReset();
    return false;
  }
#endif

  handle_net_connection = net_connection_handler;
  handle_net_disconnection = net_disconnection_handler;
  handle_net_data = net_data_handler;

  return true;
}

base_netpeer_t* I_NetConnect(const char *host,
                             uint16_t port,
                             net_connection_handler_f conn_handler,
                             net_disconnection_handler_f disconn_handler,
                             net_data_handler_f data_handler) {
  ENetPeer *peer = NULL;
  ENetAddress address;

  net_host = enet_host_create(NULL, 1, 2, NET_MAX_DOWNLOAD, NET_MAX_UPLOAD);

  if (!net_host) {
    D_MsgLocalError("I_NetConnect: Error creating host\n");
    return NULL;
  }

#if USE_RANGE_CODER
  if (enet_host_compress_with_range_coder(net_host) < 0) {
    D_MsgLocalError("I_NetConnect: Error activating range coder\n");
    I_NetReset();
    return NULL;
  }
#endif

  enet_address_set_host(&address, host);
  address.port = port;
  peer = enet_host_connect(net_host, &address, 2, 0);

  if (!peer) {
    D_MsgLocalError("I_NetConnect: Connection failed\n");
    I_NetReset();
    return NULL;
  }

  previous_host = host;
  previous_port = port;

  handle_net_connection = conn_handler;
  handle_net_disconnection = disconn_handler;
  handle_net_data = data_handler;

  return peer;
}

bool I_NetReconnect(void) {
  net_connection_handler_f *conn_handler = handle_net_connection;
  net_disconnection_handler_f *disconn_handler = handle_net_disconnection;
  net_data_handler_f *data_handler = *handle_net_data;

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
    conn_handler,
    disconn_handler,
    data_handler
  );
}

bool I_NetConnected(void) {
  return (net_host != NULL);
}

void I_NetSetConnectionHandler(net_connection_handler_f handler) {
  handle_net_connection = handler;
}

void I_NetSetDisconnectionHandler(net_disconnection_handler_f handler) {
  handle_net_disconnection = handler;
}

void I_NetSetDataHandler(net_data_handler_f handler) {
  handle_net_data = handler;
}

void I_NetDisconnect(disconnection_reason_e reason) {
  for (size_t i = 0; i < net_host->peerCount; i++) {
    enet_peer_disconnect(&net_host->peers[i], reason);
  }

  I_NetServiceNetworkTimeout(NET_DISCONNECT_WAIT);

  for (size_t i = 0; i < net_host->peerCount; i++) {
    enet_peer_disconnect_now(&net_host->peers[i], reason);
  }

  I_NetReset();
}

void I_NetReset(void) {
  enet_host_destroy(net_host);
  net_host = NULL;

  I_NetSetConnectionHandler(NULL);
  I_NetSetDisconnectionHandler(NULL);
  I_NetSetDataHandler(NULL);
}

void I_NetServiceNetworkTimeout(int timeout_ms) {
  while (net_host) {
    ENetEvent net_event;
    int status = enet_host_service(net_host, &net_event, timeout_ms);

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
        if (net_event.data >= DISCONNECT_REASON_MAX) {
          D_MsgLocalInfo("Peer disconnected: Reason unknown (%u)\n",
            net_event.data
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
          I_NetIPToConstString(ENET_NET_TO_HOST_32(
            net_event.peer->address.host
          )),
          net_event.peer->address.port
        );
      break;
    }

    if (timeout_ms != 0) {
      break;
    }
  }
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
    enet_peer_send(epeer, NET_CHANNEL_RELIABLE, epacket);
  }
  else {
    enet_peer_send(epeer, NET_CHANNEL_UNRELIABLE, epacket);
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
  ENetPeer *epeer = (ENetPeer *)peer;

  return epeer->packetLossVariance / (float)ENET_PEER_PACKET_LOSS_SCALE;
}

void I_NetPeerDisconnect(base_netpeer_t *peer,
                         disconnection_reason_e reason) {
  ENetPeer *epeer = (ENetPeer *)peer;

  enet_peer_disconnect(epeer, reason);
}

net_peer_t* I_NetPeerGetPeer(base_netpeer_t *peer) {
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

base_netpeer_t* I_NetEventGetPeer(netevent_t *event) {
  ENetEvent *eevent = (ENetEvent *)event;

  return (base_netpeer_t *)eevent->peer;
}

uint32_t I_NetEventGetData(netevent_t *event) {
  ENetEvent *eevent = (ENetEvent *)event;

  return eevent->data;
}

netpacket_t* I_NetEventGetPacket(netevent_t *event) {
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

size_t I_NetIPToString(uint32_t address, char *buffer) {
  return snprintf(buffer, 16, "%u.%u.%u.%u",
    to_uchar(address >> 24),
    to_uchar(address >> 16),
    to_uchar(address >>  8),
    to_uchar(address      )
  );
}

const char* I_NetIPToConstString(uint32_t address) {
  static char buf[16];

  I_NetIPToString(address, &buf[0]);

  return &buf[0];
}

bool I_NetIPToInt(const char *address_string, uint32_t *address_int) {
  int ip = 0;
  int arg_count;
  unsigned char octets[4];

  arg_count = sscanf(address_string, "%hhu.%hhu.%hhu.%hhu",
    &octets[0],
    &octets[1],
    &octets[2],
    &octets[3]
  );

  if (arg_count != 4) {
    D_MsgLocalError("Malformed IP address %s.\n", address_string);
    return false;
  }

  for (int i = 0; i < 4; i++) {
    ip += octets[i] << (8 * (3 - i));
  }

  return ip;
}

size_t I_NetParseAddressString(const char *address, char **host,
                                                    uint16_t *port) {
  unsigned char octets[4];
  uint16_t      tmp_port;
  int           parsed_tokens;
  size_t        address_length;
  size_t        bytes_written;

  parsed_tokens = sscanf(address, "%hhu.%hhu.%hhu.%hhu:%hu",
    &octets[0],
    &octets[1],
    &octets[2],
    &octets[3],
    &tmp_port
  );

  if (parsed_tokens != 5) {
    D_MsgLocalWarn("Invalid IP address %s\n", address);
    return 0;
  }

  address_length = snprintf(NULL, 0, "%hhu.%hhu.%hhu.%hhu",
    octets[0],
    octets[1],
    octets[2],
    octets[3]
  );

  if (!*host) {
    *host = calloc(address_length + 1, sizeof(char));

    if (!*host) {
      I_Error("I_NetParseAddressString: error allocating memory for host\n");
    }
  }

  bytes_written = snprintf(*host, address_length + 1, "%hhu.%hhu.%hhu.%hhu",
    octets[0],
    octets[1],
    octets[2],
    octets[3]
  );

  if (bytes_written != address_length) {
    D_MsgLocalError("Error copying host: %s\n", strerror(errno));
    return 0;
  }

  *port = tmp_port;

  return bytes_written;
}

/* vi: set et ts=2 sw=2: */
