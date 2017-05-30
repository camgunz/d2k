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

#include "i_video.h"
#include "m_random.h"
#include "f_wipe.h"
#include "e6y.h"
#include "r_defs.h"
#include "v_video.h"

#include "gl_opengl.h"
#include "gl_struct.h"

#include "n_main.h"

//
// SCREEN WIPE PACKAGE
//

// Parts re-written to support true-color video modes. Column-major
// formatting removed. - POPE

// CPhipps - macros for the source and destination screens
#define SRC_SCR 2
#define DEST_SCR 3

typedef struct {
  bool         active;
  screeninfo_t wipe_scr_start;
  screeninfo_t wipe_scr_end;
  screeninfo_t wipe_scr;
  int *y_lookup;
} screen_wipe_t;

static screen_wipe_t sw;

// e6y: resolution limitation is removed
void R_InitMeltRes(void) {
  sw.y_lookup = calloc(1, SCREENWIDTH * sizeof(*sw.y_lookup));
}

static int wipe_initMelt(void) {
  if (V_GetMode() != VID_MODEGL) {
    // copy start screen to main screen
    for (int i = 0; i < SCREENHEIGHT; i++) {
      memcpy(
        sw.wipe_scr.data + i * sw.wipe_scr.byte_pitch,
        sw.wipe_scr_start.data + i * sw.wipe_scr_start.byte_pitch,
         SCREENWIDTH * V_GetPixelDepth()
      );
    }
  }

  // setup initial column positions (y<0 => not ready to scroll yet)
  if (MULTINET)
    sw.y_lookup[0] = -(D_RandomRange(0, 16));
  else
    sw.y_lookup[0] = -(M_Random() % 16);

  for (int i = 1; i < SCREENWIDTH; i++) {
    int r;

    if (MULTINET)
      r = D_RandomRange(-1, 2);
    else
      r = (M_Random() % 3) - 1;

    sw.y_lookup[i] = sw.y_lookup[i - 1] + r;

    if (sw.y_lookup[i] > 0)
      sw.y_lookup[i] = 0;
    else if (sw.y_lookup[i] == -16)
      sw.y_lookup[i] = -15;
  }

  return 0;
}

static int doMelt(void) {
  bool done = true;
  int i;
  const int depth = V_GetPixelDepth();

  for (i=0;i<(SCREENWIDTH);i++) {
    if (sw.y_lookup[i]<0) {
      sw.y_lookup[i]++;
      done = false;
      continue;
    }
    if (sw.y_lookup[i] < SCREENHEIGHT) {
      unsigned char *s, *d;
      int j, k, dy;

      /* cph 2001/07/29 -
        *  The original melt rate was 8 pixels/sec, i.e. 25 frames to melt
        *  the whole screen, so make the melt rate depend on SCREENHEIGHT
        *  so it takes no longer in high res
        */
      dy = (sw.y_lookup[i] < 16) ? sw.y_lookup[i]+1 : SCREENHEIGHT/25;
      if (sw.y_lookup[i]+dy >= SCREENHEIGHT)
        dy = SCREENHEIGHT - sw.y_lookup[i];

     if (V_GetMode() != VID_MODEGL) {
      s = sw.wipe_scr_end.data    + (sw.y_lookup[i]*sw.wipe_scr_end.byte_pitch+(i*depth));
      d = sw.wipe_scr.data        + (sw.y_lookup[i]*sw.wipe_scr.byte_pitch+(i*depth));
      for (j=dy;j;j--) {
        for (k=0; k<depth; k++)
          d[k] = s[k];
        d += sw.wipe_scr.byte_pitch;
        s += sw.wipe_scr_end.byte_pitch;
      }
     }
      sw.y_lookup[i] += dy;
     if (V_GetMode() != VID_MODEGL) {
      s = sw.wipe_scr_start.data  + (i*depth);
      d = sw.wipe_scr.data        + (sw.y_lookup[i]*sw.wipe_scr.byte_pitch+(i*depth));
      for (j=SCREENHEIGHT-sw.y_lookup[i];j;j--) {
        for (k=0; k<depth; k++)
          d[k] = s[k];
        d += sw.wipe_scr.byte_pitch;
        s += sw.wipe_scr_end.byte_pitch;
      }
     }
      done = false;
    }
  }

#ifdef GL_DOOM
  if (V_GetMode() == VID_MODEGL) {
    gld_wipe_doMelt(sw.y_lookup);
  }
#endif
  return done;
}

// CPhipps - modified to allocate and deallocate screens[2 to 3] as needed, saving memory

static int wipe_exitMelt(void)
{
#ifdef GL_DOOM
  if (V_GetMode() == VID_MODEGL)
  {
    gld_wipe_exitMelt();
    return 0;
  }
#endif

  V_FreeScreen(&sw.wipe_scr_start);
  sw.wipe_scr_start.width = 0;
  sw.wipe_scr_start.height = 0;
  V_FreeScreen(&sw.wipe_scr_end);
  sw.wipe_scr_end.width = 0;
  sw.wipe_scr_end.height = 0;
  // Paranoia
  screens[SRC_SCR] = sw.wipe_scr_start;
  screens[DEST_SCR] = sw.wipe_scr_end;
  return 0;
}

int wipe_StartScreen(void)
{
  if(!render_wipescreen||wasWiped) return 0;//e6y
  wasWiped = true;//e6y

#ifdef GL_DOOM
  if (V_GetMode() == VID_MODEGL)
  {
    gld_wipe_StartScreen();
    return 0;
  }
#endif

  sw.wipe_scr_start.width = SCREENWIDTH;
  sw.wipe_scr_start.height = SCREENHEIGHT;
  sw.wipe_scr_start.byte_pitch = screens[0].byte_pitch;
  sw.wipe_scr_start.int_pitch = screens[0].int_pitch;
  
  //e6y: fixed slowdown at 1024x768 on some systems
  if (!(sw.wipe_scr_start.byte_pitch % 1024))
    sw.wipe_scr_start.byte_pitch += 32;

  sw.wipe_scr_start.not_on_heap = false;
  V_AllocScreen(&sw.wipe_scr_start);
  screens[SRC_SCR] = sw.wipe_scr_start;
  V_CopyRect(0, SRC_SCR, 0, 0, SCREENWIDTH, SCREENHEIGHT, VPT_NONE); // Copy start screen to buffer
  return 0;
}

int wipe_EndScreen(void)
{
  if(!render_wipescreen||!wasWiped) return 0;//e6y
  wasWiped = false;//e6y

#ifdef GL_DOOM
  if (V_GetMode() == VID_MODEGL)
  {
    gld_wipe_EndScreen();
    return 0;
  }
#endif

  sw.wipe_scr_end.width = SCREENWIDTH;
  sw.wipe_scr_end.height = SCREENHEIGHT;
  sw.wipe_scr_end.byte_pitch = screens[0].byte_pitch;
  sw.wipe_scr_end.int_pitch = screens[0].int_pitch;

  //e6y: fixed slowdown at 1024x768 on some systems
  if (!(sw.wipe_scr_end.byte_pitch % 1024))
    sw.wipe_scr_end.byte_pitch += 32;

  sw.wipe_scr_end.not_on_heap = false;
  V_AllocScreen(&sw.wipe_scr_end);
  screens[DEST_SCR] = sw.wipe_scr_end;
  V_CopyRect(0, DEST_SCR, 0, 0, SCREENWIDTH, SCREENHEIGHT, VPT_NONE); // Copy end screen to buffer
  V_CopyRect(SRC_SCR, 0, 0, 0, SCREENWIDTH, SCREENHEIGHT, VPT_NONE); // restore start screen
  return 0;
}

// killough 3/5/98: reformatted and cleaned up
void wipe_ScreenWipe(void) {
  if(!render_wipescreen)
    return;//e6y

  // initial stuff
  if (!sw.active) {
    sw.active = true;
    sw.wipe_scr = screens[0];
    wipe_initMelt();
  }
}

bool wipe_Tick(int tics) {
  while (tics--) {
    if (doMelt()) {
      wipe_exitMelt();
      sw.active = false;
      return true;
    }
  }

  return false;
}

/* vi: set et ts=2 sw=2: */

