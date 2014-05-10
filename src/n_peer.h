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

#ifndef N_PEER_H__
#define N_PEER_H__

/* CG: TODO: Break out sync stuff into its own netsync_t thing */

typedef struct netchan_s {
  cbuf_t toc;
  pbuf_t messages;
  pbuf_t packet_data;
} netchan_t;

typedef struct netcom_s {
  netchan_t incoming;
  netchan_t reliable;
  netchan_t unreliable;
} netcom_t;

typedef struct netsync_s {
  dboolean           initialized;
  dboolean           outdated;
  int                tic;
  int                cmd;
  game_state_delta_t delta;
} netsync_t;

typedef struct netpeer_s {
  unsigned short      peernum;
  unsigned short      playernum;
  ENetPeer           *peer;
  netcom_t            netcom;
  netsync_t           sync;
  time_t              connect_time;
  time_t              disconnect_time;
} netpeer_t;

void        N_InitPeers(void);
int         N_PeerAdd(void);
void        N_PeerSetConnected(int peernum, ENetPeer *peer);
void        N_PeerSetDisconnected(int peernum);
void        N_PeerRemove(netpeer_t *np);
int         N_PeerGetCount(void);
netpeer_t*  N_PeerGet(int peernum);
int         N_PeerGetNum(ENetPeer *peer);
netpeer_t*  N_PeerForPlayer(short playernum);
int         N_PeerGetNumForPlayer(short playernum);
dboolean    N_PeerCheckTimeout(int peernum);
void        N_PeerFlushBuffers(int peernum);
pbuf_t*     N_PeerBeginMessage(int peernum, net_channel_e chan_type,
                                            unsigned char type);
ENetPacket* N_PeerGetPacket(int peernum, net_channel_e chan_type);
dboolean    N_PeerLoadIncoming(int peernum, unsigned char *data, size_t size);
dboolean    N_PeerLoadNextMessage(int peernum, unsigned char *message_type);
void        N_PeerClearReliable(int peernum);
void        N_PeerClearUnreliable(int peernum);

#endif

/* vi: set et ts=2 sw=2: */

