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

#ifndef P_SETUP_H__
#define P_SETUP_H__

#include "p_mobj.h"

#ifdef __GNUG__
#pragma interface
#endif

void P_SetupLevel(int episode, int map, int playermask, skill_t skill);
void P_Init(void);               /* Called by startup code. */

extern const byte *rejectmatrix;   /* for fast sight rejection -  cph - const* */

/* killough 3/1/98: change blockmap from "short" to "long" offsets: */
extern int      *blockmaplump;   /* offsets in blockmap are from here */
extern int      *blockmap;
extern int      bmapwidth;
extern int      bmapheight;      /* in mapblocks */
extern fixed_t  bmaporgx;
extern fixed_t  bmaporgy;        /* origin of block map */
extern mobj_t   **blocklinks;    /* for thing chains */

// MAES: extensions to support 512x512 blockmaps.
extern int blockmapxneg;
extern int blockmapyneg;

#endif

