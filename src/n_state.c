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
static cbuf_t saved_game_states;
static avg_t average_state_size;

static game_state_t* get_state(int tic) {
  CBUF_FOR_EACH(&saved_game_states, entry) {
    game_state_t *gs = (game_state_t *)entry.obj;

    if (gs->tic == tic)
      return gs;
  }

  return NULL;
}

void N_InitStates(void) {
#if DEBUG_STATES
  if (SERVER)
    D_EnableLogChannel(LOG_STATE, "server-states.log");
#endif
  M_AverageInit(&average_state_size);
  M_CBufInit(&saved_game_states, sizeof(game_state_t));
}

void N_SaveState(void) {
  M_CBufConsolidate(&saved_game_states);
  latest_game_state = N_GetNewState();

  latest_game_state->tic = gametic;
  M_PBufClear(&latest_game_state->data);
  G_WriteSaveData(&latest_game_state->data);

  M_AverageUpdate(
    &average_state_size, M_PBufGetCapacity(&latest_game_state->data)
  );
}

dboolean N_LoadState(int tic, dboolean call_init_new) {
  game_state_t *gs = get_state(tic);

  if (gs != NULL) {
    M_PBufSeek(&gs->data, 0);
    G_ReadSaveData(&gs->data, true, call_init_new);
    return true;
  }

  return false;
}

void N_RemoveOldStates(int tic) {
  CBUF_FOR_EACH(&saved_game_states, entry) {
    game_state_t *gs = (game_state_t *)entry.obj;

    if (gs->tic < tic) {
      D_Log(LOG_STATE, "Removing state %d\n", gs->tic);
      M_PBufFree(&gs->data);
      M_CBufRemove(&saved_game_states, entry.index);
      entry.index--;
    }
  }
}

void N_ClearStates(void) {
  CBUF_FOR_EACH(&saved_game_states, entry) {
    game_state_t *gs = (game_state_t *)entry.obj;

    M_PBufFree(&gs->data);
    M_CBufRemove(&saved_game_states, entry.index);
    entry.index--;
  }
}

game_state_t* N_GetNewState(void) {
  game_state_t *new_gs = NULL;
  
  M_CBufConsolidate(&saved_game_states);

  new_gs = M_CBufGetFirstFreeSlot(&saved_game_states);

  if (new_gs == NULL)
    new_gs = M_CBufGetNewSlot(&saved_game_states);

  M_PBufInitWithCapacity(
    &new_gs->data, MAX(average_state_size.value, 16384)
  );

  return new_gs;
}

game_state_t* N_GetLatestState(void) {
  return latest_game_state;
}

void N_SetLatestState(game_state_t *state) {
  latest_game_state = state;
}

dboolean N_LoadLatestState(dboolean call_init_new) {
  M_PBufSeek(&latest_game_state->data, 0);
  return G_ReadSaveData(&latest_game_state->data, true, call_init_new);
}

dboolean N_ApplyStateDelta(game_state_delta_t *delta) {
  game_state_t *gs = get_state(delta->from_tic);
  game_state_t *new_gs = NULL;

  if (gs == NULL)
    return false;

  new_gs = N_GetNewState();

  M_PBufSeek(&gs->data, 0);
  M_PBufSeek(&new_gs->data, 0);
  M_BufferSeek(&delta->data, 0);

  if (M_ApplyDelta(&gs->data, &new_gs->data, &delta->data)) {
    new_gs->tic = delta->to_tic;
    N_SetLatestState(new_gs);
    return true;
  }

  CBUF_FOR_EACH(&saved_game_states, entry) {
    game_state_t *egs = entry.obj;

    if (egs == new_gs) {
      M_CBufRemove(&saved_game_states, entry.index);
      break;
    }
  }

  return false;
}

void N_BuildStateDelta(int tic, game_state_delta_t *delta) {
  game_state_t *state = get_state(tic);

  if (state == NULL)
    I_Error("N_BuildStateDelta: Missing game state %d.\n", tic);

  M_BufferClear(&delta->data);
  M_BuildDelta(&state->data, &latest_game_state->data, &delta->data);

  delta->from_tic = tic;
  delta->to_tic = latest_game_state->tic;
}

/* vi: set et ts=2 sw=2: */

