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


#ifndef N_STATE_H__
#define N_STATE_H__

typedef struct game_state_s {
  int     tic;
  pbuf_t *data;
} game_state_t;

typedef struct game_state_delta_s {
  int   from_tic;
  int   to_tic;
  buf_t data;
} game_state_delta_t;

void          N_InitStates(void);
void          N_SaveState(void);
bool          N_LoadState(int tic, bool call_init_new);
void          N_RemoveOldStates(int tic);
void          N_ClearStates(void);
game_state_t* N_ReadNewStateFromPackedBuffer(int tic, pbuf_t *pbuf);
game_state_t* N_GetLatestState(void);
void          N_SetLatestState(game_state_t *state);
bool          N_LoadLatestState(bool call_init_new);
bool          N_ApplyStateDelta(game_state_delta_t *delta);
void          N_BuildStateDelta(int tic, game_state_delta_t *delta);
int           N_GetStateFromTic(void);

#endif

/* vi: set et ts=2 sw=2: */

