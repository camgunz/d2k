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

#include "g_game.h"
#include "g_state.h"
#include "pl_main.h"

#include "n_addr.h"
#include "n_main.h"
#include "n_chan.h"
#include "n_com.h"
#include "n_peer.h"
#include "n_sync.h"

#define MAX_SETUP_REQUEST_INTERVAL 2.0

static id_hash_t net_peers;

static void peer_free(net_peer_t *np) {
  switch (np->type) {
    case PEER_TYPE_CLIENT:
      N_ComFree(&np->as.client.link.com);
      N_SyncFree(&np->as.client.link.sync);
      if (np->as.client.info.player) {
        np->as.client.info.player->playerstate = PST_DISCONNECTED;
      }
      break;
    case PEER_TYPE_SERVER:
      N_ComFree(&np->as.client.link.com);
      N_SyncFree(&np->as.client.link.sync);
      break;
    case PEER_TYPE_OTHER_CLIENT:
      if (ci->player) {
        player->playerstate = PST_DISCONNECTED;
      }
      break;
  }
}

static void peer_destroy_func(gpointer data) {
  free_peer((net_peer_t *)data);
}

void N_PeersInit(void) {
  M_IDHashInit(&net_peers, peer_destroy_func);
}

net_peer_t* peer_new(peer_type_e type) {
  net_peer_t *np = calloc(1, sizeof(net_peer_t));

  if (!np) {
    I_Error("N_PeersAdd: Error allocating memory for new net peer\n");
  }

  np->id = M_IDHashAdd(&net_peers, np);
  np->type = type;

  return np;
}

net_peer_t* client_peer_new(void *base_net_peer) {
  net_peer_t *np = peer_new(PEER_TYPE_CLIENT);

  N_ComInit(&np->as.client.link.com, base_net_peer);
  N_SyncInit(&np->as.client.link.sync);

net_peer_t* N_PeersAdd(void *enet_peer) {
  net_peer_t *np = NULL;

  np = calloc(1, sizeof(net_peer_t));

  if (!np) {
    I_Error("N_PeersAdd: Error allocating memory for new net peer\n");
  }

  np->connect_start_time = time(NULL);
  np->connect_tic = gametic;

  N_ComInit(&np->com, enet_peer);
  N_SyncInit(&np->sync);

  if (SERVER) {
    N_SyncSetTIC(&np->sync, gametic);
  }

  np->id = M_IDHashAdd(&net_peers, np);

  return np;
}

net_peer_t* N_PeersLookup(uint32_t id) {
  return M_IDHashLookup(&net_peers, id);
}

size_t N_PeersGetCount(void) {
  return M_IDHashGetCount(&net_peers);
}

bool N_PeersPeerExists(uint32_t id) {
  if (N_PeersLookup(id)) {
    return true;
  }

  return false;
}

bool N_PeerIterate(netpeer_iterator_t *iter) {
  if (!M_IDHashIterate(&net_peers, &iter->iter)) {
    return false;
  }

  iter->np = (net_peer_t *)iter->iter.obj;

  return true;
}

void N_PeerIterateRemove(netpeer_iterator_t *iter) {
  M_IDHashIterateRemove(&iter->iter);
}

void N_PeerRemove(net_peer_t *np) {
  M_IDHashRemoveID(&net_peers, np->id);
}

bool N_PeerCanRequestSetup(net_peer_t *np) {
  time_t now = time(NULL);

  if (np->last_setup_request_time == 0) {
    return true;
  }

  return (
    difftime(now, np->last_setup_request_time) <= MAX_SETUP_REQUEST_INTERVAL
  );
}

uint32_t N_PeerGetID(net_peer_t *np) {
  return np->id;
}

netpeer_status_e N_PeerGetStatus(net_peer_t *np) {
  return np->status;
}

void N_PeerSetStatus(net_peer_t *np, netpeer_status_e status) {
  np->status = status;
}

auth_level_e N_PeerGetAuthLevel(net_peer_t *np) {
  return np->auth_level;
}

void N_PeerSetAuthLevel(net_peer_t *np, auth_level_e auth_level) {
  np->auth_level = auth_level;
}

unsigned int N_PeerGetPing(net_peer_t *np) {
  return np->ping;
}

void N_PeerSetPing(net_peer_t *np, unsigned int ping) {
  np->ping = ping;
}

int N_PeerGetConnectTic(net_peer_t *np) {
  return np->connect_tic;
}

const char* N_PeerGetName(net_peer_t *np) {
  return np->name;
}

void N_PeerSetName(net_peer_t *np, const char *name) {
  if (np->name) {
    free(np->name);
  }

  np->name = strdup(name);
}

team_t* N_PeerGetTeam(net_peer_t *np) {
  return np->team;
}

void N_PeerSetTeam(net_peer_t *np, team_t *team) {
  np->team = team;
}

bool N_PeerHasTeam(net_peer_t *np) {
  if (np->team) {
    return true;
  }

  return false;
}

player_t* N_PeerGetPlayer(net_peer_t *np) {
  if (!np->player_id) {
    return NULL;
  }

  return P_PlayersLookup(np->player_id);
}

void N_PeerSetPlayer(net_peer_t *np, player_t *player) {
  np->player_id = player->id;
}

bool N_PeerHasPlayer(net_peer_t *np) {
  player_t *player = N_PeerGetPlayer(np);

  if (player) {
    return true;
  }

  return false;
}

bool N_PeerSyncOutdated(net_peer_t *np) {
  return N_SyncOutdated(&np->sync);
}

void N_PeerSyncSetOutdated(net_peer_t *np) {
  N_SyncSetOutdated(&np->sync);
}

void N_PeerSyncSetNotOutdated(net_peer_t *np) {
  N_SyncSetNotOutdated(&np->sync);
}

bool N_PeerSyncNeedsGameInfo(net_peer_t *np) {
  return N_SyncNeedsGameInfo(&np->sync);
}

void N_PeerSyncSetNeedsGameInfo(net_peer_t *np) {
  N_SyncSetNeedsGameInfo(&np->sync);
}

void N_PeerSyncSetHasGameInfo(net_peer_t *np) {
  N_SyncSetHasGameInfo(&np->sync);
}

bool N_PeerSyncNeedsGameState(net_peer_t *np) {
  return N_SyncNeedsGameState(&np->sync);
}

void N_PeerSyncSetNeedsGameState(net_peer_t *np) {
  N_SyncSetNeedsGameState(&np->sync);
}

void N_PeerSyncSetHasGameState(net_peer_t *np) {
  N_SyncSetHasGameState(&np->sync);
}

bool N_PeerSynchronized(net_peer_t *np) {
  return !(N_PeerSyncNeedsGameInfo(np) || N_PeerSyncNeedsGameState(np));
}

bool N_PeerSyncUpdated(net_peer_t *np) {
  return N_SyncUpdated(&np->sync);
}

void N_PeerSyncSetUpdated(net_peer_t *np) {
  N_SyncSetUpdated(&np->sync);
}

void N_PeerSyncSetNotUpdated(net_peer_t *np) {
  N_SyncSetNotUpdated(&np->sync);
}

int N_PeerSyncGetTIC(net_peer_t *np) {
  return N_SyncGetTIC(&np->sync);
}

void N_PeerSyncSetTIC(net_peer_t *np, int sync_tic) {
  N_SyncSetTIC(&np->sync, sync_tic);
}

void N_PeerSyncUpdateTIC(net_peer_t *np, int sync_tic) {
  N_SyncUpdateTIC(&np->sync, sync_tic);
}

unsigned int N_PeerSyncGetCommandIndex(net_peer_t *np) {
  return N_SyncGetCommandIndex(&np->sync);
}

void N_PeerSyncSetCommandIndex(net_peer_t *np, unsigned int command_index) {
  N_SyncSetCommandIndex(&np->sync, command_index);
}

void N_PeerSyncUpdateCommandIndex(net_peer_t *np, unsigned int command_index) {
  N_SyncUpdateCommandIndex(&np->sync, command_index);
}

game_state_delta_t* N_PeerSyncGetStateDelta(net_peer_t *np) {
  return N_SyncGetStateDelta(&np->sync);
}

void N_PeerSyncUpdateStateDelta(net_peer_t *np, int from_tic,
                                               int to_tic,
                                               pbuf_t *delta_data) {
  N_SyncUpdateStateDelta(&np->sync, from_tic, to_tic, delta_data);
}

void N_PeerSyncBuildNewStateDelta(net_peer_t *np) {
  N_SyncBuildNewStateDelta(&np->sync);
}

bool N_PeerSyncStateDeltaUpdated(net_peer_t *np) {
  return N_SyncUpdated(&np->sync);
}

void N_PeerSyncResetStateDelta(net_peer_t *np, int sync_tic, int from_tic,
                                                            int to_tic) {
  N_SyncResetStateDelta(&np->sync, sync_tic, from_tic, to_tic);
}

uint32_t N_PeerSyncGetCommandIndexForPlayer(net_peer_t *np, player_t *player) {
  return N_SyncGetCommandIndexForPlayer(&np->sync, player);
}

void N_PeerSyncSetCommandIndexForPlayer(net_peer_t *np, player_t *player,
                                                       uint32_t command_index) {
  N_SyncSetCommandIndexForPlayer(&np->sync, player, command_index);
}

void N_PeerSyncUpdateCommandIndexForPlayer(net_peer_t *np,
                                           player_t *player,
                                           uint32_t command_index) {
  N_SyncUpdateCommandIndexForPlayer(&np->sync, player, command_index);
}

void N_PeerSyncReset(net_peer_t *np) {
  N_SyncReset(&np->sync);
}

/* vi: set et ts=2 sw=2: */
