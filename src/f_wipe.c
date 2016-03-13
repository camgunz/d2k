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
#include "i_video.h"
#include "v_video.h"
#include "m_random.h"
#include "f_wipe.h"
#ifdef GL_DOOM
#include "gl_struct.h"
#endif
#include "e6y.h"//e6y
#include "n_net.h"

//
// SCREEN WIPE PACKAGE
//

// Parts re-written to support true-color video modes. Column-major
// formatting removed. - POPE

// CPhipps - macros for the source and destination screens
#define SRC_SCR 2
#define DEST_SCR 3

static screeninfo_t wipe_scr_start;
static screeninfo_t wipe_scr_end;
static screeninfo_t wipe_scr;

// e6y: resolution limitation is removed
static int *y_lookup = NULL;

// e6y: resolution limitation is removed
void R_InitMeltRes(void) {
  if (y_lookup)
    free(y_lookup);

  y_lookup = calloc(1, SCREENWIDTH * sizeof(*y_lookup));
}

static int wipe_initMelt(int ticks) {
  if (V_GetMode() != VID_MODEGL) {
    // copy start screen to main screen
    for (int i = 0; i < SCREENHEIGHT; i++) {
      memcpy(
        wipe_scr.data + i * wipe_scr.byte_pitch,
        wipe_scr_start.data + i * wipe_scr_start.byte_pitch,
         SCREENWIDTH * V_GetPixelDepth()
      );
    }
  }

  // setup initial column positions (y<0 => not ready to scroll yet)
  if (DELTASYNC)
    y_lookup[0] = -(D_RandomRange(0, 16));
  else
    y_lookup[0] = -(M_Random() % 16);

  for (int i = 1; i < SCREENWIDTH; i++) {
    int r;

    if (DELTASYNC)
      r = D_RandomRange(-1, 2);
    else
      r = (M_Random() % 3) - 1;

    y_lookup[i] = y_lookup[i - 1] + r;

    if (y_lookup[i] > 0)
      y_lookup[i] = 0;
    else if (y_lookup[i] == -16)
      y_lookup[i] = -15;
  }

  return 0;
}

static int wipe_doMelt(int ticks)
{
  bool done = true;
  int i;
  const int depth = V_GetPixelDepth();

  while (ticks--) {
    for (i=0;i<(SCREENWIDTH);i++) {
      if (y_lookup[i]<0) {
        y_lookup[i]++;
        done = false;
        continue;
      }
      if (y_lookup[i] < SCREENHEIGHT) {
        byte *s, *d;
        int j, k, dy;

        /* cph 2001/07/29 -
          *  The original melt rate was 8 pixels/sec, i.e. 25 frames to melt
          *  the whole screen, so make the melt rate depend on SCREENHEIGHT
          *  so it takes no longer in high res
          */
        dy = (y_lookup[i] < 16) ? y_lookup[i]+1 : SCREENHEIGHT/25;
        if (y_lookup[i]+dy >= SCREENHEIGHT)
          dy = SCREENHEIGHT - y_lookup[i];

       if (V_GetMode() != VID_MODEGL) {
        s = wipe_scr_end.data    + (y_lookup[i]*wipe_scr_end.byte_pitch+(i*depth));
        d = wipe_scr.data        + (y_lookup[i]*wipe_scr.byte_pitch+(i*depth));
        for (j=dy;j;j--) {
          for (k=0; k<depth; k++)
            d[k] = s[k];
          d += wipe_scr.byte_pitch;
          s += wipe_scr_end.byte_pitch;
        }
       }
        y_lookup[i] += dy;
       if (V_GetMode() != VID_MODEGL) {
        s = wipe_scr_start.data  + (i*depth);
        d = wipe_scr.data        + (y_lookup[i]*wipe_scr.byte_pitch+(i*depth));
        for (j=SCREENHEIGHT-y_lookup[i];j;j--) {
          for (k=0; k<depth; k++)
            d[k] = s[k];
          d += wipe_scr.byte_pitch;
          s += wipe_scr_end.byte_pitch;
        }
       }
        done = false;
      }
    }
  }
#ifdef GL_DOOM
  if (V_GetMode() == VID_MODEGL)
  {
    gld_wipe_doMelt(ticks, y_lookup);
  }
#endif
  return done;
}

// CPhipps - modified to allocate and deallocate screens[2 to 3] as needed, saving memory

static int wipe_exitMelt(int ticks)
{
#ifdef GL_DOOM
  if (V_GetMode() == VID_MODEGL)
  {
    gld_wipe_exitMelt(ticks);
    return 0;
  }
#endif

  V_FreeScreen(&wipe_scr_start);
  wipe_scr_start.width = 0;
  wipe_scr_start.height = 0;
  V_FreeScreen(&wipe_scr_end);
  wipe_scr_end.width = 0;
  wipe_scr_end.height = 0;
  // Paranoia
  screens[SRC_SCR] = wipe_scr_start;
  screens[DEST_SCR] = wipe_scr_end;
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

  wipe_scr_start.width = SCREENWIDTH;
  wipe_scr_start.height = SCREENHEIGHT;
  wipe_scr_start.byte_pitch = screens[0].byte_pitch;
  wipe_scr_start.int_pitch = screens[0].int_pitch;
  
  //e6y: fixed slowdown at 1024x768 on some systems
  if (!(wipe_scr_start.byte_pitch % 1024))
    wipe_scr_start.byte_pitch += 32;

  wipe_scr_start.not_on_heap = false;
  V_AllocScreen(&wipe_scr_start);
  screens[SRC_SCR] = wipe_scr_start;
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

  wipe_scr_end.width = SCREENWIDTH;
  wipe_scr_end.height = SCREENHEIGHT;
  wipe_scr_end.byte_pitch = screens[0].byte_pitch;
  wipe_scr_end.int_pitch = screens[0].int_pitch;

  //e6y: fixed slowdown at 1024x768 on some systems
  if (!(wipe_scr_end.byte_pitch % 1024))
    wipe_scr_end.byte_pitch += 32;

  wipe_scr_end.not_on_heap = false;
  V_AllocScreen(&wipe_scr_end);
  screens[DEST_SCR] = wipe_scr_end;
  V_CopyRect(0, DEST_SCR, 0, 0, SCREENWIDTH, SCREENHEIGHT, VPT_NONE); // Copy end screen to buffer
  V_CopyRect(SRC_SCR, 0, 0, 0, SCREENWIDTH, SCREENHEIGHT, VPT_NONE); // restore start screen
  return 0;
}

// killough 3/5/98: reformatted and cleaned up
int wipe_ScreenWipe(int ticks) {
  static bool go; // when zero, stop the wipe

  if(!render_wipescreen)
    return 0;//e6y

  // initial stuff
  if (!go) {
    go = 1;
    wipe_scr = screens[0];
    wipe_initMelt(ticks);
  }

  // do a piece of wipe-in
  // final stuff
  if (wipe_doMelt(ticks)) {
    wipe_exitMelt(ticks);
    go = 0;
  }

  return !go;
}

/* vi: set et ts=2 sw=2: */

