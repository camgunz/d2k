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

#include "doomdef.h"
#include "g_game.h"
#include "g_state.h"
#include "n_main.h"
#include "n_sync.h"
#include "p_user.h"

#define SERVER_MAX_PEER_LAG (TICRATE * 4)

void N_SyncInit(netsync_t *ns) {
  ns->state = SYNC_NEEDS_GAME_INFO | SYNC_NEEDS_GAME_STATE;
  ns->tic = 0;
  ns->command_index = 0;
  memset(ns->command_indices, 0, MAXPLAYERS * sizeof(unsigned int));
  G_DeltaInit(&ns->delta);
}

void N_SyncReset(netsync_t *ns) {
  ns->state = SYNC_NEEDS_GAME_INFO | SYNC_NEEDS_GAME_STATE;
  ns->tic = 0;
  ns->command_index = 0;
  memset(ns->command_indices, 0, MAXPLAYERS * sizeof(unsigned int));
  G_DeltaClear(&ns->delta);
}

void N_SyncFree(netsync_t *ns) {
  ns->state = SYNC_NEEDS_GAME_INFO | SYNC_NEEDS_GAME_STATE;
  ns->tic = 0;
  ns->command_index = 0;
  memset(ns->command_indices, 0, MAXPLAYERS * sizeof(unsigned int));
  G_DeltaFree(&ns->delta);
}

bool N_SyncNeedsGameInfo(netsync_t *ns) {
  return (ns->state & SYNC_NEEDS_GAME_INFO) == SYNC_NEEDS_GAME_INFO;
}

void N_SyncSetNeedsGameInfo(netsync_t *ns) {
  ns->state |= SYNC_NEEDS_GAME_INFO;
}

void N_SyncSetHasGameInfo(netsync_t *ns) {
  ns->state &= ~SYNC_NEEDS_GAME_INFO;
}

bool N_SyncNeedsGameState(netsync_t *ns) {
  return (ns->state & SYNC_NEEDS_GAME_STATE) == SYNC_NEEDS_GAME_STATE;
}

void N_SyncSetNeedsGameState(netsync_t *ns) {
  ns->state |= SYNC_NEEDS_GAME_STATE;
}

void N_SyncSetHasGameState(netsync_t *ns) {
  ns->state &= ~SYNC_NEEDS_GAME_STATE;
}

bool N_SyncOutdated(netsync_t *ns) {
  return (ns->state & SYNC_OUTDATED) == SYNC_OUTDATED;
}

void N_SyncSetOutdated(netsync_t *ns) {
  ns->state |= SYNC_OUTDATED;
}

void N_SyncSetNotOutdated(netsync_t *ns) {
  ns->state &= ~SYNC_OUTDATED;
}

bool N_SyncUpdated(netsync_t *ns) {
  return (ns->state & SYNC_UPDATED) == SYNC_UPDATED;
}

void N_SyncSetUpdated(netsync_t *ns) {
  ns->state |= SYNC_UPDATED;
}

void N_SyncSetNotUpdated(netsync_t *ns) {
  ns->state &= ~SYNC_UPDATED;
}

bool N_SyncTooLagged(netsync_t *ns) {
  return (gametic - ns->tic) > SERVER_MAX_PEER_LAG;
}

int N_SyncGetTIC(netsync_t *ns) {
  return ns->tic;
}

void N_SyncSetTIC(netsync_t *ns, int tic) {
  ns->tic = tic;
}

void N_SyncUpdateTIC(netsync_t *ns, int tic) {
  if (tic <= ns->tic) {
    return;
  }

  ns->tic = tic;
}

unsigned int N_SyncGetCommandIndex(netsync_t *ns) {
  return ns->command_index;
}

void N_SyncSetCommandIndex(netsync_t *ns, unsigned int command_index) {
  ns->command_index = command_index;
}

void N_SyncUpdateCommandIndex(netsync_t *ns, unsigned int command_index) {
  if (command_index <= ns->command_index) {
    return;
  }

  ns->command_index = command_index;
}

bool N_SyncUpdateStateDelta(netsync_t *ns, int from_tic, int to_tic,
                                                         pbuf_t *delta_data) {
  if (to_tic <= ns->tic) {
    return true;
  }

  M_BufferClear(&ns->delta.data);
  if (!M_PBufReadBytes(delta_data, &ns->delta.data)) {
    P_Printf(consoleplayer,
      "N_SyncUpdateStateDelta: Error reading delta data: %s.\n",
      M_PBufGetError(delta_data)
    );
    return false;
  }

  ns->tic = to_tic;
  ns->delta.from_tic = from_tic;
  ns->delta.to_tic = to_tic;

  N_SyncSetUpdated(ns);

  return true;
}

void N_SyncBuildNewStateDelta(netsync_t *ns) {
  if (ns->tic >= G_GetLatestState()->tic) {
    return;
  }

  G_BuildStateDelta(ns->tic, &ns->delta);
}

void N_SyncResetStateDelta(netsync_t *ns, int sync_tic, int from_tic,
                                                        int to_tic) {
  ns->tic = sync_tic;
  ns->delta.from_tic = from_tic;
  ns->delta.to_tic = to_tic;
}

unsigned int N_SyncGetCommandIndexForPlayer(netsync_t *ns,
                                            unsigned int playernum) {
  return ns->command_indices[playernum];
}

void N_SyncSetCommandIndexForPlayer(netsync_t *ns,
                                    unsigned int playernum,
                                    unsigned int command_index) {
  ns->command_indices[playernum] = command_index;
}

void N_SyncUpdateCommandIndexForPlayer(netsync_t *ns,
                                       unsigned int playernum,
                                       unsigned int command_index) {
  if (command_index <= ns->command_indices[playernum]) {
    return;
  }

  ns->command_indices[playernum] = command_index;
}

game_state_delta_t* N_SyncGetStateDelta(netsync_t *ns) {
  return &ns->delta;
}

/* vi: set et ts=2 sw=2: */
