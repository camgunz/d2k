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

#ifndef N_BUF_H__
#define N_BUF_H__

typedef struct netchan_s {
  pbuf_t header;
  pbuf_t toc;
  pbuf_t messages;
  pbuf_t packet_data;
  unsigned int message_index;
} netchan_t;

typedef struct netbuf_s {
  netchan_t incoming;
  netchan_t reliable;
  netchan_t unreliable;
} netbuf_t;

void        N_NBufInit(netbuf_t *nb);
void        N_NBufClear(netbuf_t *nb);
void        N_NBufClearIncoming(netbuf_t *nb);
void        N_NBufClearChannel(netbuf_t *nb, net_channel_e chan);
dboolean    N_NBufChannelEmpty(netbuf_t *nb, net_channel_e chan);
pbuf_t*     N_NBufBeginMessage(netbuf_t *nb, net_channel_e chan, byte type);
ENetPacket* N_NBufGetPacket(netbuf_t *nb, net_channel_e chan);
dboolean    N_NBufLoadIncoming(netbuf_t *nb, unsigned char *data, size_t size);
dboolean    N_NBufLoadNextMessage(netbuf_t *nb, unsigned char *message_type);
void        N_NBufFree(netbuf_t *nb);

#endif

/* vi: set et ts=2 sw=2: */

