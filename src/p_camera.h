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


#ifndef P_CAMERA_H__
#define P_CAMERA_H__

typedef enum {
  camera_mode_disabled,
  camera_mode_player,
  camera_mode_free
} camera_mode_e;

typedef struct camera_s {
  camera_mode_e mode;
  int x;
  int y;
  int z;
  int PrevX;
  int PrevY;
  int PrevZ;
  angle_t angle;
  angle_t pitch;
  angle_t PrevAngle;
  angle_t PrevPitch;
} camera_t;

extern camera_t walkcamera;

void P_WalkTicker(void);
void P_ResetWalkcam(void);
void P_SyncWalkcam(bool sync_coords, bool sync_sight);

#endif

/* vi: set et ts=2 sw=2: */
