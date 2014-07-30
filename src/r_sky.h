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


#ifndef R_SKY_H__
#define R_SKY_H__

#include "m_fixed.h"

#ifdef __GNUG__
#pragma interface
#endif

/* SKY, store the number for name. */
#define SKYFLATNAME  "F_SKY1"

/* The sky map is 256*128*4 maps. */
#define ANGLETOSKYSHIFT         22

extern int skyflatnum;
extern int skytexture;
extern int skytexturemid;

#define SKYSTRETCH_HEIGHT 228
extern int r_stretchsky;
extern int skystretch;
extern fixed_t freelookviewheight;

/* Called whenever the view size changes. */
void R_InitSkyMap(void);

#endif

/* vi: set et ts=2 sw=2: */

