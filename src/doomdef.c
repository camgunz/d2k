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
/* vi: set et ts=2 sw=2:                                                     */
/*                                                                           */
/*****************************************************************************/

#ifdef __GNUG__
#pragma implementation "doomdef.h"
#endif

#include "doomdef.h"

// Location for any defines turned variables.
// None.

// proff 08/17/98: Changed for high-res
int SCREENWIDTH=320;
int SCREENHEIGHT=200;
int SCREENPITCH=320;

int REAL_SCREENWIDTH;
int REAL_SCREENHEIGHT;
int REAL_SCREENPITCH;

// e6y: wide-res
int SCREEN_320x200;
int WIDE_SCREENWIDTH = 320;
int WIDE_SCREENHEIGHT = 200;
