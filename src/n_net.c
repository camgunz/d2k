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

#include "lprintf.h"
#include "n_net.h"
#include "n_pack.h"
#include "n_peer.h"
#include "n_proto.h"

#define MAX_ADDRESS_LENGTH 500
#define CONNECT_TIMEOUT 5
#define DISCONNECT_TIMEOUT 3

#define to_byte(x)  ((byte)((x) & 0xFF))
#define to_short(x) ((unsigned short)((x) & 0xFFFF))
#define string_to_byte(x) (to_byte(strtol(x, NULL, 10)))
#define string_to_short(x) (to_short(strtol(x, NULL, 10)))

static ENetAddress  net_address;
static ENetEvent    net_event;
static ENetHost    *net_host = NULL;
static const char  *previous_address = NULL;
static short        previous_port = 0;

static void check_peer_timeouts(void) {
  for (int i = 0; i < N_GetPeerCount(); i++) {
    if (N_CheckPeerTimeout(i)) {
      netpeer_t *np = N_GetPeer(i);

      doom_printf("Peer %s:%u timed out.\n",
        N_IPToString(np->peer->address.host), np->peer->address.port
      );

      enet_peer_reset(np->peer);
      N_RemovePeer(np);
    }
  }
}

size_t N_IPToString(int address, char *buffer) {
  return snprintf(buffer, 16, "%u.%u.%u.%u",
    to_byte(address >> 24),
    to_byte(address >> 16),
    to_byte(address >>  8),
    to_byte(address      )
  );
}

int N_IPToInt(const char *address) {
  int i;
  int ip = 0;
  char addr[16];
  char *p = NULL;
  char *octets[4];
  size_t address_length = strlen(address);

  if (address_length > 16 || address_length < 7) {
    doom_printf("Malformed IP address %s.", address);
    return 0;
  }

  strncpy(addr, address, 16);

  octets[0] = addr;
  for (i = 1; i < 4; i++) {
    if ((p = strchr(octets[i - 1], '.')) == NULL) {
      doom_printf("Malformed IP address %s.", address);
      return 0;
    }

    *p = '\0';
    octets[i] = p + 1;
  }

  for (i = 0; i < 4; i++)
    ip += string_to_byte(octets[i]) << (8 * (3 - i));

  return ip;
}

size_t N_GetHostFromAddressString(const char *address, char **host) {
  char *sep = NULL;
  size_t host_length = 0;
  size_t address_length = strlen(address);
  
  if (address_length > MAX_ADDRESS_LENGTH)
    return 0;

  sep = strchr(address, ':');

  if (sep == NULL) {
    host_length = address_length;

    if (*host == NULL)
      *host = strdup(address);
    else
      strncpy(*host, address, address_length + 1);
  }
  else {
    host_length = sep - address;

    if (*host == NULL)
      *host = calloc(host_length + 1, sizeof(char));
    else
      (*host)[host_length] = '\0';

    strncpy(*host, address, host_length);
  }

  return host_length;
}

dboolean N_GetPortFromAddressString(const char *address, uint16_t *port) {
  char *p = NULL;

  *port = 0;

  if (strlen(address) > MAX_ADDRESS_LENGTH)
    return false;

  if ((p = strchr(address, ':')) && strlen(p++)) {
    *port = string_to_short(p);
    return true;
  }

  return false;
}

size_t N_ParseAddressString(const char *address, char **host, uint16_t *port) {
  char *sep = NULL;
  size_t host_length = 0;
  size_t address_length = strlen(address);
  
  if (address_length > MAX_ADDRESS_LENGTH)
    return 0;

  sep = strchr(address, ':');

  if (sep == NULL) {
    host_length = address_length;

    if (*host == NULL)
      *host = strdup(address);
    else
      strncpy(*host, address, address_length + 1);
  }
  else {
    host_length = sep - address;

    if (*host == NULL)
      *host = calloc(host_length + 1, sizeof(char));
    else
      (*host)[host_length] = '\0';

    strncpy(*host, address, host_length);
  }

  if (sep && strlen(sep++))
    *port = string_to_short(sep);
  else
    *port = 0;

  return host_length;
}

void N_Init(void) {
  if (enet_initialize() != 0)
    I_Error("Error initializing ENet");

  N_InitPacker();
  atexit(N_ShutdownNetwork);
}

void N_Disconnect(void) {
  int res = 0;
  netpeer_t *np = NULL;

  if (net_host == NULL)
    return;

  for (int i = 0; i < N_GetPeerCount(); i++) {
    np = N_GetPeer(i);

    if (np == NULL)
      continue;

    enet_peer_disconnect(np->peer, 0);
  }

  while (true) {
    res = enet_host_service(net_host, &net_event, DISCONNECT_TIMEOUT * 1000);

    if (res > 0) {
      int peernum = N_GetPeernum(net_event.peer);

      if (peernum == -1) {
        doom_printf(
          "N_Disconnect: Received network event from unknown peer %s:%u\n",
          N_IPToString(net_event.peer->address.host),
          net_event.peer->address.port
        );
        continue;
      }

      if (net_event.type == ENET_EVENT_TYPE_DISCONNECT)
        N_RemovePeer(N_GetPeer(peernum));

    }
    else if (res < 0) {
      doom_printf("N_Disconnect: Unknown error disconnecting\n");
      break;
    }
    else if (res == 0) {
      break;
    }
  }

  for (int i = 0; i < N_GetPeerCount(); i++) {
    np = N_GetPeer(i);

    if (np != NULL) {
      enet_peer_reset(np->peer);
      N_RemovePeer(np);
    }
  }

  memset(&net_event, 0, sizeof(ENetEvent));

  enet_host_destroy(net_host);
  net_host = NULL;
}

void N_Shutdown(void) {
  N_Disconnect();

  enet_deinitialize();
}

dboolean N_Listen(const char *host, unsigned short port) {
  ENetAddress address;

  if (host != NULL)
    enet_address_set_host(&address, host);
  else
    address.host = ENET_HOST_ANY;

  if (port != 0)
    address.port = port;
  else
    address.port = DEFAULT_PORT;

  net_host = enet_host_create(&address, MAX_CLIENTS, MAX_CHANNELS, 0, 0);
  
  if (net_host == NULL) {
    doom_printf("N_Listen: Error creating host");
    return false;
  }
  
  return true;
}

dboolean N_Connect(const char *host, unsigned short port) {
  int peernum = -1;
  ENetAddress address;

  net_host = enet_net_host_create(NULL, 1, MAX_CHANNELS, 0, 0);

  if (net_host == NULL) {
    doom_printf("N_Connect: Error creating host");
    return false;
  }

  enet_address_set_host(&address, host);

  if (port != 0)
    address.port = port;
  else
    address.port = DEFAULT_PORT;

  peernum = N_AddPeer();
  net_peer = enet_host_connect(net_host, &address, MAX_CHANNELS, 0);

  if (net_peer == NULL) {
    doom_printf("N_Connect: Error connecting to server");
    N_ResetNetworking();
    return false;
  }

  previous_address = address;
  previous_port = port;

  return true;
}

dboolean N_Reconnect(void) {
  if ((previous_address != NULL) && (previous_port != 0))
    return N_Connect(previous_address, previous_port);

  doom_printf("No previous connection\n");
  return false;
}

void N_WaitForPacket(int ms) {
  enet_host_service(net_host, &net_event, ms);
}

dboolean N_ConnectToServer(const char *address) {
  char *host = NULL;
  unsigned short port;
  dboolean connected = false;

  N_ParseAddressString(serv, &host, &port);

  if (port == 0)
    port = DEFAULT_PORT;

  connected = N_Connect(host, port);

  free(host);

  return connected;
}

void N_PrintAddress(FILE *fp, int peernum) {
  netpeer_t *np = N_GetPeer(peernum);

  if (np == NULL)
    I_Error("N_PrintAddress: Invalid peer %d.\n", peernum);

  fprintf(
    fp, "%s:%u", N_IPToString(np->peer->address.host), np->peer->address.port
  );
}

void N_DisconnectPlayer(short playernum) {
  int peernum = N_GetPeerNumForPlayer(playernum);
  netpeer_t *np = NULL;

  if (peernum == -1)
    I_Error("N_DisconnectPlayer: Invalid peer %d.\n", peernum);
  
  np = N_GetPeer(peernum);

  doom_printf("N_DisconnectPlayer: Disconnecting player %d\n", playernum);
  enet_peer_disconnect(np->peer, 0);
  N_SetPeerDisconnected(peernum);
}

void N_ServiceNetwork(void) {
  int status = 0;
  int peernum = -1;
  netpeer_t *np = NULL;
  ENetPacket *packet = NULL;

  check_peer_timeouts();

  for (int i = 0; i < N_GetPeerCount(); i++) {
    netpeer_t *np = N_GetPeer(i);

    if (np == NULL)
      continue;

    if (np->rbuf->size != 0) {
      ENetPacket *reliable_packet = enet_packet_create(
        np->rbuf->data,
        np->rbuf->size,
        ENET_PACKET_FLAG_RELIABLE | ENET_PACKET_FLAG_NO_ALLOCATE
      );
      enet_peer_send(np->peer, NET_CHANNEL_RELIABLE, reliable_packet);
      enet_packet_destroy(reliable_packet);
      msgpack_sbuffer_clear(np->rbuf);
    }

    if (np->ubuf->size != 0) {
      ENetPacket *unreliable_packet = enet_packet_create(
        np->ubuf->data,
        np->ubuf->size,
        ENET_PACKET_FLAG_NO_ALLOCATE
      );
      enet_peer_send(np->peer, NET_CHANNEL_UNRELIABLE, unreliable_packet);
      enet_packet_destroy(unreliable_packet);
      msgpack_sbuffer_clear(np->ubuf);
    }
  }

  while (true) {
    status = enet_host_service(net_host, &net_event, 0);

    /*
     * CG: ENet says this must be cleared, and since we don't use it, we do so
     *     here up top.
     */
    if (net_event.peer->data != NULL)
      net_event.peer->data = NULL;

    if (status == 0)
      break;

    if (status < 0) {
      doom_printf(
        "N_ServiceNetwork: Unknown error occurred while servicing host\n"
      );
      break;
    }

    if (net_event.type == ENET_EVENT_TYPE_CONNECT) {
      if (server) {
        peernum = N_AddPeer();
        N_SetPeerConnected(peernum, net_event.peer);
      }
      else {
        np = N_GetPeer(0);

        if (np == NULL) {
          doom_printf(
            "N_ServiceNetwork: Received a connect event but no connection was "
            "requested.\n"
          );
          continue;
        }

        N_SetPeerConnected(0, net_event.peer);
      }
    }
    else if (net_event.type == ENET_EVENT_TYPE_DISCONNECT) {
      if ((peernum = get_peer_num(net_event.peer)) == -1) {
        doom_printf(
          "N_ServiceNetwork: Received disconnect event from unknown "
          "peer %s:%u.\n",
          N_IPToString(net_event.peer->address.host),
          net_event.peer->address.port
        );
        continue;
      }
      N_RemovePeer(N_GetPeer(peernum));
    }
    else if (net_event.type == ENET_EVENT_TYPE_RECEIVE) {
      if ((peernum = get_peer_num(net_event.peer)) == -1) {
        doom_printf(
          "N_ServiceNetwork: Received packet event from unknown peer %s:%u.\n",
          N_IPToString(net_event.peer->address.host),
          net_event.peer->address.port
        );
        continue;
      }
      N_HandlePacket(
        peernum, net_event.packet->data, net_event.packet->dataLength
      );
      enet_packet_destroy(net_event.packet);
    }
    else if (net_event.type != ENET_EVENT_TYPE_NONE) {
      doom_printf(
        "N_ServiceNetwork: Received \"NONE\" event from peer %s:%u.\n",
        N_IPToString(net_event.peer->address.host),
        net_event.peer->address.port
      );
    }
  }
}

/* vi: set cindent et ts=2 sw=2: */

