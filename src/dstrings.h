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


#ifndef DSTRINGS__
#define DSTRINGS__

/* All important printed strings.
 * Language selection (message strings).
 * Use -DFRENCH etc.
 */

#ifdef FRENCH
#include "d_french.h"
#else
#include "d_englsh.h"
#endif

/* Note this is not externally modifiable through DEH/BEX
 * Misc. other strings.
 * #define SAVEGAMENAME  "boomsav"      * killough 3/22/98 *
 * Ty 05/04/98 - replaced with a modifiable string, see d_deh.c
 */

/*
 * File locations,
 *  relative to current position.
 * Path names are OS-sensitive.
 */
#define DEVMAPS "devmaps"
#define DEVDATA "devdata"


/* Not done in french?
 * QuitDOOM messages *
 * killough 1/18/98:
 * replace hardcoded limit with extern var (silly hack, I know)
 */

extern const size_t NUM_QUITMESSAGES;  /* Calculated in dstrings.c */

extern const char* const endmsg[];   /* killough 1/18/98 const added */

#endif

/* vi: set et ts=2 sw=2: */

