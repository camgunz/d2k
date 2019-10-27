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

#include "m_idhash.h"

struct player_s;
typedef struct player_s player_t;

typedef enum {
  PEER_TYPE_CLIENT;
  PEER_TYPE_SERVER;
  PEER_TYPE_OTHER_CLIENT;
} peer_type_e;

typedef enum {
  CLIENT_TYPE_BASIC,
  CLIENT_TYPE_SPECTATOR,
  CLIENT_TYPE_PLAYER,
} client_type_e;

typedef struct client_info_s {
  uint32_t       peer_id;
  char          *name;
  client_type_e  status;
  auth_level_e   auth_level;
  int            connect_tic;
  unsigned int   ping;
  team_t        *team;
  player_t      *player;
} client_info_t;

typedef struct net_link_s {
  netcom_t   com;
  netsync_t  sync;
  time_t     connection_start_time;
  time_t     disconnection_start_time;
} net_link_t;

typedef struct client_s {
  net_link_t    link;
  client_info_t info;
  time_t        last_setup_request_time;
} client_t;

typedef struct server_s {
  net_link_t  link;
} server_t;

typedef struct net_peer_s {
  uint32_t    id;
  peer_type_e type;
  union {
    client_t      client;
    server_t      server;
    client_info_t other_client;
  } as;
} net_peer_t;

typedef struct {
  id_hash_iterator_t iter;
  net_peer_t *np;
} net_peer_iterator_t;

#define NET_PEER_FOR_EACH(_it) \
  for (net_peer_iterator_t _it = { { 0 }, NULL }; N_PeerIterate(&_it);)

void        N_PeersInit(void);
net_peer_t* N_PeersAdd(void *base_net_peer);
net_peer_t* N_PeersAddRaw(uint32_t id);
net_peer_t* N_PeersLookup(uint32_t id);
net_peer_t* N_PeersLookupByENetPeer(void *enet_peer);
net_peer_t* N_PeersLookupByPlayer(player_t *player);
size_t      N_PeersGetCount(void);
bool        N_PeersPeerExists(uint32_t id);

bool        N_PeerIterate(netpeer_iterator_t *iter);
void        N_PeerIterateRemove(netpeer_iterator_t *iter);

void       N_PeerRemove(net_peer_t *np);
bool       N_PeerCheckTimeout(net_peer_t *np);
bool       N_PeerCanRequestSetup(net_peer_t *np);
void       N_PeerSetConnected(net_peer_t *np);
bool       N_PeerTooLagged(net_peer_t *np);
void       N_PeerDisconnect(net_peer_t *np, disconnection_reason_e reason);

uint32_t         N_PeerGetID(net_peer_t *np);
uint32_t         N_PeerGetIPAddress(net_peer_t *np);
const char*      N_PeerGetIPAddressConstString(net_peer_t *np);
uint16_t         N_PeerGetPort(net_peer_t *np);
netpeer_status_e N_PeerGetStatus(net_peer_t *np);
void             N_PeerSetStatus(net_peer_t *np, netpeer_status_e status);
auth_level_e     N_PeerGetAuthLevel(net_peer_t *np);
void             N_PeerSetAuthLevel(net_peer_t *np, auth_level_e auth_level);
unsigned int     N_PeerGetPing(net_peer_t *np);
void             N_PeerSetPing(net_peer_t *np, unsigned int ping);
int              N_PeerGetConnectTic(net_peer_t *np);
const char*      N_PeerGetName(net_peer_t *np);
void             N_PeerSetName(net_peer_t *np, const char *name);
team_t*          N_PeerGetTeam(net_peer_t *np);
void             N_PeerSetTeam(net_peer_t *np, team_t *team);
bool             N_PeerHasTeam(net_peer_t *np);
void             N_PeerSetTeamRaw(net_peer_t *np, team_t *team);
void             N_PeerSetTeam(net_peer_t *np, team_t *team);
player_t*        N_PeerGetPlayer(net_peer_t *np);
void             N_PeerSetPlayer(net_peer_t *np, player_t *player);
bool             N_PeerHasPlayer(net_peer_t *np);

void    N_PeerFlushReliableChannel(net_peer_t *np);
void    N_PeerFlushUnreliableChannel(net_peer_t *np);
void    N_PeerFlushChannels(net_peer_t *np);
pbuf_t* N_PeerBeginMessage(net_peer_t *np, net_message_e type);
bool    N_PeerSetIncoming(net_peer_t *np, unsigned char *data, size_t size);
bool    N_PeerLoadNextMessage(net_peer_t *np, net_message_e *message_type);
pbuf_t* N_PeerGetIncomingMessageData(net_peer_t *np);
void    N_PeerClearReliableChannel(net_peer_t *np);
void    N_PeerClearUnreliableChannel(net_peer_t *np);
void    N_PeerSendReset(net_peer_t *np);
size_t  N_PeerGetBytesUploaded(net_peer_t *np);
size_t  N_PeerGetBytesDownloaded(net_peer_t *np);

void*   N_PeerGetENetPeer(net_peer_t *np);
float   N_PeerGetPacketLoss(net_peer_t *np);
float   N_PeerGetPacketLossJitter(net_peer_t *np);

bool N_PeerSyncNeedsGameInfo(net_peer_t *np);
void N_PeerSyncSetNeedsGameInfo(net_peer_t *np);
void N_PeerSyncSetHasGameInfo(net_peer_t *np);
bool N_PeerSyncNeedsGameState(net_peer_t *np);
void N_PeerSyncSetNeedsGameState(net_peer_t *np);
void N_PeerSyncSetHasGameState(net_peer_t *np);
bool N_PeerSynchronized(net_peer_t *np);
bool N_PeerSyncOutdated(net_peer_t *np);
void N_PeerSyncSetOutdated(net_peer_t *np);
void N_PeerSyncSetNotOutdated(net_peer_t *np);
bool N_PeerSyncUpdated(net_peer_t *np);
void N_PeerSyncSetUpdated(net_peer_t *np);
void N_PeerSyncSetNotUpdated(net_peer_t *np);
void N_PeerSyncReset(net_peer_t *np);

int  N_PeerSyncGetTIC(net_peer_t *np);
void N_PeerSyncSetTIC(net_peer_t *np, int sync_tic);
void N_PeerSyncUpdateTIC(net_peer_t *np, int sync_tic);

uint32_t N_PeerSyncGetCommandIndex(net_peer_t *np);
void     N_PeerSyncSetCommandIndex(net_peer_t *np, uint32_t command_index);
void     N_PeerSyncUpdateCommandIndex(net_peer_t *np, uint32_t command_index);

game_state_delta_t* N_PeerSyncGetStateDelta(net_peer_t *np);
void                N_PeerSyncUpdateStateDelta(net_peer_t *np,
                                               int from_tic,
                                               int to_tic,
                                               pbuf_t *delta_data);
void                N_PeerSyncBuildNewStateDelta(net_peer_t *np);
bool                N_PeerSyncStateDeltaUpdated(net_peer_t *np);
void                N_PeerSyncResetStateDelta(net_peer_t *np, int sync_tic,
                                                             int from_tic,
                                                             int to_tic);

uint32_t N_PeerSyncGetCommandIndexForPlayer(net_peer_t *np, player_t *player);
void     N_PeerSyncSetCommandIndexForPlayer(net_peer_t *np,
                                            player_t *player,
                                            uint32_t command_index);
void     N_PeerSyncUpdateCommandIndexForPlayer(net_peer_t *np,
                                               player_t *player,
                                               uint32_t command_index);

#endif

/* vi: set et ts=2 sw=2: */
