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


#ifndef I_VIDEO_H__
#define I_VIDEO_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "doomtype.h"
#include "v_video.h"

#ifdef __GNUG__
#pragma interface
#endif

extern const char *screen_resolutions_list[];
extern const char *screen_resolution;

extern const char *sdl_videodriver;
extern const char *sdl_video_window_pos;

void I_PreInitGraphics(void); /* CPhipps - do stuff immediately on start */
void I_InitScreenResolution(void); /* init resolution */
void I_SetWindowCaption(void); /* Set the window caption */
void I_SetWindowIcon(void); /* Set the application icon */
void I_InitGraphics (void);
void I_UpdateVideoMode(void);
void I_ShutdownGraphics(void);

void* I_GetRenderContext(void);
void  I_ResetRenderContext(void);
void  I_RenderOverlay(void);

/* Takes full 8 bit values. */
void I_SetPalette(int pal); /* CPhipps - pass down palette number */

void I_UpdateNoBlit (void);
void I_FinishUpdate (void);

int I_ScreenShot (const char *fname);
// NSM expose lower level screen data grab for vidcap
unsigned char *I_GrabScreen (void);

/* I_StartTic
 * Called by D_DoomLoop,
 * called before processing each tic in a frame.
 * Quick syncronous operations are performed here.
 * Can call D_PostEvent.
 */
void I_StartTic (void);

/* I_StartFrame
 * Called by D_DoomLoop,
 * called before processing any tics in a frame
 * (just after displaying a frame).
 * Time consuming syncronous operations
 * are performed here (joystick reading).
 * Can call D_PostEvent.
 */

void I_StartFrame (void);

extern int use_doublebuffer;  /* proff 2001-7-4 - controls wether to use doublebuffering*/
extern int use_fullscreen;  /* proff 21/05/2000 */
extern int desired_fullscreen; //e6y

// Set the process affinity mask so that all threads
extern int process_affinity_mask;
// Priority class for the prboom-plus process
extern int process_priority;
// Try to optimise screen pitch for reducing of CPU cache misses.
extern int try_to_reduce_cpu_cache_misses;

extern dboolean window_focused;
void UpdateGrab(void);

const char* I_GetKeyString(int keycode);

#ifdef GL_DOOM
typedef struct SDL_Surface *PSDL_Surface;
typedef struct vid_8ingl_s
{
  int enabled;

  PSDL_Surface screen;
  PSDL_Surface surface;

  GLuint texid;
  GLuint pboids[2];

  int width, height, size;
  unsigned char *buf;

  byte *colours;
  int palette;

  float fU1, fU2, fV1, fV2;
} vid_8ingl_t;

extern vid_8ingl_t vid_8ingl;
extern int use_gl_surface;
#endif // GL_DOOM

#endif

/* vi: set et ts=2 sw=2: */

