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

#include <time.h>

#include <enet/enet.h>
#include <msgpack.h>

#include "lprintf.h"
#include "m_obuf.h"
#include "n_peer.h"
#include "n_net.h"

static objbuf_t *net_peers = NULL;

int N_AddPeer(void) {
  int peernum = -1;
  short playernum = -1;
  netpeer_t *np = NULL;

  if (server) {
    for (int i = 0; i < MAXPLAYERS; i++) {
      if (!playeringame[i]) {
        playernum = i;
      }
    }

    if (playernum == -1)
      return -1;
  }

  np = calloc(1, sizeof(netpeer_t));

  np->peer = NULL;
  np->rbuf = msgpack_sbuffer_new();
  np->rpk = msgpack_packer_new(np->buf, msgpack_sbuffer_write);
  np->ubuf = msgpack_sbuffer_new();
  np->upk = msgpack_packer_new(np->buf, msgpack_sbuffer_write);
  np->connect_time = time();
  np->disconnect_time = 0;
  np->playernum = playernum;

  if ((peernum = M_ObjBufferInsertAtFirstFreeSlot(net_peers, np)))
    return peernum;

  M_ObjBufferAppend(net_peers, np);
  return net_peers->size - 1;
}

void N_SetPeerConnected(int peernum, ENetPeer *peer) {
  netpeer_t *np = N_GetPeer(peernum);

  if (np == NULL)
    I_Error("N_SetPeerConnected: Invalid peer %d.\n", peernum);

  np->peer = peer;
  np->connect_time = 0;
}

void N_SetPeerDisconnected(int peernum) {
  netpeer_t *np = N_GetPeer(peernum);

  if (np == NULL)
    I_Error("N_SetPeerConnected: Invalid peer %d.\n", peernum);

  np->disconnect_time = time();
}

void N_RemovePeer(netpeer_t *np) {
  doom_printf("Removing peer %s:%u\n",
    N_IPToString(np->peer->address.host), np->peer->address.port
  );

  msgpack_sbuffer_free(np->rbuf);
  msgpack_packer_free(np->rpk);
  msgpack_sbuffer_free(np->ubuf);
  msgpack_packer_free(np->upk);

  np->peer            = NULL;
  np->rbuf            = NULL;
  np->rpk             = NULL;
  np->ubuf            = NULL;
  np->upk             = NULL;
  np->connect_time    = 0;
  np->disconnect_time = 0;
  np->playernum       = -1;

  M_ObjBufferRemove(net_peers, peernum);
}

int N_GetPeerCount(void) {
  return net_peers->size;
}

netpeer_t* N_GetPeer(int peernum) {
  if (peernum < 0 || peernum >= net_peers->size)
    N_Error("N_GetPeer: peernum %d out of range.\n", peernum);

  return net_peers->objects[i];
}

int N_GetPeerNum(ENetPeer *peer) {
  for (int i = 0; i < net_peers->size; i++) {
    netpeer_t *np = net_peers->objects[i];

    if (np != NULL && np->peer->connectID == peer->connectID)
      return i;
  }

  return -1;
}

netpeer_t* N_GetPeerForPlayer(short playernum) {
  for (int i = 0; i < net_peers->size; i++) {
    netpeer_t *np = net_peers->objects[i];

    if (np != NULL && np->playernum == playernum)
      return np;
  }

  return NULL;
}

int N_GetPeerNumForPlayer(short playernum) {
  for (int i = 0; i < net_peers->size; i++) {
    netpeer_t *np = net_peers->objects[i];

    if (np != NULL && np->playernum == playernum)
      return i;
  }

  return -1;
}

dboolean N_CheckPeerTimeout(int peernum) {
  netpeer_t *np = N_GetPeer(peernum);

  if (np != NULL) {
    if ((np->connection_time != 0) &&
        (difftime(time(), np->connection_time) > CONNECTION_TIMEOUT)) {
      return true;
    }
    else if ((np->disconnection_time != 0) &&
             (difftime(time(), np->disconnection_time) > CONNECTION_TIMEOUT)) {
      return true;
    }
  }

  return false;
}

/* vi: set cindent et ts=2 sw=2: */

