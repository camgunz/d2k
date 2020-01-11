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
#include "n_main.h"
#include "p_user.h"
#include "n_chan.h"
#include "n_com.h"
#include "n_sync.h"

#define NET_THROTTLE_INTERVAL 300
#define NET_THROTTLE_ACCEL 2
#define NET_THROTTLE_DECEL 1

#define MAX_SETUP_REQUEST_INTERVAL 2.0

struct netpeer_s {
  netpeer_status_e status;
  uint32_t         id;
  uint32_t         player_id;
  netcom_t         com;
  netsync_t        sync;
  time_t           connect_start_time;
  time_t           disconnect_start_time;
  time_t           last_setup_request_time;
  auth_level_e     auth_level;
};

static GHashTable *net_peers = NULL;

static void free_peer(netpeer_t *np) {
  P_Printf(consoleplayer, "Removing peer for %u (%s:%u)\n",
    np->playernum,
    N_PeerGetIPAddressConstString(np),
    N_PeerGetPort(np)
  );

  players[np->playernum].playerstate = PST_DISCONNECTED;
  N_ComFree(&np->com);
  N_SyncFree(&np->sync);
  free(np);
}

static void peer_destroy_func(gpointer data) {
  free_peer((netpeer_t *)data);
}

unsigned int N_PeerGetPlayernum(netpeer_t *np) {
  return np->playernum;
}

void N_PeerSetPlayernum(netpeer_t *np, unsigned int playernum) {
  np->playernum = playernum;
}

double N_PeerGetConnecionWaitTime(netpeer_t *np) {
  return difftime(time(NULL), np->connect_start_time);
}

double N_PeerGetDisconnectionWaitTime(netpeer_t *np) {
  return difftime(time(NULL), np->disconnect_start_time);
}

double N_PeerGetLastSetupRequestTime(netpeer_t *np) {
  return difftime(time(NULL), np->last_setup_request_time);
}

void N_PeerUpdateLastSetupRequestTime(netpeer_t *np) {
  np->last_setup_request_time = time(NULL);
}

auth_level_e N_PeerGetAuthLevel(netpeer_t *np) {
  return np->auth_level;
}

void N_PeerSetAuthLevel(netpeer_t *np, auth_level_e auth_level) {
  np->auth_level = auth_level;
}

bool N_PeerCheckTimeout(netpeer_t *np) {
  time_t now = time(NULL);

  if (np->connect_start_time != 0) {
    if (difftime(now, np->connect_start_time) >
        (NET_CONNECT_TIMEOUT * 1000)) {
      return true;
    }
  }

  if (np->disconnect_start_time != 0) {
    if (difftime(now, np->disconnect_start_time) >
        (NET_DISCONNECT_TIMEOUT * 1000)) {
      return true;
    }
  }

  return false;
}

bool N_PeerCanRequestSetup(netpeer_t *np) {
  time_t now = time(NULL);

  if (np->last_setup_request_time == 0) {
    return true;
  }

  return (
    difftime(now, np->last_setup_request_time) <= MAX_SETUP_REQUEST_INTERVAL
  );
}

uint32_t N_PeerGetID(netpeer_t *np) {
  return np->id;
}

netpeer_status_e N_PeerGetStatus(netpeer_t *np) {
  return np->status;
}

uint32_t N_PeerGetIPAddress(netpeer_t *np) {
  ENetPeer *enet_peer = N_ComGetENetPeer(&np->com);

  return ENET_NET_TO_HOST_32(enet_peer->address.host);
}

const char* N_PeerGetIPAddressConstString(netpeer_t *np) {
  ENetPeer *enet_peer = N_ComGetENetPeer(&np->com);

  return N_IPToConstString(ENET_NET_TO_HOST_32(enet_peer->address.host));
}

unsigned short N_PeerGetPort(netpeer_t * np) {
  ENetPeer *enet_peer = N_ComGetENetPeer(&np->com);

  return enet_peer->address.port;
}

void N_PeersInit(void) {
  M_IDHashInit(&net_peers, peer_destroy_func);
}

unsigned int N_PeersGetCount(void) {
  return M_IDHashTableGetCount(&net_peers);
}

netpeer_t* N_PeersAdd(void *enet_peer) {
  netpeer_t *np = NULL;

  np = calloc(1, sizeof(netpeer_t));

  if (!np) {
    I_Error("N_PeersAdd: Error allocating memory for new net peer\n");
  }

  np->connect_start_time = time(NULL);

  N_ComInit(&np->com, enet_peer);
  N_SyncInit(&np->sync);

  if (SERVER) {
    enet_peer_throttle_configure(
      enet_peer,
      NET_THROTTLE_INTERVAL,
      NET_THROTTLE_ACCEL,
      NET_THROTTLE_DECEL
    );
    N_SyncSetTIC(&np->sync, gametic);
  }

  np->id = M_IDHashAdd(&net_peers, np);

  return np;
}

netpeer_t* N_PeersLookup(uint32_t id) {
  return M_IDHashLookup(&net_peers, id);
}

netpeer_t* N_PeersLookupByENetPeer(void *enet_peer) {
  GHashTableIter it;
  gpointer key, value;
  ENetPeer *epeer = (ENetPeer *)enet_peer;

  g_hash_table_iter_init(&it, net_peers);

  while (g_hash_table_iter_next(&it, &key, &value)) {
    netpeer_t *np = (netpeer_t *)value;
    ENetPeer *np_epeer = N_ComGetENetPeer(&np->com);

    if (np_epeer->connectID == epeer->connectID) {
      return np;
    }
  }

  return NULL;
}

netpeer_t* N_PeersLookupByPlayer(unsigned int playernum) {
  GHashTableIter it;
  gpointer key, value;

  g_hash_table_iter_init(&it, net_peers);

  while (g_hash_table_iter_next(&it, &key, &value)) {
    netpeer_t *np = (netpeer_t *)value;

    if (np->playernum == playernum) {
      return np;
    }
  }

  return NULL;
}

unsigned int N_PeerGetNumForPlayer(unsigned int playernum) {
  netpeer_t *np = N_PeerForPlayer(playernum);

  if (np) {
    return np->peernum;
  }

  return 0;
}

void N_PeerRemove(netpeer_t *np) {
  g_hash_table_remove(net_peers, GUINT_TO_POINTER(np->peernum));
}

void N_PeerSetConnected(netpeer_t *np) {
  np->connect_start_time = 0;
}

void N_PeerDisconnect(netpeer_t *np, disconnection_reason_e reason) {
  ENetPeer *epeer = N_ComGetENetPeer(&np->com);

  enet_peer_disconnect(epeer, reason);
  np->disconnect_start_time = time(NULL);
}

bool N_PeerIter(netpeer_iter_t **it, netpeer_t **np) {
  gpointer key, value;

  if (!*it) {
    *it = malloc(sizeof(GHashTableIter));
    g_hash_table_iter_init(*it, net_peers);
  }

  if (g_hash_table_iter_next(*it, &key, &value)) {
    *np = (netpeer_t *)value;
    return true;
  }

  free(*it);

  return false;
}

void N_PeerIterRemove(netpeer_iter_t *it, netpeer_t *np) {
  g_hash_table_iter_remove(it);
}

void N_PeerFlushReliableChannel(netpeer_t *np) {
  N_ComFlushChannel(&np->com, NET_CHANNEL_RELIABLE);
}

void N_PeerFlushUnreliableChannel(netpeer_t *np) {
  N_ComFlushChannel(&np->com, NET_CHANNEL_UNRELIABLE);
}

void N_PeerFlushChannels(netpeer_t *np) {
  N_PeerFlushReliableChannel(np);
  N_PeerFlushUnreliableChannel(np);
}

pbuf_t* N_PeerBeginMessage(netpeer_t *np, net_message_e type) {
  return N_ComBeginMessage(&np->com, type);
}

bool N_PeerSetIncoming(netpeer_t *np, unsigned char *data, size_t size) {
  return N_ComSetIncoming(&np->com, data, size);
}

bool N_PeerLoadNextMessage(netpeer_t *np, net_message_e *message_type) {
  return N_ComLoadNextMessage(&np->com, message_type);
}

pbuf_t* N_PeerGetIncomingMessageData(netpeer_t *np) {
  return N_ComGetIncomingMessageData(&np->com);
}

void N_PeerClearReliableChannel(netpeer_t *np) {
  N_ComClearChannel(&np->com, NET_CHANNEL_RELIABLE);
}

void N_PeerClearUnreliableChannel(netpeer_t *np) {
  N_ComClearChannel(&np->com, NET_CHANNEL_UNRELIABLE);
}

void N_PeerSendReset(netpeer_t *np) {
  N_ComSendReset(&np->com);
}

size_t N_PeerGetBytesUploaded(netpeer_t *np) {
  return N_ComGetBytesUploaded(&np->com);
}

size_t N_PeerGetBytesDownloaded(netpeer_t *np) {
  return N_ComGetBytesDownloaded(&np->com);
}

bool N_PeerSyncOutdated(netpeer_t *np) {
  return N_SyncOutdated(&np->sync);
}

void N_PeerSyncSetOutdated(netpeer_t *np) {
  N_SyncSetOutdated(&np->sync);
}

void N_PeerSyncSetNotOutdated(netpeer_t *np) {
  N_SyncSetNotOutdated(&np->sync);
}

bool N_PeerSyncNeedsGameInfo(netpeer_t *np) {
  return N_SyncNeedsGameInfo(&np->sync);
}

void N_PeerSyncSetNeedsGameInfo(netpeer_t *np) {
  N_SyncSetNeedsGameInfo(&np->sync);
}

void N_PeerSyncSetHasGameInfo(netpeer_t *np) {
  N_SyncSetHasGameInfo(&np->sync);
}

bool N_PeerSyncNeedsGameState(netpeer_t *np) {
  return N_SyncNeedsGameState(&np->sync);
}

void N_PeerSyncSetNeedsGameState(netpeer_t *np) {
  N_SyncSetNeedsGameState(&np->sync);
}

void N_PeerSyncSetHasGameState(netpeer_t *np) {
  N_SyncSetHasGameState(&np->sync);
}

bool N_PeerSynchronized(netpeer_t *np) {
  return !(N_PeerSyncNeedsGameInfo(np) || N_PeerSyncNeedsGameState(np));
}

bool N_PeerSyncUpdated(netpeer_t *np) {
  return N_SyncUpdated(&np->sync);
}

void N_PeerSyncSetUpdated(netpeer_t *np) {
  N_SyncSetUpdated(&np->sync);
}

void N_PeerSyncSetNotUpdated(netpeer_t *np) {
  N_SyncSetNotUpdated(&np->sync);
}

bool N_PeerTooLagged(netpeer_t *np) {
  return N_SyncTooLagged(&np->sync);
}

int N_PeerSyncGetTIC(netpeer_t *np) {
  return N_SyncGetTIC(&np->sync);
}

void N_PeerSyncSetTIC(netpeer_t *np, int sync_tic) {
  N_SyncSetTIC(&np->sync, sync_tic);
}

void N_PeerSyncUpdateTIC(netpeer_t *np, int sync_tic) {
  N_SyncUpdateTIC(&np->sync, sync_tic);
}

unsigned int N_PeerSyncGetCommandIndex(netpeer_t *np) {
  return N_SyncGetCommandIndex(&np->sync);
}

void N_PeerSyncSetCommandIndex(netpeer_t *np, unsigned int command_index) {
  N_SyncSetCommandIndex(&np->sync, command_index);
}

void N_PeerSyncUpdateCommandIndex(netpeer_t *np, unsigned int command_index) {
  N_SyncUpdateCommandIndex(&np->sync, command_index);
}

game_state_delta_t* N_PeerSyncGetStateDelta(netpeer_t *np) {
  return N_SyncGetStateDelta(&np->sync);
}

void N_PeerSyncUpdateStateDelta(netpeer_t *np, int from_tic,
                                               int to_tic,
                                               pbuf_t *delta_data) {
  N_SyncUpdateStateDelta(&np->sync, from_tic, to_tic, delta_data);
}

void N_PeerSyncBuildNewStateDelta(netpeer_t *np) {
  N_SyncBuildNewStateDelta(&np->sync);
}

bool N_PeerSyncStateDeltaUpdated(netpeer_t *np) {
  return N_SyncUpdated(&np->sync);
}

void N_PeerSyncResetStateDelta(netpeer_t *np, int sync_tic, int from_tic,
                                                            int to_tic) {
  N_SyncResetStateDelta(&np->sync, sync_tic, from_tic, to_tic);
}

unsigned int N_PeerSyncGetCommandIndexForPlayer(netpeer_t *np,
                                            unsigned int playernum) {
  return N_SyncGetCommandIndexForPlayer(&np->sync, playernum);
}

void N_PeerSyncSetCommandIndexForPlayer(netpeer_t *np,
                                       unsigned int playernum,
                                       unsigned int command_index) {
  N_SyncSetCommandIndexForPlayer(&np->sync, playernum, command_index);
}

void N_PeerSyncUpdateCommandIndexForPlayer(netpeer_t *np,
                                           unsigned int playernum,
                                           unsigned int command_index) {
  N_SyncUpdateCommandIndexForPlayer(&np->sync, playernum, command_index);
}

void N_PeerSyncReset(netpeer_t *np) {
  N_SyncReset(&np->sync);
}

void* N_PeerGetENetPeer(netpeer_t *np) {
  return N_ComGetENetPeer(&np->com);
}

float N_PeerGetPacketLoss(netpeer_t *np) {
  ENetPeer *epeer = N_PeerGetENetPeer(np);

  return epeer->packetLoss / (float)ENET_PEER_PACKET_LOSS_SCALE;
}

float N_PeerGetPacketLossJitter(netpeer_t *np) {
  ENetPeer *epeer = N_PeerGetENetPeer(np);

  return epeer->packetLossVariance / (float)ENET_PEER_PACKET_LOSS_SCALE;
}

/* vi: set et ts=2 sw=2: */
