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


#ifndef R_FPS_H__
#define R_FPS_H__

#include "doomstat.h"

extern int movement_smooth_default;
extern int movement_smooth;

extern int interpolation_maxobjects;

typedef struct {
  unsigned int start;
  unsigned int next;
  unsigned int step;
  fixed_t frac;
  float msec;
} tic_vars_t;

extern tic_vars_t tic_vars;

void M_ChangeUncappedFrameRate(void);

void R_InitInterpolation(void);
void R_InterpolateView(player_t *player);

extern dboolean WasRenderedInTryRunTics;

void R_ResetViewInterpolation ();
void R_UpdateInterpolations();
void R_StopAllInterpolations(void);
void R_RestoreInterpolations();
void R_ActivateSectorInterpolations();
void R_ActivateThinkerInterpolations(thinker_t *th);
void R_StopInterpolationIfNeeded(thinker_t *th);

#endif

/* vi: set et ts=2 sw=2: */

