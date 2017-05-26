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

int   maxViewPitch;
int   minViewPitch;
float viewPitch;

void P_ChangeMaxViewPitch(void) {
  int max_up;
  int max_dn;
  int angle_up;
  int angle_dn;
  
  if (V_GetMode() == VID_MODEGL) {
    max_up = movement_maxviewpitch;
    max_dn = movement_maxviewpitch;
  }
  else {
    max_up = MIN(movement_maxviewpitch, 61);
    max_dn = MIN(movement_maxviewpitch, 36);
  }

  angle_up = (int)((float)max_up / 45.0f * ANG45);
  angle_dn = (int)((float)max_dn / 45.0f * ANG45);

  maxViewPitch = (+angle_up - (1 << ANGLETOFINESHIFT));
  minViewPitch = (-angle_dn + (1 << ANGLETOFINESHIFT));

  viewpitch = 0;
}

void P_CheckPitch(signed int *pitch) {
  if (*pitch > maxViewPitch) {
    *pitch = maxViewPitch;
  }

  if (*pitch < minViewPitch) {
    *pitch = minViewPitch;
  }

  (*pitch) >>= 16;
  (*pitch) <<= 16;
}

/* vi: set et ts=2 sw=2: */
