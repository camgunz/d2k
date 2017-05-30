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

#include "d_mouse.h"
#include "e6y.h"
#include "g_game.h"
#include "m_misc.h"
#include "mn_main.h"
#include "p_camera.h"
#include "pl_main.h"
#include "r_sky.h"
#include "v_video.h"
#include "gl_opengl.h"
#include "gl_struct.h"

int     mouse_sensitivity_horiz; // has default   //  killough
int     mouse_sensitivity_vert;  // has default
int     mouse_sensitivity_mlook;
int     mouse_acceleration;
int     mouse_handler;
int     mouse_doubleclick_as_use;
float   mouse_accelfactor;
int     movement_mouselook;
int     movement_mouseinvert;
int     movement_maxviewpitch;
bool    mlook_or_fov;
int     mlooky = 0;
angle_t viewpitch;

bool D_MouseShouldBeGrabbed(void) {
  if (walkcamera.mode != camera_mode_disabled) {
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

void D_MouseChangeMouseLook(bool increase) {
  D_Mouse(increase, &mouse_sensitivity_mlook);
}

void D_MouseChangeAccel(bool increase) {
  D_Mouse(increase, &mouse_acceleration);
  D_MouseScaleAccel();
}

void D_MouseScaleAccel(void) {
  mouse_accelfactor = (float)mouse_acceleration/100.0f+1.0f;
}

void D_MouseChangeHoriz(bool increase) {
  D_Mouse(increase, &mouse_sensitivity_horiz);
}

void D_MouseChangeVert(bool increase) {
  D_Mouse(increase, &mouse_sensitivity_vert);
}

void D_Mouse(bool increase, int *value) {
  if (increase) {
    if (*value < 99) {
      *value = (*value) + 1;
    }
  }
  else {
    if (*value > 0) {
      *value  = (*value) - 1;
    }
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

void D_ChangeMouseLook(void) {
  viewpitch = 0;

  R_InitSkyMap();

#ifdef GL_DOOM
  if (gl_skymode == skytype_auto) {
    if (movement_mouselook) {
      gl_drawskys = skytype_skydome;
    }
    else {
      gl_drawskys = skytype_standard;
    }
  }
  else {
    gl_drawskys = gl_skymode;
  }
#endif
}

bool D_GetMouseLook(void) {
  return movement_mouselook;
}

bool D_HaveMouseLook(void) {
  return (viewpitch != 0);
}

void MN_MouseChangeHoriz(int choice) {
  D_MouseChangeHoriz((bool)choice);
}

void MN_MouseChangeVert(int choice) {
  D_MouseChangeVert((bool)choice);
}

void MN_MouseChangeMouseLook(int choice) {
  D_MouseChangeMouseLook((bool)choice);
}

void MN_MouseChangeAccel(int choice) {
  D_MouseChangeAccel((bool)choice);
}

/* vi: set et ts=2 sw=2: */
