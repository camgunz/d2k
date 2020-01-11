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

struct team_s;
typedef struct team_s team_t;

struct game_state_delta_s;
typedef struct game_state_delta_s game_state_delta_t;

struct netpeer_s;
typedef struct netpeer_s netpeer_t;

typedef enum {
  NETPEER_STATUS_BASE,
  NETPEER_STATUS_SPECTATOR,
  NETPEER_STATUS_PLAYER
} netpeer_status_e;

typedef struct {
  id_hash_iterator_t iter;
  netpeer_t *np;
} netpeer_iterator_t;

#define NETPEER_FOR_EACH(_it) \
  for (netpeer_iterator_t _it = { { 0 }, NULL }; N_PeerIterate(&_it);)

void       N_PeersInit(void);
netpeer_t* N_PeersAdd(void *enet_peer);
netpeer_t* N_PeersAddRaw(uint32_t id);
netpeer_t* N_PeersLookup(uint32_t id);
netpeer_t* N_PeersLookupByENetPeer(void *enet_peer);
netpeer_t* N_PeersLookupByPlayer(player_t *player);
size_t     N_PeersGetCount(void);
bool       N_PeersPeerExists(uint32_t id);

bool       N_PeerIterate(netpeer_iterator_t *iter);
void       N_PeerIterateRemove(netpeer_iterator_t *iter);

void       N_PeerRemove(netpeer_t *np);
bool       N_PeerCheckTimeout(netpeer_t *np);
bool       N_PeerCanRequestSetup(netpeer_t *np);
void       N_PeerSetConnected(netpeer_t *np);
bool       N_PeerTooLagged(netpeer_t *np);
void       N_PeerDisconnect(netpeer_t *np, disconnection_reason_e reason);

uint32_t         N_PeerGetID(netpeer_t *np);
uint32_t         N_PeerGetIPAddress(netpeer_t *np);
const char*      N_PeerGetIPAddressConstString(netpeer_t *np);
uint16_t         N_PeerGetPort(netpeer_t *np);
netpeer_status_e N_PeerGetStatus(netpeer_t *np);
void             N_PeerSetStatus(netpeer_t *np, netpeer_status_e status);
double           N_PeerGetConnectionWaitTime(netpeer_t *np);
double           N_PeerGetDisconnectionWaitTime(netpeer_t *np);
double           N_PeerGetLastSetupRequestTime(netpeer_t *np);
void             N_PeerUpdateLastSetupRequestTime(netpeer_t *np);
auth_level_e     N_PeerGetAuthLevel(netpeer_t *np);
void             N_PeerSetAuthLevel(netpeer_t *np, auth_level_e auth_level);
unsigned int     N_PeerGetPing(netpeer_t *np);
void             N_PeerSetPing(netpeer_t *np, unsigned int ping);
int              N_PeerGetConnectTic(netpeer_t *np);
const char*      N_PeerGetName(netpeer_t *np);
void             N_PeerSetName(netpeer_t *np, const char *name);
team_t*          N_PeerGetTeam(netpeer_t *np);
void             N_PeerSetTeam(netpeer_t *np, team_t *team);
bool             N_PeerHasTeam(netpeer_t *np);
void             N_PeerSetTeamRaw(netpeer_t *np, team_t *team);
void             N_PeerSetTeam(netpeer_t *np, team_t *team);
player_t*        N_PeerGetPlayer(netpeer_t *np);
void             N_PeerSetPlayer(netpeer_t *np, player_t *player);
bool             N_PeerHasPlayer(netpeer_t *np);

void    N_PeerFlushReliableChannel(netpeer_t *np);
void    N_PeerFlushUnreliableChannel(netpeer_t *np);
void    N_PeerFlushChannels(netpeer_t *np);
pbuf_t* N_PeerBeginMessage(netpeer_t *np, net_message_e type);
bool    N_PeerSetIncoming(netpeer_t *np, unsigned char *data, size_t size);
bool    N_PeerLoadNextMessage(netpeer_t *np, net_message_e *message_type);
pbuf_t* N_PeerGetIncomingMessageData(netpeer_t *np);
void    N_PeerClearReliableChannel(netpeer_t *np);
void    N_PeerClearUnreliableChannel(netpeer_t *np);
void    N_PeerSendReset(netpeer_t *np);
size_t  N_PeerGetBytesUploaded(netpeer_t *np);
size_t  N_PeerGetBytesDownloaded(netpeer_t *np);

void*   N_PeerGetENetPeer(netpeer_t *np);
float   N_PeerGetPacketLoss(netpeer_t *np);
float   N_PeerGetPacketLossJitter(netpeer_t *np);

bool N_PeerSyncNeedsGameInfo(netpeer_t *np);
void N_PeerSyncSetNeedsGameInfo(netpeer_t *np);
void N_PeerSyncSetHasGameInfo(netpeer_t *np);
bool N_PeerSyncNeedsGameState(netpeer_t *np);
void N_PeerSyncSetNeedsGameState(netpeer_t *np);
void N_PeerSyncSetHasGameState(netpeer_t *np);
bool N_PeerSynchronized(netpeer_t *np);
bool N_PeerSyncOutdated(netpeer_t *np);
void N_PeerSyncSetOutdated(netpeer_t *np);
void N_PeerSyncSetNotOutdated(netpeer_t *np);
bool N_PeerSyncUpdated(netpeer_t *np);
void N_PeerSyncSetUpdated(netpeer_t *np);
void N_PeerSyncSetNotUpdated(netpeer_t *np);
void N_PeerSyncReset(netpeer_t *np);

int  N_PeerSyncGetTIC(netpeer_t *np);
void N_PeerSyncSetTIC(netpeer_t *np, int sync_tic);
void N_PeerSyncUpdateTIC(netpeer_t *np, int sync_tic);

uint32_t N_PeerSyncGetCommandIndex(netpeer_t *np);
void     N_PeerSyncSetCommandIndex(netpeer_t *np, uint32_t command_index);
void     N_PeerSyncUpdateCommandIndex(netpeer_t *np, uint32_t command_index);

game_state_delta_t* N_PeerSyncGetStateDelta(netpeer_t *np);
void                N_PeerSyncUpdateStateDelta(netpeer_t *np,
                                               int from_tic,
                                               int to_tic,
                                               pbuf_t *delta_data);
void                N_PeerSyncBuildNewStateDelta(netpeer_t *np);
bool                N_PeerSyncStateDeltaUpdated(netpeer_t *np);
void                N_PeerSyncResetStateDelta(netpeer_t *np, int sync_tic,
                                                             int from_tic,
                                                             int to_tic);

uint32_t N_PeerSyncGetCommandIndexForPlayer(netpeer_t *np, player_t *player);
void     N_PeerSyncSetCommandIndexForPlayer(netpeer_t *np,
                                            player_t *player,
                                            uint32_t command_index);
void     N_PeerSyncUpdateCommandIndexForPlayer(netpeer_t *np,
                                               player_t *player,
                                               uint32_t command_index);

#endif

/* vi: set et ts=2 sw=2: */
