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
#include "m_pbuf.h"
#include "n_net.h"
#include "n_buf.h"
#include "n_state.h"
#include "n_peer.h"

#define get_netchan(nc, nb, chan)                                             \
  if (chan == NET_CHANNEL_RELIABLE)                                           \
    nc = &nb->reliable;                                                       \
  else if (chan == NET_CHANNEL_UNRELIABLE)                                    \
    nc = &nb->unreliable;                                                     \
  else                                                                        \
    I_Error("%s: Invalid network channel %d.\n", __func__, chan)

typedef struct tocentry_s {
  unsigned int index;
  unsigned char type;
} tocentry_t;

static void init_channel(netchan_t *nc) {
  M_CBufInit(&nc->toc, sizeof(tocentry_t));
  M_PBufInit(&nc->messages);
  M_PBufInit(&nc->packet_data);
}

static void clear_channel(netchan_t *nc) {
  M_CBufClear(&nc->toc);
  M_PBufClear(&nc->messages);
  M_PBufClear(&nc->packet_data);
}

static void serialize_toc(pbuf_t *packet_data, cbuf_t *toc) {
  M_PBufWriteMap(packet_data, M_CBufGetObjectCount(toc));

  CBUF_FOR_EACH(toc, entry) {
    tocentry_t *message = (tocentry_t *)entry.obj;

    M_PBufWriteUInt(packet_data, message->index);
    M_PBufWriteUChar(packet_data, message->type);
  }
}

static dboolean deserialize_toc(cbuf_t *toc, pbuf_t *packet_data) {
  unsigned int toc_size = 0;

  if (!M_PBufReadMap(packet_data, &toc_size)) {
    M_PBufPrint(packet_data);
    return false;
  }

  M_CBufClear(toc);
  M_CBufEnsureCapacity(toc, toc_size);

  for (int i = 0; i < toc_size; i++) {
    tocentry_t *message = (tocentry_t *)M_CBufGetFirstFreeOrNewSlot(toc);

    if (!M_PBufReadUInt(packet_data, &message->index))
      return false;

    if (!M_PBufReadUChar(packet_data, &message->type))
      return false;
  }

  return true;
}

static dboolean channel_empty(netchan_t *nc) {
  return M_CBufGetObjectCount(&nc->toc) <= 0;
}

void N_NBufInit(netbuf_t *nb) {
  init_channel(&nb->incoming);
  init_channel(&nb->reliable);
  init_channel(&nb->unreliable);
}

void N_NBufClear(netbuf_t *nb) {
  clear_channel(&nb->incoming);
  clear_channel(&nb->reliable);
  clear_channel(&nb->unreliable);
}

void N_NBufClearIncoming(netbuf_t *nb) {
  clear_channel(&nb->incoming);
}

void N_NBufClearChannel(netbuf_t *nb, net_channel_e chan) {
  netchan_t *nc = NULL;

  get_netchan(nc, nb, chan);

  clear_channel(nc);
}

dboolean N_NBufChannelEmpty(netbuf_t *nb, net_channel_e chan) {
  netchan_t *nc = NULL;

  get_netchan(nc, nb, chan);

  return channel_empty(nc);
}

pbuf_t* N_NBufBeginMessage(netbuf_t *nb, net_channel_e chan, byte type) {
  netchan_t *nc = NULL;
  tocentry_t *message = NULL;

  get_netchan(nc, nb, chan);

  printf("Beginning %s message.\n",
    chan == NET_CHANNEL_RELIABLE ? "reliable" : "unreliable"
  );
  message = M_CBufGetFirstFreeOrNewSlot(&nc->toc);
  message->index = M_PBufGetCursor(&nc->messages);
  message->type = type;

  CBUF_FOR_EACH(&nc->toc, entry) {
    tocentry_t *te = (tocentry_t *)entry.obj;

    printf("%u: %u\n", te->index, te->type);
  }

  return &nc->messages;
}

ENetPacket* N_NBufGetPacket(netbuf_t *nb, net_channel_e chan) {
  netchan_t *nc = NULL;
  size_t toc_size, msg_size, packet_size;
  ENetPacket *packet = NULL;
  pbuf_t packed_toc;

  get_netchan(nc, nb, chan);

  M_PBufInit(&packed_toc);
  serialize_toc(&packed_toc, &nc->toc);

  toc_size = M_PBufGetSize(&packed_toc);
  msg_size = M_PBufGetSize(&nc->messages);
  packet_size = toc_size + msg_size;

  printf("Building %s packet (%zu/%zu, %p).\n",
    chan == NET_CHANNEL_RELIABLE ? "reliable" : "unreliable",
    toc_size,
    packet_size,
    &nc->toc
  );

  printf("TOC:\n");
  M_PBufPrint(&packed_toc);

  if (chan == NET_CHANNEL_RELIABLE) {
    packet = enet_packet_create(NULL, packet_size, ENET_PACKET_FLAG_RELIABLE);

    // memcpy(packet->data, M_PBufGetData(&packed_toc), toc_size);
    memset(packet->data, 5, toc_size);
    memcpy(packet->data + toc_size, M_PBufGetData(&nc->messages), msg_size);
  }
  else if (chan == NET_CHANNEL_UNRELIABLE) {
    buf_t *buf = M_PBufGetBuffer(&nc->packet_data);

    M_BufferClear(buf);
    M_BufferEnsureTotalCapacity(buf, packet_size);
    M_BufferWrite(buf, M_PBufGetData(&packed_toc), toc_size);
    M_BufferWrite(buf, M_PBufGetData(&nc->messages), msg_size);

    packet = enet_packet_create(
      M_BufferGetData(buf),
      M_BufferGetSize(buf),
      ENET_PACKET_FLAG_UNSEQUENCED | ENET_PACKET_FLAG_NO_ALLOCATE
    );
  }

  printf("Packet data:\n");
  for (int i = 0; i < MIN(64, packet->dataLength); i++)
    printf("%X ", packet->data[i] & 0xFF);
  printf("\n");

  printf("Sending packet (packet size %zu):\n", packet->dataLength);

  return packet;
}

dboolean N_NBufLoadIncoming(netbuf_t *nb, unsigned char *data, size_t size) {
  netchan_t *incoming = &nb->incoming;
  size_t toc_size = 0;
  buf_t *buf = M_PBufGetBuffer(&incoming->packet_data);

  M_PBufSetData(&incoming->packet_data, data, size);

  if (!deserialize_toc(&incoming->toc, &incoming->packet_data)) {
    doom_printf("N_NBufLoadIncoming: Error reading packet's TOC.\n");
    return false;
  }

  toc_size = M_PBufGetCursor(&incoming->packet_data);

  M_PBufSetData(
    &incoming->messages,
    M_BufferGetData(buf) + toc_size,
    M_PBufGetSize(&incoming->packet_data) - toc_size
  );

  return true;
}

dboolean N_NBufLoadNextMessage(netbuf_t *nb, unsigned char *message_type) {
  netchan_t *incoming = &nb->incoming;
  buf_t *buf = M_PBufGetBuffer(&incoming->packet_data);
  int64_t message_size = 0;
  unsigned int next_index = 0;
  tocentry_t *message = NULL;
  size_t message_count = M_CBufGetObjectCount(&incoming->toc);

  if (message_count == 0)
    return false;

  message = M_CBufGet(&incoming->toc, 0);

  M_CBufRemove(&incoming->toc, 0);

  if (message_count > 1)
    next_index = ((tocentry_t *)M_CBufGet(&incoming->toc, 0))->index;
  else
    next_index = M_PBufGetSize(&incoming->packet_data);

  message_size = next_index - message->index;

  if (message_size <= 0)
    return false;

  if (message->index >= M_BufferGetSize(buf)) {
    doom_printf("N_NBufLoadNextMessage: Invalid message index (%u >= %zu).\n",
      message->index,
      M_BufferGetSize(buf)
    );
    return false;
  }

  M_PBufSetData(
    &incoming->messages, M_BufferGetData(buf) + message->index, message_size
  );

  return true;
}

void N_NBufFree(netbuf_t *nb) {
  M_CBufFree(&nb->incoming.toc);
  M_PBufFree(&nb->incoming.messages);
  M_PBufFree(&nb->incoming.packet_data);
  M_CBufFree(&nb->reliable.toc);
  M_PBufFree(&nb->reliable.messages);
  M_PBufFree(&nb->reliable.packet_data);
  M_CBufFree(&nb->unreliable.toc);
  M_PBufFree(&nb->unreliable.messages);
  M_PBufFree(&nb->unreliable.packet_data);
}

/* vi: set et ts=2 sw=2: */

