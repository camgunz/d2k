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

static obuf_t *net_peers = NULL;

int N_AddPeer(void) {
  netpeer_t *np = calloc(1, sizeof(netpeer_t));

  /* CG: TODO: Add some kind of check for MAXCLIENTS */

  np->peer = NULL;
  np->rbuf = msgpack_sbuffer_new();
  np->rpk = msgpack_packer_new(np->buf, msgpack_sbuffer_write);
  np->ubuf = msgpack_sbuffer_new();
  np->upk = msgpack_packer_new(np->buf, msgpack_sbuffer_write);
  np->connect_time = time();
  np->disconnect_time = 0;
  np->playernum = 0;
  np->last_sync_received_tic = 0;
  np->last_sync_sent_tic = 0;
  M_BufferInit(&np->state);
  M_BufferInit(&np->delta);

  np->playernum = M_OBufInsertAtFirstFreeSlotOrAppend(netpeers, np);

  return np->playernum;
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
  M_BufferClear(&np->state);
  M_BufferFree(&np->state);
  M_BufferClear(&np->delta);
  M_BufferFree(&np->delta);

  np->peer                   = NULL;
  np->rbuf                   = NULL;
  np->rpk                    = NULL;
  np->ubuf                   = NULL;
  np->upk                    = NULL;
  np->connect_time           = 0;
  np->disconnect_time        = 0;
  np->playernum              = -1;
  np->last_sync_received_tic = 0;
  np->last_sync_sent_tic     = 0;
  np->state                  = NULL;
  np->delta                  = NULL;

  M_OBufRemove(net_peers, peernum);
}

int N_GetPeerCount(void) {
  return M_OBufGetObjectCount(net_peers);
}

netpeer_t* N_GetPeer(int peernum) {
  return M_OBufGet(net_peers, peernum);
}

int N_GetPeerNum(ENetPeer *peer) {
  int index = -1;
  netpeer_t *np = NULL;

  while (M_OBufIter(net_peers, &index, (void **)&np)) {
    if (np->peer->connectID == peer->connectID) {
      return index;
    }
  }

  return -1;
}

netpeer_t* N_GetPeerForPlayer(short playernum) {
  int index = -1;
  netpeer_t *np = NULL;

  while (M_OBufIter(net_peers, &index, (void **)&np)) {
    if (np->playernum == playernum) {
      return np;
    }
  }

  return NULL;
}

int N_GetPeerNumForPlayer(short playernum) {
  int index = -1;
  netpeer_t *np = NULL;

  while (M_OBufIter(net_peers, &index, (void **)&np)) {
    if (np->playernum == playernum) {
      return index;
    }
  }

  return -1;
}

dboolean N_CheckPeerTimeout(int peernum) {
  netpeer_t *np = N_GetPeer(peernum);

  if (np == NULL) {
    return false;
  }
  else if ((np->connection_time != 0) &&
           (difftime(time(), np->connection_time) > CONNECT_TIMEOUT)) {
    return true;
  }
  else if ((np->disconnection_time != 0) &&
           (difftime(time(), np->disconnection_time) > DISCONNECT_TIMEOUT)) {
    return true;
  }

  return false;
}

/* vi: set et ts=2 sw=2: */

