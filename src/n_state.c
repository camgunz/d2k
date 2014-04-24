/* Emacs style mode select   -*- C++ -*-
 *-----------------------------------------------------------------------------
 *
 *
 *  PrBoom: a Doom port merged with LxDoom and LSDLDoom
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 1999 by
 *  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
 *  Copyright (C) 1999-2000 by
 *  Jess Haas, Nicolas Kalkhof, Colin Phipps, Florian Schulze
 *  Copyright 2005, 2006 by
 *  Florian Schulze, Colin Phipps, Neil Stevens, Andrey Budko
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 *  02111-1307, USA.
 *
 * DESCRIPTION:
 *
 *
 *-----------------------------------------------------------------------------
 */

#include "z_zone.h"

#include <enet/enet.h>
#include <msgpack.h>

#include "doomstat.h"
#include "g_game.h"
#include "lprintf.h"
#include "m_delta.h"
#include "m_pbuf.h"

#include "n_net.h"
#include "n_buf.h"
#include "n_state.h"
#include "n_peer.h"

static game_state_t *latest_game_state = NULL;
static cbuf_t saved_game_states;
static size_t average_state_size = 0;
static size_t state_count = 0;

static game_state_t* get_state(int tic) {
  CBUF_FOR_EACH(&saved_game_states, entry) {
    game_state_t *gs = (game_state_t *)entry.obj;

    if (gs->tic == tic)
      return gs;
  }

  return NULL;
}

void N_InitStates(void) {
  M_CBufInit(&saved_game_states, sizeof(game_state_t));
}

void N_SaveState(void) {
  M_CBufConsolidate(&saved_game_states);
  latest_game_state = N_GetNewState();

  latest_game_state->tic = gametic;
  M_BufferClear(&latest_game_state->data);
  G_WriteSaveData(&latest_game_state->data);

  average_state_size = (
    ((average_state_size * state_count) + latest_game_state->data.capacity) /
    ++state_count
  );
}

dboolean N_LoadState(int tic, dboolean call_init_new) {
  game_state_t *gs = get_state(tic);

  if (gs != NULL) {
    M_BufferSeek(&gs->data, 0);
    G_ReadSaveData(&gs->data, true, call_init_new);
    return true;
  }

  return false;
}

void N_RemoveOldStates(int tic) {
  CBUF_FOR_EACH(&saved_game_states, entry) {
    game_state_t *gs = (game_state_t *)entry.obj;

    if (gs->tic < tic) {
      M_BufferFree(&gs->data);
      M_CBufRemove(&saved_game_states, entry.index);
      entry.index--;
    }
  }
}

void N_ClearStates(void) {
  CBUF_FOR_EACH(&saved_game_states, entry) {
    game_state_t *gs = (game_state_t *)entry.obj;

    M_BufferFree(&gs->data);
    M_CBufRemove(&saved_game_states, entry.index);
    entry.index--;
  }
}

game_state_t* N_GetNewState(void) {
  game_state_t *new_gs = NULL;
  
  M_CBufConsolidate(&saved_game_states);

  new_gs = M_CBufGetFirstFreeSlot(&saved_game_states);

  if (new_gs == NULL) {
    new_gs = M_CBufGetNewSlot(&saved_game_states);
    M_BufferInit(&new_gs->data);
  }

  M_BufferEnsureTotalCapacity(&new_gs->data, MAX(average_state_size, 16384));

  return new_gs;
}

game_state_t* N_GetLatestState(void) {
  return latest_game_state;
}

void N_SetLatestState(game_state_t *state) {
  latest_game_state = state;
}

void N_LoadLatestState(dboolean call_init_new) {
  M_BufferSeek(&latest_game_state->data, 0);
  G_ReadSaveData(&latest_game_state->data, true, call_init_new);
}

dboolean N_ApplyStateDelta(game_state_delta_t *delta) {
  game_state_t *gs = get_state(delta->from_tic);
  game_state_t *new_gs = NULL;

  if (gs == NULL)
    return false;

  new_gs = N_GetNewState();

  M_BufferSeek(&gs->data, 0);
  M_BufferSeek(&new_gs->data, 0);
  M_BufferSeek(&delta->data, 0);

  M_ApplyDelta(&gs->data, &new_gs->data, &delta->data);

  new_gs->tic = delta->to_tic;

  N_SetLatestState(new_gs);

  return true;
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

/* vi: set et sw=2 ts=2: */

