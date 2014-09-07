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
#include "d_ticcmd.h"
#include "g_game.h"
#include "lprintf.h"
#include "p_user.h"

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

typedef struct packet_buf_s {
  unsigned char *data;
  unsigned int size;
  unsigned int cursor;
} packet_buf_t;

static GHashTable *net_peers = NULL;

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
      P_Printf(consoleplayer,
        "Error reading message index: %s\n", cmp_strerror(&cmp)
      );
      return false;
    }

    if (!cmp_read_uchar(&cmp, &message->type)) {
      P_Printf(consoleplayer, "Error reading message type: %s\n",
        cmp_strerror(&cmp)
      );
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

static void free_peer(netpeer_t *np) {
  P_Printf(consoleplayer, "Removing peer %u %s:%u\n",
    np->peernum,
    N_IPToConstString((doom_b32(np->peer->address.host))),
    np->peer->address.port
  );

  players[np->playernum].playerstate = PST_DISCONNECTED;
  free_netcom(&np->netcom);
  free_netsync(&np->sync);
  free(np);
}

void N_InitPeers(void) {
  net_peers = g_hash_table_new(NULL, NULL);
}

unsigned int N_PeerAdd(void) {
  netpeer_t *np = calloc(1, sizeof(netpeer_t));

  /* CG: TODO: Add some kind of check for MAXCLIENTS */

  init_netcom(&np->netcom);
  init_netsync(&np->sync);

  np->peernum = 1;
  while (g_hash_table_contains(net_peers, GUINT_TO_POINTER(np->peernum)))
    np->peernum++;

  np->playernum       = 0;
  np->peer            = NULL;
  np->connect_time    = time(NULL);
  np->disconnect_time = 0;

  g_hash_table_insert(net_peers, GUINT_TO_POINTER(np->peernum), np);

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
  enet_peer_disconnect(np->peer, 0);
}

void N_PeerRemove(netpeer_t *np) {
  g_hash_table_remove(net_peers, GUINT_TO_POINTER(np->peernum));
  free_peer(np);
}

void N_PeerIterRemove(netpeer_iter_t *it, netpeer_t *np) {
  g_hash_table_iter_remove(it);
  free_peer(np);
}

unsigned int N_PeerGetCount(void) {
  return g_hash_table_size(net_peers);
}

netpeer_t* N_PeerGet(int peernum) {
  return g_hash_table_lookup(net_peers, GUINT_TO_POINTER(peernum));
}

netpeer_t* CL_GetServerPeer(void) {
  return N_PeerGet(1);
}

unsigned int N_PeerForPeer(ENetPeer *peer) {
  GHashTableIter it;
  gpointer key, value;

  g_hash_table_iter_init(&it, net_peers);

  while (g_hash_table_iter_next(&it, &key, &value)) {
    netpeer_t *np = (netpeer_t *)value;

    if (np->peer->connectID == peer->connectID)
      return np->peernum;
  }

  return 0;
}

netpeer_t* N_PeerForPlayer(short playernum) {
  GHashTableIter it;
  gpointer key, value;

  g_hash_table_iter_init(&it, net_peers);

  while (g_hash_table_iter_next(&it, &key, &value)) {
    netpeer_t *np = (netpeer_t *)value;

    if (np->playernum == playernum)
      return np;
  }

  return NULL;
}

unsigned int N_PeerGetNumForPlayer(short playernum) {
  GHashTableIter it;
  gpointer key, value;

  g_hash_table_iter_init(&it, net_peers);

  while (g_hash_table_iter_next(&it, &key, &value)) {
    netpeer_t *np = (netpeer_t *)value;

    if (np->playernum == playernum)
      return np->peernum;
  }

  return 0;
}

bool N_PeerIter(netpeer_iter_t **it, netpeer_t **np) {
  gpointer key, value;

  if (*it == NULL) {
    *it = malloc(sizeof(GHashTableIter));
    g_hash_table_iter_init(*it, net_peers);
  }

  if (g_hash_table_iter_next(*it, &key, &value)) {
    *np = (netpeer_t *)value;
    return true;
  }

  free(*it);
  return false;
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
    P_Echo(consoleplayer, "N_PeerLoadIncoming: Received empty packet.");
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
    P_Printf(consoleplayer,
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

