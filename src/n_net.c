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

#include "d_ticcmd.h"

#include "g_game.h"
#include "lprintf.h"
#include "i_system.h"
#include "m_pbuf.h"
#include "m_swap.h"

#include "n_net.h"
#include "n_state.h"
#include "n_peer.h"
#include "n_pack.h"
#include "n_proto.h"

#define USE_RANGE_CODER 0
#define MAX_DOWNLOAD 0
#define MAX_UPLOAD 0

#define MAX_ADDRESS_LENGTH 500

#define to_byte(x)  ((byte)((x) & 0xFF))
#define to_short(x) ((unsigned short)((x) & 0xFFFF))
#define string_to_byte(x) (to_byte(strtol(x, NULL, 10)))
#define string_to_short(x) (to_short(strtol(x, NULL, 10)))

/* CG: General networking */

static ENetHost    *net_host = NULL;
static const char  *previous_host = NULL;
static short        previous_port = 0;

/* CG: Externally viewable */
dboolean        netgame   = false;
dboolean        solonet   = false;
dboolean        netserver = false;
net_sync_type_e netsync   = NET_SYNC_TYPE_NONE;

size_t N_IPToString(int address, char *buffer) {
  return snprintf(buffer, 16, "%u.%u.%u.%u",
    to_byte(address >> 24),
    to_byte(address >> 16),
    to_byte(address >>  8),
    to_byte(address      )
  );
}

const char* N_IPToConstString(int address) {
  static char buf[16];

  snprintf(buf, 16, "%u.%u.%u.%u",
    to_byte(address >> 24),
    to_byte(address >> 16),
    to_byte(address >>  8),
    to_byte(address      )
  );

  return buf;
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

  if (address_length == 0)
    return 0;

  sep = strchr(address, ':');

  if (sep == NULL) {
    host_length = address_length;

    if (host_length > 0) {
      if (*host == NULL) {
        *host = strdup(address);
      }
      else {
        memset(*host, 0, (address_length + 1) * sizeof(char));
        strncpy(*host, address, address_length);
      }
    }
  }
  else {
    host_length = sep - address;

    if (host_length > 0) {
      if (*host == NULL) {
        *host = calloc(host_length + 1, sizeof(char));
        strncpy(*host, address, host_length);
      }
      else {
        memset(host, 0, (host_length + 1) * sizeof(char));
        strncpy(*host, address, host_length);
      }
    }

  }

  if (sep && strlen(++sep))
    *port = string_to_short(sep);

  return host_length;
}

void N_Init(void) {
  if (enet_initialize() != 0)
    I_Error("Error initializing ENet");

  N_InitPeers();
  N_InitStates();
  atexit(N_Shutdown);
}

void N_Disconnect(void) {
  int res = 0;
  netpeer_t *np = NULL;
  ENetEvent net_event;

  if (net_host == NULL)
    return;

  for (int i = 0; i < N_PeerGetCount(); i++) {
    np = N_PeerGet(i);

    if (np == NULL)
      continue;

    printf("N_Disconnect: Disconnecting peer %d\n", i);
    enet_peer_disconnect(np->peer, 0);
  }

  while (true) {
    res = enet_host_service(net_host, &net_event, DISCONNECT_TIMEOUT * 1000);

    if (res > 0) {
      int peernum = N_PeerGetNum(net_event.peer);

      if (peernum == -1) {
        doom_printf(
          "N_Disconnect: Received network event from unknown peer %s:%u\n",
          N_IPToConstString(doom_b32(net_event.peer->address.host)),
          net_event.peer->address.port
        );
        continue;
      }

      if (net_event.type == ENET_EVENT_TYPE_DISCONNECT)
        N_PeerRemove(N_PeerGet(peernum));

    }
    else if (res < 0) {
      doom_printf("N_Disconnect: Unknown error disconnecting\n");
      break;
    }
    else if (res == 0) {
      break;
    }
  }

  for (int i = 0; i < N_PeerGetCount(); i++) {
    np = N_PeerGet(i);

    if (np != NULL) {
      enet_peer_reset(np->peer);
      N_PeerRemove(np);
    }
  }

  memset(&net_event, 0, sizeof(ENetEvent));

  enet_host_destroy(net_host);
  net_host = NULL;
}

void N_Shutdown(void) {
  printf("N_Shutdown: shutting down\n");
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

  net_host = enet_host_create(
    &address, MAXPLAYERS, NET_CHANNEL_MAX, MAX_DOWNLOAD, MAX_UPLOAD
  );
  
  if (net_host == NULL) {
    doom_printf("N_Listen: Error creating host on %s:%u\n", host, port);
    return false;
  }

#if USE_RANGE_CODER
  if (enet_host_compress_with_range_coder(net_host) < 0) {
    doom_printf("N_Listen: Error activating range coder\n");
    return false;
  }
#endif

  return true;
}

dboolean N_Connect(const char *host, unsigned short port) {
  int peernum = -1;
  ENetPeer *server = NULL;
  ENetAddress address;

  net_host = enet_host_create(NULL, 1, NET_CHANNEL_MAX, 0, 0);

  if (net_host == NULL) {
    doom_printf("N_Connect: Error creating host");
    return false;
  }

#if USE_RANGE_CODER
  if (enet_host_compress_with_range_coder(net_host) < 0) {
    doom_printf("N_Connect: Error activating range coder\n");
    return false;
  }
#endif

  enet_address_set_host(&address, host);

  if (port != 0)
    address.port = port;
  else
    address.port = DEFAULT_PORT;

  peernum = N_PeerAdd();
  server = enet_host_connect(net_host, &address, NET_CHANNEL_MAX, 0);

  if (server == NULL) {
    doom_printf("N_Connect: Error connecting to server");
    N_Disconnect();
    return false;
  }

  N_PeerGet(peernum)->peer = server;

  previous_host = host;
  previous_port = port;

  return true;
}

dboolean N_Reconnect(void) {
  if ((previous_host != NULL) && (previous_port != 0))
    return N_Connect(previous_host, previous_port);

  doom_printf("No previous connection\n");
  return false;
}

void N_WaitForPacket(int ms) {
  ENetEvent net_event;

  enet_host_service(net_host, &net_event, ms);
}

dboolean N_ConnectToServer(const char *address) {
  char *host = NULL;
  unsigned short port;
  dboolean connected = false;

  N_ParseAddressString(address, &host, &port);

  if (port == 0)
    port = DEFAULT_PORT;

  connected = N_Connect(host, port);

  free(host);

  return connected;
}

void N_PrintAddress(FILE *fp, int peernum) {
  netpeer_t *np = N_PeerGet(peernum);

  if (np == NULL)
    I_Error("N_PrintAddress: Invalid peer %d.\n", peernum);

  fprintf(fp, "%s:%u",
    N_IPToConstString(doom_b32(np->peer->address.host)), np->peer->address.port
  );
}

void N_DisconnectPeer(int peernum) {
  netpeer_t *np = N_PeerGet(peernum);

  if (np == NULL)
    I_Error("N_DisconnectPeer: Invalid peer %d.\n", peernum);

  doom_printf("N_DisconnectPeer: Disconnecting peer %d\n", peernum);
  printf("N_DisconnectPeer: Disconnecting peer %d\n", peernum);
  enet_peer_disconnect(np->peer, 0);
  N_PeerSetDisconnected(peernum);
}

void N_DisconnectPlayer(short playernum) {
  int peernum = N_PeerGetNumForPlayer(playernum);

  if (peernum == -1)
    I_Error("N_DisconnectPlayer: Invalid player %d.\n", playernum);
  
  doom_printf("N_DisconnectPlayer: Disconnecting player %d\n", playernum);
  N_DisconnectPeer(peernum);
}

void N_ServiceNetworkTimeout(int timeout_ms) {
  int status = 0;
  int peernum = -1;
  netpeer_t *np = NULL;
  ENetEvent net_event;

  for (int i = 0; i < N_PeerGetCount(); i++) {
    netpeer_t *np = N_PeerGet(i);

    if (np == NULL)
      continue;

    if (N_PeerCheckTimeout(i)) {
      doom_printf("Peer %s:%u timed out.\n",
        N_IPToConstString(doom_b32(np->peer->address.host)),
        np->peer->address.port
      );

      enet_peer_reset(np->peer);
      N_PeerRemove(np);
      continue;
    }

    N_PeerFlushBuffers(i);
  }

  while (true) {
    status = enet_host_service(net_host, &net_event, timeout_ms);

    /*
     * [CG]: ENet says this must be cleared, and since we don't use it, we do
     *       so here up top.
     */
    if (net_event.peer != NULL && net_event.peer->data != NULL)
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
      if (SERVER) {
        peernum = N_PeerAdd();
        netpeer_t *np = N_PeerGet(peernum);

        if (np == NULL) {
          doom_printf("N_ServiceNetwork: Getting new peer failed\n");
          continue;
        }

        N_PeerSetConnected(peernum, net_event.peer);

        SV_SetupNewPeer(peernum);
      }
      else {
        np = N_PeerGet(0);

        if (np == NULL) {
          doom_printf(
            "N_ServiceNetwork: Received a 'connect' event but no connection "
            "was requested.\n"
          );
          continue;
        }

        N_PeerSetConnected(0, net_event.peer);
      }
    }
    else if (net_event.type == ENET_EVENT_TYPE_DISCONNECT) {
      if ((peernum = N_PeerGetNum(net_event.peer)) == -1) {
        doom_printf(
          "N_ServiceNetwork: Received 'disconnect' event from unknown "
          "peer %s:%u.\n",
          N_IPToConstString(doom_b32(net_event.peer->address.host)),
          net_event.peer->address.port
        );
        continue;
      }
      N_PeerRemove(N_PeerGet(peernum));
    }
    else if (net_event.type == ENET_EVENT_TYPE_RECEIVE) {
      if ((peernum = N_PeerGetNum(net_event.peer)) == -1) {
        doom_printf(
          "N_ServiceNetwork: Received 'packet' event from unknown peer %s:%u.\n",
          N_IPToConstString(doom_b32(net_event.peer->address.host)),
          net_event.peer->address.port
        );
        continue;
      }
      netpeer_t *np = N_PeerGet(peernum);

      if (np == NULL) {
        doom_printf("N_ServiceNetwork: Error getting peer that just sent data\n");
        continue;
      }

      if ((net_event.packet->data[0] & 0x80) == 0x80) {
        N_HandlePacket(
          peernum, net_event.packet->data, net_event.packet->dataLength
        );
      }
      enet_packet_destroy(net_event.packet);
    }
    else if (net_event.type != ENET_EVENT_TYPE_NONE) {
      doom_printf(
        "N_ServiceNetwork: Received unknown event from peer %s:%u.\n",
        N_IPToConstString(doom_b32(net_event.peer->address.host)),
        net_event.peer->address.port
      );
    }

    if (timeout_ms != 0)
      break;
  }
}

void N_ServiceNetwork(void) {
  N_ServiceNetworkTimeout(0);
}

uint32_t N_GetUploadBandwidth(void) {
  static uint32_t last_ms = 0;

  uint32_t ms;
  uint32_t bytes_sent;
  uint32_t time_elapsed;
  netpeer_t *np = N_PeerGet(0);

  if (np == NULL)
    return 0;

  ms = I_GetTicks();

  if (last_ms == 0) {
    last_ms = ms;
    return 0;
  }

  bytes_sent = net_host->totalSentData;

  if (ms < last_ms)
    time_elapsed = (0xFFFFFFFF - last_ms) + ms;
  else
    time_elapsed = ms - last_ms;

  if (time_elapsed == 0)
    return 0;

  last_ms = ms;
  net_host->totalSentData = 0;

  return (bytes_sent / time_elapsed);
}

uint32_t N_GetDownloadBandwidth(void) {
  static uint32_t last_ms = 0;

  uint32_t ms;
  uint32_t bytes_received;
  uint32_t time_elapsed;
  netpeer_t *np = N_PeerGet(0);

  if (np == NULL)
    return 0;

  ms = I_GetTicks();

  if (last_ms == 0) {
    last_ms = ms;
    return 0;
  }

  bytes_received = net_host->totalReceivedData;

  if (ms < last_ms)
    time_elapsed = (0xFFFFFFFF - last_ms) + ms;
  else
    time_elapsed = ms - last_ms;

  if (time_elapsed == 0)
    return 0;

  last_ms = ms;
  net_host->totalReceivedData = 0;

  return (bytes_received / time_elapsed);
}

/* vi: set et ts=2 sw=2: */

