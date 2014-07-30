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


#ifndef R_SCREENMULTIPLY_H__
#define R_SCREENMULTIPLY_H__

#ifdef __GNUG__
#pragma interface
#endif

#include "doomtype.h"

extern int render_screen_multiply;
extern int screen_multiply;
extern int render_interlaced_scanning;
extern int interlaced_scanning_requires_clearing;

void R_ProcessScreenMultiply(byte* pixels_src, byte* pixels_dest,
  int pixel_depth, int pitch_src, int pitch_dest);

#endif

/* vi: set et ts=2 sw=2: */

