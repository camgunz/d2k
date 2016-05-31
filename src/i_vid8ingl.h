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


#ifndef I_VID_IN_8GL__
#define I_VID_IN_8GL__

#ifdef GL_DOOM

typedef struct vid_8ingl_s {
  int enabled;
  SDL_Surface *screen;
  SDL_Surface *surface;
  GLuint texid;
  GLuint pboids[2];
  int width;
  int height;
  int size;
  unsigned char *buf;
  unsigned char *colours;
  int palette;
  float fU1;
  float fU2;
  float fV1;
  float fV2;
} vid_8ingl_t;

extern vid_8ingl_t vid_8ingl;
extern int use_gl_surface;

#endif
#endif

/* vi: set et ts=2 sw=2: */

