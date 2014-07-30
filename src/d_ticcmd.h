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


#ifndef D_TICCMD_H__
#define D_TICCMD_H__

#include "doomtype.h"

#ifdef __GNUG__
#pragma interface
#endif

/* The data sampled per tick (single player)
 * and transmitted to other peers (multiplayer).
 * Mainly movements/button commands per game tick,
 * plus a checksum for internal state consistency.
 * CPhipps - explicitely signed the elements, since they have to be signed to work right
 */

 /*
  * CG 04/23/2014: Un-explicitly sign the elements.  If a platform's char's are
  *                implicitly unsigned, that's too bad.  They'll be fixed again
  *                when the grand "use explicitly sized types where needed"
  *                initiative is completed.
  */

typedef struct {
  char   forwardmove;  /* *2048 for move       */
  char   sidemove;     /* *2048 for move       */
  short  angleturn;    /* <<16 for angle delta */
  short  consistancy;  /* checks for net game  */
  byte   chatchar;
  byte   buttons;
} ticcmd_t;

#endif

/* vi: set et ts=2 sw=2: */

