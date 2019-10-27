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

#include "n_main.h"
#include "n_proto.h"
#include "n_chan.h"
#include "n_com.h"

#define ENET_CHANNEL_RELIABLE 0
#define ENET_CHANNEL_UNRELIABLE 1

void N_ComInit(netcom_t *nc, void *enet_peer) {
  N_ChannelInit(&nc->incoming, false, false);
  /* Reliable and not throttled */
  N_ChannelInit(&nc->outgoing_reliable, true, false);
  /* Unreliable and not throttled */
  N_ChannelInit(&nc->outgoing_unreliable, false, true);
  nc->bytes_uploaded = 0;
  nc->bytes_downloaded = 0;
  nc->enet_peer = enet_peer;
}

void N_ComFree(netcom_t *nc) {
  N_ChannelFree(&nc->incoming);
  N_ChannelFree(&nc->outgoing_reliable);
  N_ChannelFree(&nc->outgoing_unreliable);
}

void N_ComReset(netcom_t *nc) {
  nc->bytes_uploaded = 0;
  nc->bytes_downloaded = 0;
}

size_t N_ComGetBytesUploaded(netcom_t *nc) {
  return nc->bytes_uploaded;
}

size_t N_ComGetBytesDownloaded(netcom_t *nc) {
  return nc->bytes_downloaded;
}

pbuf_t* N_ComBeginMessage(netcom_t *nc, net_message_e type) {
  if (type == NM_SYNC) {
    return N_ChannelBeginMessage(&nc->outgoing_unreliable, type);
  }

  return N_ChannelBeginMessage(&nc->outgoing_reliable, type);
}

bool N_ComSetIncoming(netcom_t *nc, unsigned char *data, size_t size) {
  if (!N_ChannelLoadFromData(&nc->incoming, data, size)) {
    return false;
  }

  nc->bytes_downloaded += size;

  return true;
}

bool N_ComLoadNextMessage(netcom_t *nc, net_message_e *message_type) {
  return N_ChannelLoadNextMessage(&nc->incoming, message_type);
}

void N_ComFlushReliableChannel(netcom_t *nc) {
  netchan_t *netchan = &nc->outgoing_reliable;
  ENetPacket *packet = NULL;

  if (!N_ChannelReady(netchan)) {
    return;
  }

  packet = N_ChannelGetPacket(netchan);

  enet_peer_send((ENetPeer *)nc->enet_peer, ENET_CHANNEL_RELIABLE, packet);

  N_ChannelClear(netchan);

  nc->bytes_uploaded += packet->dataLength;
}

void N_ComFlushUnreliableChannel(netcom_t *nc) {
  netchan_t *netchan = &nc->outgoing_unreliable;
  ENetPacket *packet = NULL;

  if (!N_ChannelReady(netchan)) {
    return;
  }

  packet = N_ChannelGetPacket(netchan);

  enet_peer_send((ENetPeer *)nc->enet_peer, ENET_CHANNEL_UNRELIABLE, packet);

  N_ChannelClear(netchan);

  nc->bytes_uploaded += packet->dataLength;
}

void N_ComClearReliableChannel(netcom_t *nc) {
  N_ChannelClear(&nc->outgoing_reliable);
}

void N_ComClearUnreliableChannel(netcom_t *nc) {
  N_ChannelClear(&nc->outgoing_unreliable);
}

void N_ComSendReset(netcom_t *nc) {
  enet_peer_reset((ENetPeer *)nc->enet_peer);
}

pbuf_t* N_ComGetIncomingMessageData(netcom_t *nc) {
  return N_ChannelGetMessage(&nc->incoming);
}

void* N_ComGetENetPeer(netcom_t *nc) {
  return nc->enet_peer;
}

/* vi: set et ts=2 sw=2: */
