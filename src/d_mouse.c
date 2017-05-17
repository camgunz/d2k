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
#include "e6y.h"
#include "g_game.h"
#include "m_misc.h"
#include "d_mouse.h"

int mouse_sensitivity_horiz; // has default   //  killough
int mouse_sensitivity_vert;  // has default
int mouse_sensitivity_mlook;
int mouse_acceleration;
float mouse_accelfactor;

bool D_MouseShouldBeGrabbed(void) {
  if (walkcamera.type) {
    return (demoplayback && gamestate == GS_LEVEL && !menuactive);
  }

  if (menuactive || paused) { /* [CG] [TODO] Add consoleactive here */
    return false;
  }

  if (demoplayback) {
    return false;
  }

  if (gamestate != GS_LEVEL) {
    return false;
  }

  return true;
}

void D_MouseMLook(int choice) {
  D_Mouse(choice, &mouse_sensitivity_mlook);
}

void D_MouseAccel(int choice) {
  D_Mouse(choice, &mouse_acceleration);
  D_MouseScaleAccel();
}

void D_MouseScaleAccel(void) {
  mouse_accelfactor = (float)mouse_acceleration/100.0f+1.0f;
}

void D_MouseHoriz(int choice) {
  D_Mouse(choice, &mouse_sensitivity_horiz);
}

void D_MouseVert(int choice) {
  D_Mouse(choice, &mouse_sensitivity_vert);
}

void D_Mouse(int choice, int *sens) {
  switch(choice) {
    case 0:
      if (*sens)
        --*sens;
      break;
    case 1:
      if (*sens < 99) {
        ++*sens;              /*mead*/
      }
      break;
  }
}

int D_MouseAccelerate(int val) {
  if (!mouse_acceleration) {
    return val;
  }

  if (val < 0) {
    return -D_MouseAccelerate(-val);
  }

  return M_DoubleToInt(pow((double)val, (double)mouse_accelfactor));
}

/* vi: set et ts=2 sw=2: */
