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
#include "d_event.h"

#include "c_main.h"
#include "d_player.h"
#include "d_ticcmd.h"
#include "g_game.h"
#include "i_main.h"
#include "i_system.h"
#include "m_swap.h"
#include "n_net.h"
#include "n_main.h"
#include "n_state.h"
#include "n_peer.h"
#include "n_pack.h"
#include "n_proto.h"
#include "cl_main.h"
#include "p_user.h"

#define USE_RANGE_CODER 0
#define MAX_DOWNLOAD 0
#define MAX_UPLOAD 0

#define MAX_ADDRESS_LENGTH 500

#define to_uchar(x)  ((byte)((x) & 0xFF))
#define to_ushort(x) ((unsigned short)((x) & 0xFFFF))
#define string_to_uchar(x) (to_uchar(strtol(x, NULL, 10)))
#define string_to_ushort(x) (to_ushort(strtol(x, NULL, 10)))

static ENetHost   *net_host = NULL;
static const char *previous_host = NULL;
static uint16_t    previous_port = 0;

bool            netgame   = false;
bool            solonet   = false;
bool            netserver = false;
net_sync_type_e netsync   = NET_SYNC_TYPE_NONE;

size_t N_IPToString(uint32_t address, char *buffer) {
  return snprintf(buffer, 16, "%u.%u.%u.%u",
    to_uchar(address >> 24),
    to_uchar(address >> 16),
    to_uchar(address >>  8),
    to_uchar(address      )
  );
}

const char* N_IPToConstString(uint32_t address) {
  static char buf[16];

  N_IPToString(address, &buf[0]);

  return &buf[0];
}

bool N_IPToInt(const char *address_string, uint32_t *address_int) {
  int ip = 0;
  int arg_count;
  unsigned char *octets[4];

  arg_count = sscanf(address_string, "%hhu.%hhu.%hhu.%hhu",
    &octets[0],
    &octets[1],
    &octets[2],
    &octets[3]
  );

  if (arg_count != 4) {
    D_Msg(MSG_ERROR, "Malformed IP address %s.\n", address_string);
    return false;
  }

  for (int i = 0; i < 4; i++)
    ip += string_to_uchar(octets[i]) << (8 * (3 - i));

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

bool N_GetPortFromAddressString(const char *address, uint16_t *port) {
  char *p = NULL;

  *port = 0;

  if (strlen(address) > MAX_ADDRESS_LENGTH)
    return false;

  if ((p = strchr(address, ':')) && strlen(p++)) {
    *port = string_to_ushort(p);
    return true;
  }

  return false;
}

size_t N_ParseAddressString(const char *address, char **host, uint16_t *port) {
  char *sep = NULL;
  size_t host_length = 0;
  size_t address_length = strlen(address);
  bool should_free = false;
  unsigned char a;
  unsigned char b;
  unsigned char c;
  unsigned char d;
  
  if (address_length > MAX_ADDRESS_LENGTH)
    return 0;

  if (address_length == 0)
    return 0;

  sep = strchr(address, ':');

  if (!sep) {
    if (!*host) {
      *host = strdup(address);
      should_free = true;
    }
    else {
      memset(*host, 0, (address_length + 1) * sizeof(char));
      strncpy(*host, address, address_length + 1);
    }
  }
  else {
    host_length = sep - address;

    if (host_length > 0) {
      if (!*host) {
        *host = calloc(host_length + 1, sizeof(char));
        should_free = true;
      }
      else {
        memset(host, 0, (host_length + 1) * sizeof(char));
      }
      strncpy(*host, address, host_length);
    }
  }

  if (sscanf(address, "%hhu.%hhu.%hhu.%hhu", &a, &b, &c, &d) != 4) {
    if (should_free) {
      free(*host);
      *host = NULL;
    }

    return 0;
  }

  if (sep && strlen(++sep))
    *port = string_to_ushort(sep);

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
  ENetEvent net_event;

  if (net_host == NULL)
    return;

  NETPEER_FOR_EACH(iter) {
    D_Msg(MSG_INFO, "Disconnecting peer %d\n", iter.np->peernum);
    enet_peer_disconnect(iter.np->peer, 0);
  }

  if (CLIENT)
    CL_SetReceivedSetup(false);

  while (true) {
    res = enet_host_service(
      net_host, &net_event, NET_DISCONNECT_TIMEOUT * 1000
    );

    if (res > 0) {
      int peernum = N_PeerForPeer(net_event.peer);

      if (!peernum) {
        D_Msg(MSG_WARN,
          "N_Disconnect: Received network event from unknown peer %s:%u\n",
          N_IPToConstString(ENET_NET_TO_HOST_32(net_event.peer->address.host)),
          net_event.peer->address.port
        );
        continue;
      }

      if (net_event.type == ENET_EVENT_TYPE_DISCONNECT)
        N_PeerRemove(N_PeerGet(peernum));

    }
    else if (res < 0) {
      D_Msg(MSG_WARN, "N_Disconnect: Unknown error disconnecting\n");
      break;
    }
    else if (res == 0) {
      break;
    }
  }

  NETPEER_FOR_EACH(iter) {
    enet_peer_reset(iter.np->peer);
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
  int peernum = -1;
  ENetPeer *server = NULL;
  ENetAddress address;

  net_host = enet_host_create(NULL, 1, NET_CHANNEL_MAX, 0, 0);

  if (net_host == NULL) {
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

  if (port != 0)
    address.port = port;
  else
    address.port = DEFAULT_PORT;

  peernum = N_PeerAdd();

  server = enet_host_connect(net_host, &address, NET_CHANNEL_MAX, 0);

  if (server == NULL) {
    D_Msg(MSG_ERROR, "N_Connect: Error connecting to server\n");
    N_Disconnect();
    return false;
  }

  N_PeerGet(peernum)->peer = server;

  previous_host = host;
  previous_port = port;

  return true;
}

bool N_Reconnect(void) {
  if ((previous_host != NULL) && (previous_port != 0))
    return N_Connect(previous_host, previous_port);

  D_Msg(MSG_INFO, "No previous connection\n");
  return false;
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
    N_IPToConstString(ENET_NET_TO_HOST_32(np->peer->address.host)),
    np->peer->address.port
  );
}

void N_DisconnectPeer(int peernum) {
  netpeer_t *np = N_PeerGet(peernum);

  if (np == NULL)
    I_Error("N_DisconnectPeer: Invalid peer %d.\n", peernum);

  D_Msg(MSG_INFO, "N_DisconnectPeer: Disconnecting peer %d\n", peernum);
  N_PeerSetDisconnected(peernum);
}

void N_DisconnectPlayer(short playernum) {
  int peernum = N_PeerGetNumForPlayer(playernum);

  if (peernum == -1)
    I_Error("N_DisconnectPlayer: Invalid player %d.\n", playernum);
  
  D_Msg(MSG_INFO, "N_DisconnectPlayer: Disconnecting player %d\n", playernum);
  N_DisconnectPeer(peernum);
}

void N_ServiceNetworkTimeout(int timeout_ms) {
  netpeer_t *np;
  int status = 0;
  int peernum = -1;
  ENetEvent net_event;

  // printf("N_ServiceNetworkTimeout: %d\n", timeout_ms);

  if (net_host == NULL)
    return;

  NETPEER_FOR_EACH(iter) {
    np = iter.np;

    if (N_PeerCheckTimeout(np->peernum)) {
      D_Msg(MSG_INFO, "Peer %s:%u timed out.\n",
        N_IPToConstString(ENET_NET_TO_HOST_32(np->peer->address.host)),
        np->peer->address.port
      );

      enet_peer_reset(np->peer);
      N_PeerIterRemove(iter.it, np);
      continue;
    }

    N_PeerFlushBuffers(np->peernum);
  }

  while (net_host != NULL) {
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
      D_Msg(MSG_INFO,
        "N_ServiceNetwork: Unknown error occurred while servicing host\n"
      );
      break;
    }

    if (net_event.type == ENET_EVENT_TYPE_CONNECT) {
      if (SERVER) {
        peernum = N_PeerAdd();
        np = N_PeerGet(peernum);

        if (np == NULL) {
          D_Msg(MSG_ERROR, "N_ServiceNetwork: Getting new peer failed\n");
          continue;
        }

        /*
        enet_peer_timeout(
          net_event.peer,
          0,
          NET_PEER_MINIMUM_TIMEOUT * 1000,
          NET_PEER_MAXIMUM_TIMEOUT * 1000
        );
        */

        N_PeerSetConnected(np->peernum, net_event.peer);

        SV_SetupNewPeer(peernum);
      }
      else {
        np = CL_GetServerPeer();

        if (np == NULL) {
          D_Msg(MSG_WARN,
            "N_ServiceNetwork: Received a 'connect' event but no connection "
            "was requested.\n"
          );
          continue;
        }

        N_PeerSetConnected(np->peernum, net_event.peer);
      }
    }
    else if (net_event.type == ENET_EVENT_TYPE_DISCONNECT) {
      if (!(peernum = N_PeerForPeer(net_event.peer))) {
        D_Msg(MSG_WARN,
          "N_ServiceNetwork: Received 'disconnect' event from unknown "
          "peer %s:%u.\n",
          N_IPToConstString(ENET_NET_TO_HOST_32(net_event.peer->address.host)),
          net_event.peer->address.port
        );
        continue;
      }
      if (CLIENT) {
        N_Disconnect();
        I_SafeExit(0);
      }

      N_PeerRemove(N_PeerGet(peernum));
    }
    else if (net_event.type == ENET_EVENT_TYPE_RECEIVE) {
      if (!(peernum = N_PeerForPeer(net_event.peer))) {
        D_Msg(MSG_WARN,
          "N_ServiceNetwork: Received 'packet' event from unknown peer %s:%u.\n",
          N_IPToConstString(ENET_NET_TO_HOST_32(net_event.peer->address.host)),
          net_event.peer->address.port
        );
        continue;
      }
      netpeer_t *np = N_PeerGet(peernum);

      if (np == NULL) {
        D_Msg(MSG_ERROR,
          "N_ServiceNetwork: Error getting peer that just sent data\n"
        );
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
      D_Msg(MSG_WARN,
        "N_ServiceNetwork: Received unknown event from peer %s:%u.\n",
        N_IPToConstString(ENET_NET_TO_HOST_32(net_event.peer->address.host)),
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

  if (net_host == NULL)
    return 0;

  if (CL_GetServerPeer() == NULL)
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

  if (net_host == NULL)
    return 0;

  if (CL_GetServerPeer() == NULL)
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

