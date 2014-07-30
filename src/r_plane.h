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

#ifndef R_PLANE_H__
#define R_PLANE_H__

#include "r_data.h"

#ifdef __GNUG__
#pragma interface
#endif

/* killough 10/98: special mask indicates sky flat comes from sidedef */
#define PL_SKYFLAT (0x80000000)

/* Visplane related. */
extern int *lastopening; // dropoff overflow

// e6y: resolution limitation is removed
extern int *floorclip, *ceilingclip; // dropoff overflow
extern fixed_t *yslope, *distscale;

void R_InitVisplanesRes(void);
void R_InitPlanesRes(void);
void R_InitPlanes(void);
void R_ClearPlanes(void);
void R_DrawPlanes (void);

visplane_t *R_FindPlane(
                        fixed_t height,
                        int picnum,
                        int lightlevel,
                        fixed_t xoffs,  /* killough 2/28/98: add x-y offsets */
                        fixed_t yoffs
                       );

visplane_t *R_CheckPlane(visplane_t *pl, int start, int stop);
visplane_t *R_DupPlane(const visplane_t *pl, int start, int stop);

#endif

