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


#ifndef N_PEER_H__
#define N_PEER_H__

typedef struct netchan_s {
  cbuf_t toc;
  pbuf_t messages;
  pbuf_t packet_data;
  pbuf_t packed_toc;
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
  unsigned int  peernum;
  unsigned int  playernum;
  ENetPeer     *peer;
  netcom_t      netcom;
  netsync_t     sync;
  time_t        connect_time;
  time_t        disconnect_time;
} netpeer_t;

typedef void netpeer_iter_t;

#define NETPEER_FOR_EACH(pin) \
  for (netpeeriternode_t (pin) = {NULL, NULL}; N_PeerIter(&pin.it, &pin.np);)

typedef struct netpeeriternode_s {
  netpeer_iter_t *it;
  netpeer_t *np;
} netpeeriternode_t;

void         N_InitPeers(void);
unsigned int N_PeerAdd(void);
void         N_PeerSetConnected(int peernum, ENetPeer *peer);
void         N_PeerSetDisconnected(int peernum);
void         N_PeerRemove(netpeer_t *np);
void         N_PeerIterRemove(netpeer_iter_t *it, netpeer_t *np);
unsigned int N_PeerGetCount(void);
netpeer_t*   N_PeerGet(int peernum);
netpeer_t*   CL_GetServerPeer(void);
unsigned int N_PeerForPeer(ENetPeer *peer);
netpeer_t*   N_PeerForPlayer(short playernum);
unsigned int N_PeerGetNumForPlayer(short playernum);
bool         N_PeerIter(netpeer_iter_t **it, netpeer_t **np);
dboolean     N_PeerCheckTimeout(int peernum);
void         N_PeerFlushBuffers(int peernum);
pbuf_t*      N_PeerBeginMessage(int peernum, net_channel_e chan_type,
                                            unsigned char type);
ENetPacket*  N_PeerGetPacket(int peernum, net_channel_e chan_type);
dboolean     N_PeerLoadIncoming(int peernum, unsigned char *data, size_t size);
dboolean     N_PeerLoadNextMessage(int peernum, unsigned char *message_type);
void         N_PeerClearReliable(int peernum);
void         N_PeerClearUnreliable(int peernum);
void         N_PeerResetSync(int peernum);

#endif

/* vi: set et ts=2 sw=2: */

