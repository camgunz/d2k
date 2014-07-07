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

#include "doomstat.h"
#include "d_ticcmd.h"
#include "g_game.h"
#include "lprintf.h"
#include "m_pbuf.h"

#include "n_net.h"
#include "n_state.h"
#include "n_peer.h"

#define get_netchan(netchan, netcom, chan_type)                               \
  if (chan_type == NET_CHANNEL_RELIABLE)                                      \
    netchan = &netcom->reliable;                                              \
  else if (chan_type == NET_CHANNEL_UNRELIABLE)                               \
    netchan = &netcom->unreliable;                                            \
  else                                                                        \
    I_Error("%s: Invalid network channel %d.\n", __func__, chan_type)

typedef struct tocentry_s {
  unsigned int index;
  unsigned char type;
} tocentry_t;

static obuf_t net_peers;

static void init_channel(netchan_t *nc) {
  M_CBufInit(&nc->toc, sizeof(tocentry_t));
  M_PBufInit(&nc->messages);
  M_PBufInit(&nc->packet_data);
  M_PBufInit(&nc->packed_toc);
}

static void clear_channel(netchan_t *nc) {
  M_CBufClear(&nc->toc);
  M_PBufClear(&nc->messages);
  M_PBufClear(&nc->packet_data);
  M_PBufClear(&nc->packed_toc);
}

static void serialize_toc(netchan_t *chan) {
  M_PBufClear(&chan->packed_toc);

  if (!M_PBufWriteMap(&chan->packed_toc, M_CBufGetObjectCount(&chan->toc)))
    I_Error("Error writing map: %s.\n", cmp_strerror(&chan->packed_toc.cmp));

  D_Log(LOG_MEM, "serialize_toc: Buffer cursor, size, capacity: %zu/%zu/%zu\n",
    M_PBufGetCursor(&chan->packed_toc),
    M_PBufGetSize(&chan->packed_toc),
    M_PBufGetCapacity(&chan->packed_toc)
  );

  CBUF_FOR_EACH(&chan->toc, entry) {
    tocentry_t *message = (tocentry_t *)entry.obj;

    if (!M_PBufWriteUInt(&chan->packed_toc, message->index))
      I_Error("Error writing UInt: %s.\n", cmp_strerror(&chan->packed_toc.cmp));
    if (!M_PBufWriteUChar(&chan->packed_toc, message->type))
      I_Error("Error writing UChar: %s\n", cmp_strerror(&chan->packed_toc.cmp));
  }
}

typedef struct packet_buf_s {
  unsigned char *data;
  unsigned int size;
  unsigned int cursor;
} packet_buf_t;

static bool packet_read(cmp_ctx_t *ctx, void *data, size_t limit) {
  packet_buf_t *packet_data = (packet_buf_t *)ctx->buf;

  memcpy(data, packet_data->data + packet_data->cursor, limit);
  packet_data->cursor += limit;

  return true;
}

static dboolean deserialize_toc(cbuf_t *toc, unsigned char *data,
                                             unsigned int size,
                                             size_t *message_start_point) {
  cmp_ctx_t cmp;
  packet_buf_t packet_data;
  unsigned int toc_size = 0;

  cmp_init(&cmp, &packet_data, &packet_read, NULL);
  packet_data.data = data;
  packet_data.cursor = 0;
  packet_data.size = size;

  if (!cmp_read_map(&cmp, &toc_size)) {
    printf("Error reading map: %s\n", cmp_strerror(&cmp));

    printf("Packet data: ");
    for (unsigned int i = 0; i < MIN(size, 26); i++)
      printf("%02X ", data[i] & 0xFF);
    printf("\n");
    return false;
  }

  M_CBufClear(toc);
  M_CBufEnsureCapacity(toc, toc_size);

  for (int i = 0; i < toc_size; i++) {
    tocentry_t *message = (tocentry_t *)M_CBufGetFirstFreeOrNewSlot(toc);

    if (!cmp_read_uint(&cmp, &message->index)) {
      printf("Error reading message index: %s\n", cmp_strerror(&cmp));
      return false;
    }

    if (!cmp_read_uchar(&cmp, &message->type)) {
      printf("Error reading message type: %s\n", cmp_strerror(&cmp));
      return false;
    }
  }

  *message_start_point = packet_data.cursor;
  return true;
}

static dboolean channel_empty(netchan_t *nc) {
  return M_CBufGetObjectCount(&nc->toc) <= 0;
}

static void init_netcom(netcom_t *nc) {
  init_channel(&nc->incoming);
  init_channel(&nc->reliable);
  init_channel(&nc->unreliable);
}

static void free_netcom(netcom_t *nc) {
  M_CBufFree(&nc->incoming.toc);
  M_PBufFree(&nc->incoming.messages);
  M_PBufFree(&nc->incoming.packet_data);
  M_CBufFree(&nc->reliable.toc);
  M_PBufFree(&nc->reliable.messages);
  M_PBufFree(&nc->reliable.packet_data);
  M_CBufFree(&nc->unreliable.toc);
  M_PBufFree(&nc->unreliable.messages);
  M_PBufFree(&nc->unreliable.packet_data);
}

static void init_netsync(netsync_t *ns) {
  ns->initialized = false;
  ns->outdated = false;
  ns->tic = 0;
  ns->cmd = 0;
  M_BufferInit(&ns->delta.data);
}

static void free_netsync(netsync_t *ns) {
  ns->initialized = false;
  ns->outdated = false;
  ns->tic = 0;
  ns->cmd = 0;
  M_BufferFree(&ns->delta.data);
}

void N_InitPeers(void) {
  M_OBufInit(&net_peers);
}

int N_PeerAdd(void) {
  netpeer_t *np = calloc(1, sizeof(netpeer_t));

  /* CG: TODO: Add some kind of check for MAXCLIENTS */

  init_netcom(&np->netcom);
  init_netsync(&np->sync);

  np->playernum       = 0;
  np->peer            = NULL;
  np->connect_time    = time(NULL);
  np->disconnect_time = 0;

  np->peernum = M_OBufInsertAtFirstFreeSlotOrAppend(&net_peers, np);

  return np->peernum;
}

void N_PeerSetConnected(int peernum, ENetPeer *peer) {
  netpeer_t *np = N_PeerGet(peernum);

  if (np == NULL)
    I_Error("N_PeerSetConnected: Invalid peer %d.\n", peernum);

  np->peer = peer;
  np->connect_time = 0;
  enet_peer_throttle_configure(
    peer,
    ENET_PEER_PACKET_THROTTLE_INTERVAL,
    ENET_PEER_PACKET_THROTTLE_SCALE,
    ENET_PEER_PACKET_THROTTLE_SCALE
  );
}

void N_PeerSetDisconnected(int peernum) {
  netpeer_t *np = N_PeerGet(peernum);

  if (np == NULL)
    I_Error("N_PeerSetConnected: Invalid peer %d.\n", peernum);

  np->disconnect_time = time(NULL);
}

void N_PeerRemove(netpeer_t *np) {
  int peernum = N_PeerGetNum(np->peer);

  doom_printf("Removing peer %s:%u\n",
    N_IPToConstString((doom_b32(np->peer->address.host))),
    np->peer->address.port
  );

  players[np->playernum].playerstate = PST_DISCONNECTED;

  free_netcom(&np->netcom);
  free_netsync(&np->sync);

  np->peernum         = 0;
  np->playernum       = 0;
  np->peer            = NULL;
  np->connect_time    = 0;
  np->disconnect_time = 0;

  M_OBufRemove(&net_peers, peernum);
}

int N_PeerGetCount(void) {
  return M_OBufGetObjectCount(&net_peers);
}

netpeer_t* N_PeerGet(int peernum) {
  return M_OBufGet(&net_peers, peernum);
}

int N_PeerGetNum(ENetPeer *peer) {
  int index = -1;
  netpeer_t *np = NULL;

  while (M_OBufIter(&net_peers, &index, (void **)&np)) {
    if (np->peer->connectID == peer->connectID) {
      return index;
    }
  }

  return -1;
}

netpeer_t* N_PeerForPlayer(short playernum) {
  int index = -1;
  netpeer_t *np = NULL;

  while (M_OBufIter(&net_peers, &index, (void **)&np)) {
    if (np->playernum == playernum) {
      return np;
    }
  }

  return NULL;
}

int N_PeerGetNumForPlayer(short playernum) {
  int index = -1;
  netpeer_t *np = NULL;

  while (M_OBufIter(&net_peers, &index, (void **)&np)) {
    if (np->playernum == playernum) {
      return index;
    }
  }

  return -1;
}

dboolean N_PeerCheckTimeout(int peernum) {
  time_t t = time(NULL);
  netpeer_t *np = N_PeerGet(peernum);

  if (np == NULL)
    return false;

  if ((np->connect_time != 0) &&
      (difftime(t, np->connect_time) > (CONNECT_TIMEOUT * 1000))) {
    return true;
  }

  if ((np->disconnect_time != 0) &&
      (difftime(t, np->disconnect_time) > (DISCONNECT_TIMEOUT * 1000))) {
    return true;
  }

  return false;
}

void N_PeerFlushBuffers(int peernum) {
  netpeer_t *np = N_PeerGet(peernum);

  if (np == NULL)
    return;

  if (!channel_empty(&np->netcom.reliable)) {
    enet_peer_send(
      np->peer,
      NET_CHANNEL_RELIABLE,
      N_PeerGetPacket(peernum, NET_CHANNEL_RELIABLE)
    );
    clear_channel(&np->netcom.reliable);
  }

  if (!channel_empty(&np->netcom.unreliable)) {
    enet_peer_send(
      np->peer,
      NET_CHANNEL_UNRELIABLE,
      N_PeerGetPacket(peernum, NET_CHANNEL_UNRELIABLE)
    );
  }
}

pbuf_t* N_PeerBeginMessage(int peernum, net_channel_e chan_type,
                                        unsigned char type) {
  netpeer_t *np = N_PeerGet(peernum);
  netcom_t *nc = NULL;
  netchan_t *chan = NULL;
  tocentry_t *message = NULL;

  if (np == NULL)
    I_Error("N_PeerBeginMessage: Invalid peer number %d.\n", peernum);

  nc = &np->netcom;
  get_netchan(chan, nc, chan_type);

  message = M_CBufGetFirstFreeOrNewSlot(&chan->toc);
  message->index = M_PBufGetCursor(&chan->messages);
  message->type = type;

  return &chan->messages;
}

ENetPacket* N_PeerGetPacket(int peernum, net_channel_e chan_type) {
  netpeer_t *np = N_PeerGet(peernum);
  netcom_t *nc = NULL;
  netchan_t *chan = NULL;
  size_t toc_size, msg_size, packet_size;
  ENetPacket *packet = NULL;

  if (np == NULL)
    I_Error("N_PeerGetPacket: Invalid peer number %d.\n", peernum);

  nc = &np->netcom;
  get_netchan(chan, nc, chan_type);

  M_PBufClear(&chan->packed_toc);
  serialize_toc(chan);

  toc_size = M_PBufGetSize(&chan->packed_toc);
  msg_size = M_PBufGetSize(&chan->messages);
  packet_size = toc_size + msg_size;

  if (chan_type == NET_CHANNEL_RELIABLE) {
    packet = enet_packet_create(NULL, packet_size, ENET_PACKET_FLAG_RELIABLE);
  }
  else {
    packet = enet_packet_create(
      NULL, packet_size, ENET_PACKET_FLAG_UNSEQUENCED
    );
  }

  memcpy(packet->data, M_PBufGetData(&chan->packed_toc), toc_size);
  memcpy(packet->data + toc_size, M_PBufGetData(&chan->messages), msg_size);

  if (packet->data[2] != 4) {
    D_Log(LOG_NET,
      "Sending packet (packet size %zu):\n", packet->dataLength
    );
    for (int i = 0; i < MIN(26, packet->dataLength); i++)
      D_Log(LOG_NET, "%02X ", packet->data[i] & 0xFF);
    D_Log(LOG_NET, "\n");
  }

  return packet;
}

dboolean N_PeerLoadIncoming(int peernum, unsigned char *data, size_t size) {
  netpeer_t *np = N_PeerGet(peernum);
  netchan_t *incoming = NULL;
  size_t message_start_point = 0;
  buf_t buf;

  if (np == NULL)
    I_Error("N_PeerLoadIncoming: Invalid peer number %d.\n", peernum);

  incoming = &np->netcom.incoming;
  clear_channel(incoming);
  M_BufferInitWithCapacity(&buf, size);
  M_BufferWrite(&buf, data, size);

  if (!deserialize_toc(&incoming->toc, data, size, &message_start_point)) {
    for (size_t i = 0; i < size; i++)
      printf("%02X ", data[i] & 0xFF);
    printf("\n");
    M_BufferPrint(&buf);
    I_Error("N_PeerLoadIncoming: Error reading packet's TOC.\n");
    return false;
  }

  M_BufferFree(&buf);

  if (message_start_point >= size) {
    doom_printf("N_PeerLoadIncoming: Received empty packet.\n");
    return false;
  }

  M_PBufSetData(
    &incoming->messages, data + message_start_point, size - message_start_point
  );

  if (data[2] != 4) {
    D_Log(LOG_NET, "Received packet (packet size %zu):\n", size);
    for (int i = 0; i < MIN(26, size); i++)
      D_Log(LOG_NET, "%02X ", data[i] & 0xFF);
    D_Log(LOG_NET, "\n");
  }

  return true;
}

dboolean N_PeerLoadNextMessage(int peernum, unsigned char *message_type) {
  netpeer_t *np = N_PeerGet(peernum);
  tocentry_t *toc_entry = NULL;
  netchan_t *incoming = NULL;

  if (np == NULL)
    I_Error("N_PeerLoadNextMessage: Invalid peer number %d.\n", peernum);

  incoming = &np->netcom.incoming;

  if (M_CBufGetObjectCount(&incoming->toc) == 0)
    return false;

  toc_entry = M_CBufGet(&incoming->toc, 0);

  if (toc_entry->index >= M_PBufGetSize(&incoming->messages)) {
    doom_printf(
      "N_PeerLoadNextMessage: Invalid message index (%u >= %zu).\n",
      toc_entry->index, M_PBufGetSize(&incoming->messages)
    );
    M_CBufRemove(&incoming->toc, 0);
    return false;
  }

  *message_type = toc_entry->type;
  M_PBufSeek(&incoming->messages, toc_entry->index);

  M_CBufRemove(&incoming->toc, 0);
  return true;
}

void N_PeerClearReliable(int peernum) {
  netpeer_t *np = N_PeerGet(peernum);

  if (np == NULL)
    I_Error("N_PeerClearReliable: Invalid peer number %d.\n", peernum);

  clear_channel(&np->netcom.reliable);
}

void N_PeerClearUnreliable(int peernum) {
  netpeer_t *np = N_PeerGet(peernum);

  if (np == NULL)
    I_Error("N_PeerClearUnreliable: Invalid peer number %d.\n", peernum);

  clear_channel(&np->netcom.unreliable);
}

void N_PeerResetSync(int peernum) {
  netpeer_t *np = N_PeerGet(peernum);

  if (np == NULL)
    I_Error("N_PeerResetSync: Invalid peer number %d.\n", peernum);

  np->sync.initialized = false;
  np->sync.outdated = false;
}

/* vi: set et ts=2 sw=2: */

