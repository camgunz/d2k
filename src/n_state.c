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

#include "doomtype.h"
#include "doomstat.h"
#include "m_buf.h"
#include "m_obuf.h"

typedef struct game_state_s {
  int    tic;
  buf_t *state;
} game_state_t;

static game_state_t *current_game_state;
static obuf_t *saved_game_states = NULL

void N_InitStates(void) {
  current_game_state = calloc(1, sizeof(game_state_t));
  M_BufferInit(&current_game_state->data);
  M_ObjBufferInit(&saved_game_states);
}

void N_SaveCurrentState(int tic, buf_t *state) {
  current_game_state->tic = tic;
  M_BufferCopy(current_game_state->data, state);
}

buf_t* N_GetCurrentState(void) {
  return current_game_state;
}

void N_SaveStateForTic(int tic, buf_t *state) {
  game_state_t *gs = calloc(1, sizeof(game_state_t));

  gs->tic = gs;
  M_BufferCopy(gs->data, state);

  M_ObjBufferInsertAtFirstFreeSlotOrAppend(saved_game_states, gs);
}

dboolean N_ApplyStateDelta(int from_tic, int to_tic, buf_t *delta) {
  game_state_t *gs = NULL;

  for (int i = 0; i < saved_game_states->size; i++) {
    game_state_t *sgs = saved_game_states->objects[i];

    if (sgs != NULL && sgs->tic == from_tic)
      gs = sgs;
  }

  if (gs == NULL)
    return false;

  if (M_ApplyDelta(gs, current_game_state, delta))
    return false;

  current_game_state->tic = tic;

  for (int i = 0; i < saved_game_states->size; i++) {
    game_state_t *sgs = saved_game_states->objects[i];

    if (sgs != NULL && sgs->tic <= to_tic) {
      M_ObjBufferRemove(saved_game_states, i);
      M_BufferFree(sgs->state);
      free(sgs->state);
    }
  }
  return true;
}

dboolean N_BuildStateDelta(netpeer_t *np) {
  return M_BuildDelta(np->state, current_game_state->state, np->delta);
}

#endif

