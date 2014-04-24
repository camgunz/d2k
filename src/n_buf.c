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

static void init_channel(netchan_t *nc, pbuf_mode_t mode) {
  M_PBufInit(&nc->header, mode);
  M_PBufInit(&nc->toc, mode);
  M_PBufInit(&nc->messages, mode);
  M_PBufInit(&nc->packet_data, mode);
}

static void clear_channel(netchan_t *nc) {
  M_PBufClear(&nc->header);
  M_PBufClear(&nc->toc);
  M_PBufClear(&nc->messages);
  M_PBufClear(&nc->packet_data);
}

void N_NBufInit(netbuf_t *nb) {
  init_channel(&nb->incoming, PBUF_MODE_READ);
  init_channel(&nb->reliable, PBUF_MODE_WRITE);
  init_channel(&nb->unreliable, PBUF_MODE_WRITE);
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

  return M_PBufGetSize(&nc->toc) > 0;
}

pbuf_t* N_NBufBeginMessage(netbuf_t *nb, net_channel_e chan, byte type) {
  netchan_t *nc = NULL;

  get_netchan(nc, nb, chan);

  M_PBufWriteUInt(&nc->toc, M_PBufGetCursor(&nc->messages));
  M_PBufWriteUChar(&nc->toc, type);

  return &nc->messages;
}

ENetPacket* N_NBufGetPacket(netbuf_t *nb, net_channel_e chan) {
  netchan_t *nc = NULL;
  size_t header_size, toc_size, msg_size, packet_size;
  ENetPacket *packet = NULL;

  get_netchan(nc, nb, chan);

  M_PBufClear(&nc->toc);
  toc_size = M_PBufGetSize(&nc->toc);
  M_PBufWriteUInt(&nc->header, toc_size);

  header_size = M_PBufGetSize(&nc->header);
  toc_size = M_PBufGetSize(&nc->toc);
  msg_size = M_PBufGetSize(&nc->messages);
  packet_size = header_size + toc_size + msg_size;

  if (chan == NET_CHANNEL_RELIABLE) {
    size_t msg_index = header_size + toc_size;

    packet = enet_packet_create(NULL, packet_size, ENET_PACKET_FLAG_RELIABLE);

    memcpy(packet->data, M_PBufGetData(&nc->header), header_size);
    memcpy(packet->data + header_size, M_PBufGetData(&nc->toc), toc_size);
    memcpy(packet->data + msg_index, M_PBufGetData(&nc->messages), msg_size);
  }
  else if (chan == NET_CHANNEL_UNRELIABLE) {
    buf_t *buf = M_PBufGetBuffer(&nc->packet_data);

    M_BufferClear(buf);
    M_BufferEnsureTotalCapacity(buf, packet_size);
    M_BufferWrite(buf, M_PBufGetData(&nc->header), header_size);
    M_BufferWrite(buf, M_PBufGetData(&nc->toc), toc_size);
    M_BufferWrite(buf, M_PBufGetData(&nc->messages), msg_size);

    packet = enet_packet_create(
      M_BufferGetData(buf),
      M_BufferGetSize(buf),
      ENET_PACKET_FLAG_UNSEQUENCED | ENET_PACKET_FLAG_NO_ALLOCATE
    );
  }

  return packet;
}

dboolean N_NBufLoadIncoming(netbuf_t *nb, unsigned char *data, size_t size) {
  netchan_t *incoming = &nb->incoming;
  unsigned int toc_size = 0;
  buf_t *buf = M_PBufGetBuffer(&incoming->packet_data);

  M_PBufSetData(&incoming->packet_data, data, size);

  if (!M_PBufReadUInt(&incoming->packet_data, &toc_size)) {
    doom_printf("N_NBufLoadIncoming: Error reading TOC size.\n");
    return false;
  }

  M_PBufSetData(
    &incoming->toc, M_BufferGetData(buf) + M_BufferGetCursor(buf), toc_size
  );

  incoming->message_index = 0;

  return true;
}

dboolean N_NBufLoadNextMessage(netbuf_t *nb, unsigned char *message_type) {
  netchan_t *incoming = &nb->incoming;
  buf_t *buf = M_PBufGetBuffer(&incoming->packet_data);
  unsigned int  m_message_index = 0;
  unsigned int  m_next_message_index = 0;
  unsigned char m_message_type = 0;
  size_t        message_size = 0;

  if (M_PBufAtEOF(&incoming->toc))
    return false;

  if (incoming->message_index == 0) {
    if (!M_PBufReadUInt(&incoming->messages, &m_message_index)) {
      doom_printf("N_NBufLoadNextMessage: Error reading message index.\n");
      return false;
    }
  }
  else {
    m_message_index = incoming->message_index;
  }

  if (!M_PBufReadUChar(&incoming->toc, &m_message_type)) {
    doom_printf("N_NBufLoadNextMessage: Error reading message type.\n");
    return false;
  }

  if (m_message_index <= incoming->message_index)
    return false;

  if (M_PBufAtEOF(&incoming->toc)) {
    m_next_message_index = M_PBufGetCursor(&incoming->toc);
  }
  else {
    if (!M_PBufReadUInt(&incoming->toc, &m_next_message_index)) {
      doom_printf(
        "N_NBufLoadNextMessage: Error reading next message index.\n"
      );
      return false;
    }
  }

  if (m_next_message_index <= m_message_index)
    return false;

  message_size = m_next_message_index - m_message_index;

  if (!M_BufferSeek(buf, m_message_index)) {
    doom_printf("N_NBufLoadNextMessage: Error seeking to message index.\n");
    return false;
  }

  M_PBufSetData(
    &incoming->messages,
    M_BufferGetData(buf) + M_BufferGetCursor(buf),
    message_size
  );

  incoming->message_index = m_next_message_index;

  return true;
}

void N_NBufFree(netbuf_t *nb) {
  M_PBufFree(&nb->incoming.header);
  M_PBufFree(&nb->incoming.toc);
  M_PBufFree(&nb->incoming.messages);
  M_PBufFree(&nb->incoming.packet_data);
  M_PBufFree(&nb->reliable.header);
  M_PBufFree(&nb->reliable.toc);
  M_PBufFree(&nb->reliable.messages);
  M_PBufFree(&nb->reliable.packet_data);
  M_PBufFree(&nb->unreliable.header);
  M_PBufFree(&nb->unreliable.toc);
  M_PBufFree(&nb->unreliable.messages);
  M_PBufFree(&nb->unreliable.packet_data);
}

/* vi: set et ts=2 sw=2: */

