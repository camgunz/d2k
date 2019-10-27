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


#ifndef N_SYNC_H__
#define N_SYNC_H__

struct player_s;
typedef struct player_s player_t;

struct game_state_delta_s;
typedef struct game_state_delta_s game_state_delta_t;

typedef enum {
  SYNC_SYNCHRONIZED     = 0,
  SYNC_NEEDS_GAME_INFO  = 1,
  SYNC_NEEDS_GAME_STATE = 1 << 1,
  SYNC_OUTDATED         = 1 << 2,
  SYNC_UPDATED          = 1 << 3,
} net_sync_state_e;

typedef struct netsync_s {
  unsigned int        state;
  int                 tic;
  uint32_t            command_index;
  GHashTable         *command_indices;
  game_state_delta_t  delta;
} netsync_t;

void N_SyncInit(netsync_t *ns);
void N_SyncFree(netsync_t *ns);
void N_SyncReset(netsync_t *ns);

bool N_SyncNeedsGameInfo(netsync_t *ns);
void N_SyncSetNeedsGameInfo(netsync_t *ns);
void N_SyncSetHasGameInfo(netsync_t *ns);

bool N_SyncNeedsGameState(netsync_t *ns);
void N_SyncSetNeedsGameState(netsync_t *ns);
void N_SyncSetHasGameState(netsync_t *ns);

bool N_SyncOutdated(netsync_t *ns);
void N_SyncSetOutdated(netsync_t *ns);
void N_SyncSetNotOutdated(netsync_t *ns);

bool N_SyncUpdated(netsync_t *ns);
void N_SyncSetUpdated(netsync_t *ns);
void N_SyncSetNotUpdated(netsync_t *ns);

bool     N_SyncTooLagged(netsync_t *ns);

int      N_SyncGetTIC(netsync_t *ns);
void     N_SyncSetTIC(netsync_t *ns, int tic);
void     N_SyncUpdateTIC(netsync_t *ns, int tic);

uint32_t N_SyncGetCommandIndex(netsync_t *ns);
void     N_SyncSetCommandIndex(netsync_t *ns, uint32_t command_index);
void     N_SyncUpdateCommandIndex(netsync_t *ns, uint32_t command_index);

game_state_delta_t* N_SyncGetStateDelta(netsync_t *ns);
bool                N_SyncUpdateStateDelta(netsync_t *ns, int from_tic,
                                                          int to_tic,
                                                          pbuf_t *delta_data);
void                N_SyncBuildNewStateDelta(netsync_t *ns);
void                N_SyncResetStateDelta(netsync_t *ns, int sync_tic,
                                                         int from_tic,
                                                         int to_tic);

uint32_t N_SyncGetCommandIndexForPlayer(netsync_t *ns, player_t *player);
void     N_SyncSetCommandIndexForPlayer(netsync_t *ns, player_t *player,
                                                       uint32_t command_index);
void     N_SyncUpdateCommandIndexForPlayer(netsync_t *ns,
                                           player_t *player,
                                           uint32_t command_index);

#endif

/* vi: set et ts=2 sw=2: */
