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

typedef struct netpeer_s {
  ENetPeer        *peer;
  msgpack_sbuffer *buf;
  msgpack_packer  *pk;
  time_t           connect_time;
  time_t           disconnect_time;
  int              playernum;
} netpeer_t;

int        N_AddPeer(void);
void       N_SetPeerConnected(int peernum, ENetPeer *peer);
void       N_SetPeerDisconnected(int peernum);
void       N_RemovePeer(netpeer_t *np);
int        N_GetPeerCount(void);
netpeer_t* N_GetPeer(int peernum);
int        N_GetPeerNum(ENetPeer *peer);
netpeer_t* N_GetPeerForPlayer(int playernum);
int        N_GetPeerNumForPlayer(int playernum);
dboolean   N_CheckPeerTimeout(int peernum);

#endif

