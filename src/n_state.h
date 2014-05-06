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

#ifndef N_STATE_H__
#define N_STATE_H__

typedef struct game_state_s {
  int   tic;
  buf_t data;
} game_state_t;

typedef struct game_state_delta_s {
  int   from_tic;
  int   to_tic;
  buf_t data;
} game_state_delta_t;

void          N_InitStates(void);
void          N_SaveState(void);
dboolean      N_LoadState(int tic, dboolean call_init_new);
void          N_RemoveOldStates(int tic);
void          N_ClearStates(void);
game_state_t* N_GetNewState(void);
game_state_t* N_GetLatestState(void);
void          N_SetLatestState(game_state_t *state);
dboolean      N_LoadLatestState(dboolean call_init_new);
dboolean      N_ApplyStateDelta(game_state_delta_t *delta);
void          N_BuildStateDelta(int tic, game_state_delta_t *delta);

#endif

/* vi: set et sw=2 ts=2: */

