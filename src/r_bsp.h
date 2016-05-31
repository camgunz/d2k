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


#ifndef R_BSP_H__
#define R_BSP_H__

struct seg_s;
typedef struct seg_s seg_t;

struct line_s;
typedef struct line_s line_t;

struct drawseg_s;
typedef struct drawseg_s drawseg_t;

struct sector_s;
typedef struct sector_s sector_t;

struct side_s;
typedef struct side_s side_t;

extern seg_t    *curline;
extern side_t   *sidedef;
extern line_t   *linedef;
extern sector_t *frontsector;
extern sector_t *backsector;

/* old code -- killough:
 * extern drawseg_t drawsegs[MAXDRAWSEGS];
 * new code -- killough: */
extern drawseg_t *drawsegs;
extern unsigned maxdrawsegs;

// e6y: resolution limitation is removed
extern unsigned char *solidcol;

extern drawseg_t *ds_p;

void R_ClearClipSegs(void);
void R_ClearDrawSegs(void);
void R_RenderBSPNode(int bspnum);

/* killough 4/13/98: fake floors/ceilings for deep water / fake ceilings: */
sector_t* R_FakeFlat(sector_t *sec, sector_t *tempsec,
                     int *floorlightlevel, int *ceilinglightlevel,
                     bool back);

#endif

/* vi: set et ts=2 sw=2: */

