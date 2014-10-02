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


#define DEBUG_STATES 0

#include "z_zone.h"

#include <enet/enet.h>

#include "doomstat.h"
#include "m_avg.h"
#include "m_delta.h"

#include "g_game.h"
#include "g_save.h"
#include "lprintf.h"
#include "n_net.h"
#include "n_state.h"
#include "n_peer.h"

static game_state_t *latest_game_state = NULL;
static GHashTable *saved_game_states = NULL;
static GQueue *state_data_buffer_queue = NULL;
static avg_t average_state_size;

static void clear_state_buffer(gpointer gp) {
  pbuf_t *pbuf = gp;

  M_PBufClear(pbuf);
  free(pbuf);
}

static void clear_game_state_data(gpointer gp) {
  game_state_t *gs = (game_state_t *)gp;

  M_PBufClear(gs->data);
  g_queue_push_tail(state_data_buffer_queue, gs->data);
  gs->data = NULL;

  free(gs);
}

static gboolean state_is_old(gpointer key, gpointer value, gpointer user_data) {
  int tic = GPOINTER_TO_INT(user_data);
  game_state_t *gs = (game_state_t *)value;

  if (gs->tic < tic) {
    D_Log(LOG_STATE, "Removing state %d (< %d)\n", gs->tic, tic);
    return true;
  }

  return false;
}

game_state_t* get_new_state(int tic) {
  game_state_t *new_gs = malloc(sizeof(game_state_t));

  if (new_gs == NULL)
    I_Error("get_new_state: Error allocating new game state");

  if (g_queue_is_empty(state_data_buffer_queue))
    new_gs->data = M_PBufNewWithCapacity(MAX(average_state_size.value, 16384));
  else
    new_gs->data = g_queue_pop_head(state_data_buffer_queue);

  new_gs->tic = tic;

  return new_gs;
}

void N_InitStates(void) {
#if DEBUG_STATES
  if (SERVER)
    D_EnableLogChannel(LOG_STATE, "server-states.log");
#endif
  M_AverageInit(&average_state_size);

  if (saved_game_states != NULL)
    g_hash_table_destroy(saved_game_states);

  saved_game_states = g_hash_table_new_full(
    g_direct_hash, g_direct_equal, NULL, clear_game_state_data
  );

  if (state_data_buffer_queue != NULL)
    g_queue_free_full(state_data_buffer_queue, clear_state_buffer);

  state_data_buffer_queue = g_queue_new();
}

void N_SaveState(void) {
  game_state_t *gs = latest_game_state = get_new_state(gametic);

  M_PBufClear(gs->data);
  G_WriteSaveData(gs->data);

  g_hash_table_insert(saved_game_states, GINT_TO_POINTER(gs->tic), gs);

  M_AverageUpdate(
    &average_state_size, M_PBufGetCapacity(gs->data)
  );

  if (SERVER && playeringame[1]) {
    N_LogPlayerPosition(&players[1]);
    // D_Log(LOG_SYNC, "(%d) Saved state %d\n\n", gametic, gs->tic);
  }
}

bool N_LoadState(int tic, bool call_init_new) {
  game_state_t *gs = g_hash_table_lookup(
    saved_game_states, GINT_TO_POINTER(tic)
  );

  if (gs != NULL) {
    M_PBufSeek(gs->data, 0);
    G_ReadSaveData(gs->data, true, call_init_new);
    return true;
  }

  return false;
}

void N_RemoveOldStates(int tic) {
  g_hash_table_foreach_remove(
    saved_game_states, state_is_old, GINT_TO_POINTER(tic)
  );
}

void N_ClearStates(void) {
  g_hash_table_remove_all(saved_game_states);
}

game_state_t* N_ReadNewStateFromPackedBuffer(int tic, pbuf_t *pbuf) {
  game_state_t *gs = get_new_state(tic);

  if (!M_PBufReadBytes(pbuf, M_PBufGetBuffer(gs->data))) {
    M_PBufClear(gs->data);
    free(gs);
    return NULL;
  }

  g_hash_table_insert(saved_game_states, GINT_TO_POINTER(gs->tic), gs);

  return gs;
}

game_state_t* N_GetLatestState(void) {
  return latest_game_state;
}

void N_SetLatestState(game_state_t *state) {
  latest_game_state = state;
}

bool N_LoadLatestState(bool call_init_new) {
  M_PBufSeek(latest_game_state->data, 0);

  return G_ReadSaveData(latest_game_state->data, true, call_init_new);
}

bool N_ApplyStateDelta(game_state_delta_t *delta) {
  game_state_t *gs = g_hash_table_lookup(
    saved_game_states, GINT_TO_POINTER(delta->from_tic)
  );
  game_state_t *new_gs = NULL;

  if (gs == NULL)
    return false;

  new_gs = get_new_state(delta->to_tic);

  M_PBufSeek(gs->data, 0);
  M_PBufSeek(new_gs->data, 0);
  M_BufferSeek(&delta->data, 0);

  if (!M_ApplyDelta(gs->data, new_gs->data, &delta->data))
    return false;

  g_hash_table_insert(saved_game_states, GINT_TO_POINTER(new_gs->tic), new_gs);
  N_SetLatestState(new_gs);

  return true;
}

void N_BuildStateDelta(int tic, game_state_delta_t *delta) {
  game_state_t *gs = g_hash_table_lookup(
    saved_game_states, GINT_TO_POINTER(tic)
  );

  if (gs == NULL)
    I_Error("N_BuildStateDelta: Missing game state %d.\n", tic);

  M_BufferClear(&delta->data);
  M_BuildDelta(gs->data, latest_game_state->data, &delta->data);

  delta->from_tic = tic;
  delta->to_tic = latest_game_state->tic;
}

/* vi: set et ts=2 sw=2: */

