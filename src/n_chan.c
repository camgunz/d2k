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
#include "i_main.h"
#include "g_game.h"
#include "n_main.h"
#include "p_user.h"
#include "n_chan.h"

typedef struct tocentry_s {
  unsigned int  index;
  unsigned char type;
} tocentry_t;

typedef struct packet_buf_s {
  unsigned char *data;
  unsigned int   size;
  unsigned int   cursor;
} packet_buf_t;

static void serialize_toc(netchan_t *chan) {
  M_PBufClear(&chan->packed_toc);

  if (!M_PBufWriteMap(&chan->packed_toc, chan->toc->len))
    I_Error("Error writing map: %s.\n", cmp_strerror(&chan->packed_toc.cmp));

  D_Msg(MSG_MEM, "serialize_toc: Buffer cursor, size, capacity: %zu/%zu/%zu\n",
    M_PBufGetCursor(&chan->packed_toc),
    M_PBufGetSize(&chan->packed_toc),
    M_PBufGetCapacity(&chan->packed_toc)
  );

  for (unsigned int i = 0; i < chan->toc->len; i++) {
    tocentry_t *message = &g_array_index(chan->toc, tocentry_t, i);

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

static bool deserialize_toc(GArray *toc, unsigned char *data,
                                         unsigned int size,
                                         size_t *message_start_point) {
  cmp_ctx_t cmp;
  packet_buf_t packet_data;
  unsigned int toc_size = 0;

  cmp_init(&cmp, &packet_data, &packet_read, NULL, NULL);
  packet_data.data = data;
  packet_data.cursor = 0;
  packet_data.size = size;

  if (!cmp_read_map(&cmp, &toc_size)) {
    D_Msg(MSG_DEBUG, "Error reading map: %s\n", cmp_strerror(&cmp));

    D_Msg(MSG_DEBUG, "Packet data: ");
    for (unsigned int i = 0; i < MIN(size, 26); i++) {
      D_Msg(MSG_DEBUG, "%02X ", data[i] & 0xFF);
    }
    D_Msg(MSG_DEBUG, "\n");

    return false;
  }

  if (toc->len > 0) {
    g_array_remove_range(toc, 0, toc->len);
  }

  g_array_set_size(toc, toc_size);

  for (unsigned int i = 0; i < toc_size; i++) {
    tocentry_t *toc_entry = &g_array_index(toc, tocentry_t, i);

    if (!cmp_read_uint(&cmp, &toc_entry->index)) {
      P_Printf(consoleplayer,
        "Error reading message index: %s\n", cmp_strerror(&cmp)
      );
      return false;
    }

    if (!cmp_read_uchar(&cmp, &toc_entry->type)) {
      P_Printf(consoleplayer, "Error reading message type: %s\n",
        cmp_strerror(&cmp)
      );
      return false;
    }
  }

  *message_start_point = packet_data.cursor;
  return true;
}

void N_ChannelInit(netchan_t *nc, bool reliable, bool throttled) {
  nc->reliable = reliable;
  nc->throttled = throttled;
  nc->toc = g_array_new(false, true, sizeof(tocentry_t));
  M_PBufInit(&nc->messages);
  M_PBufInit(&nc->packet_data);
  M_PBufInit(&nc->packed_toc);
  nc->last_flush_tic = 0;
}

void N_ChannelClear(netchan_t *nc) {
  if (nc->toc->len > 0) {
    g_array_remove_range(nc->toc, 0, nc->toc->len);
  }

  M_PBufClear(&nc->messages);
  M_PBufClear(&nc->packet_data);
  M_PBufClear(&nc->packed_toc);
}

void* N_ChannelGetPacket(netchan_t *nc) {
  size_t toc_size;
  size_t msg_size;
  size_t packet_size;
  ENetPacket *packet = NULL;

  M_PBufClear(&nc->packed_toc);
  serialize_toc(nc);

  toc_size = M_PBufGetSize(&nc->packed_toc);
  msg_size = M_PBufGetSize(&nc->messages);
  packet_size = toc_size + msg_size;

  if (nc->reliable) {
    packet = enet_packet_create(NULL, packet_size, ENET_PACKET_FLAG_RELIABLE);
  }
  else {
    packet = enet_packet_create(
      NULL, packet_size, ENET_PACKET_FLAG_UNSEQUENCED
    );
  }

  if (!packet) {
    I_Error("Error allocating packet\n");
  }

  memcpy(packet->data, M_PBufGetData(&nc->packed_toc), toc_size);
  memcpy(packet->data + toc_size, M_PBufGetData(&nc->messages), msg_size);

  if (packet->data[2] != 4) {
    D_Msg(MSG_NET,
      "Sending packet (packet size %zu):\n", packet->dataLength
    );

    for (int i = 0; i < MIN(26, packet->dataLength); i++) {
      D_Msg(MSG_NET, "%02X ", packet->data[i] & 0xFF);
    }

    D_Msg(MSG_NET, "\n");
  }

  if (nc->throttled) {
    nc->last_flush_tic = I_GetTime();
  }

  return packet;
}

pbuf_t* N_ChannelBeginMessage(netchan_t *nc, unsigned char type) {
  tocentry_t *toc_entry = NULL;

  for (unsigned int i = 0; i < nc->toc->len; i++) {
    toc_entry = &g_array_index(nc->toc, tocentry_t, i);

    if (toc_entry->index == 0 && toc_entry->type == 0) {
      break;
    }

    toc_entry = NULL;
  }

  if (!toc_entry) {
    g_array_set_size(nc->toc, nc->toc->len + 1);
    toc_entry = &g_array_index(nc->toc, tocentry_t, nc->toc->len - 1);
  }

  toc_entry->index = M_PBufGetCursor(&nc->messages);
  toc_entry->type = type;

  return &nc->messages;
}

bool N_ChannelReady(netchan_t *nc) {
  if (nc->toc->len == 0) {
    return false;
  }
  
  if (!nc->throttled) {
    return true;
  }

  return (I_GetTime() - nc->last_flush_tic) > 0;
}

pbuf_t* N_ChannelGetMessage(netchan_t *nc) {
  return &nc->messages;
}

bool N_ChannelLoadFromData(netchan_t *nc, unsigned char *data, size_t size) {
  size_t message_start_point = 0;

  N_ChannelClear(nc);

  if (!deserialize_toc(nc->toc, data, size, &message_start_point)) {
    P_Printf(consoleplayer,
      "N_ChannelLoadFromData: Error reading packet's TOC.\n"
    );
    return false;
  }

  if (message_start_point >= size) {
    P_Printf(consoleplayer, "N_ChannelLoadFromData: Received empty packet.\n");
    return false;
  }

  M_PBufSetData(
    &nc->messages, data + message_start_point, size - message_start_point
  );

  if (data[2] != 4) {
    D_Msg(MSG_NET, "Received packet (packet size %zu):\n", size);
    for (int i = 0; i < MIN(26, size); i++) {
      D_Msg(MSG_NET, "%02X ", data[i] & 0xFF);
    }
    D_Msg(MSG_NET, "\n");
  }

  return true;
}

bool N_ChannelLoadNextMessage(netchan_t *nc, net_message_e *message_type) {
  tocentry_t *toc_entry = NULL;
  bool valid_message = true;
  unsigned int i = 0;

  if (nc->toc->len == 0) {
    return false;
  }

  while (i < nc->toc->len) {
    toc_entry = &g_array_index(nc->toc, tocentry_t, i);

    if (toc_entry->index >= M_PBufGetSize(&nc->messages)) {
      P_Printf(consoleplayer,
        "N_PeerLoadNextMessage: Invalid message index (%u >= %zu).\n",
        toc_entry->index, M_PBufGetSize(&nc->messages)
      );
      valid_message = false;
      break;
    }

    *message_type = toc_entry->type;
    M_PBufSeek(&nc->messages, toc_entry->index);
    i++;
  }

  if (nc->toc->len > 0) {
    g_array_remove_range(nc->toc, 0, i);
  }

  return valid_message;
}

/* vi: set et ts=2 sw=2: */
