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
size_t      N_PeersGetCount(void);
bool        N_PeersPeerExists(uint32_t id);
bool        N_PeersIterate(netpeer_iterator_t *iter);
void        N_PeersIterateRemove(netpeer_iterator_t *iter);

uint32_t N_PeerGetID(net_peer_t *np);
void     N_PeerRemove(net_peer_t *np);


client_type_e N_ClientGetType(client_info_t *ci);
void          N_ClientSetType(client_info_t *ci, client_type_e type);
auth_level_e  N_ClientGetAuthLevel(client_info_t *ci);
void          N_ClientSetAuthLevel(client_info_t *ci, auth_level_e auth_level);
const char*   N_ClientGetName(client_info_t *ci);
void          N_ClientSetName(client_info_t *ci, const char *name);
team_t*       N_ClientGetTeam(client_info_t *ci);
void          N_ClientSetTeam(client_info_t *ci, team_t *team);
void          N_ClientSetTeamRaw(client_info_t *ci, team_t *team);
bool          N_ClientHasTeam(client_info_t *ci);
player_t*     N_ClientGetPlayer(client_info_t *ci);
void          N_ClientSetPlayer(client_info_t *ci, player_t *player);
bool          N_ClientHasPlayer(client_info_t *ci);

#endif

/* vi: set et ts=2 sw=2: */
