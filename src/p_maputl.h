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


#ifndef P_MAPUTL_H__
#define P_MAPUTL_H__

struct line_s;
typedef struct line_s line_t;

struct mobj_s;
typedef struct mobj_s mobj_t;

/* mapblocks are used to check movement against lines and things */
#define MAPBLOCKUNITS   128
#define MAPBLOCKSIZE    (MAPBLOCKUNITS*FRACUNIT)
#define MAPBLOCKSHIFT   (FRACBITS+7)
#define MAPBMASK        (MAPBLOCKSIZE-1)
#define MAPBTOFRAC      (MAPBLOCKSHIFT-FRACBITS)

#define PT_ADDLINES     1
#define PT_ADDTHINGS    2
#define PT_EARLYOUT     4

typedef struct {
  fixed_t     x;
  fixed_t     y;
  fixed_t     dx;
  fixed_t     dy;
} divline_t;

typedef struct intercept_s {
  fixed_t     frac;           /* along trace line */
  bool        isaline;
  union {
    mobj_t* thing;
    line_t* line;
  } d;
} intercept_t;

typedef bool (*traverser_t)(intercept_t *in);

fixed_t CONSTFUNC P_AproxDistance (fixed_t dx, fixed_t dy);
int     PUREFUNC  P_PointOnLineSide (fixed_t x, fixed_t y, const line_t *line);
int     PUREFUNC  P_BoxOnLineSide (const fixed_t *tmbox, const line_t *ld);
fixed_t PUREFUNC  P_InterceptVector (const divline_t *v2, const divline_t *v1);
/* cph - old compatibility version below */
fixed_t PUREFUNC  P_InterceptVector2(const divline_t *v2, const divline_t *v1);

extern intercept_t *intercepts, *intercept_p;
void P_MakeDivline(const line_t *li, divline_t *dl);
int PUREFUNC P_PointOnDivlineSide(fixed_t x, fixed_t y, const divline_t *line);
void check_intercept(void);

void P_LineOpening (const line_t *linedef);
void P_UnsetThingPosition(mobj_t *thing);
void P_SetThingPosition(mobj_t *thing);
bool P_BlockLinesIterator (int x, int y, bool func(line_t *));
bool P_BlockThingsIterator(int x, int y, bool func(mobj_t *));
bool P_PathTraverse(fixed_t x1, fixed_t y1, fixed_t x2, fixed_t y2,
                   int flags, bool trav(intercept_t *));

// MAES: support 512x512 blockmaps.
int P_GetSafeBlockX(int coord);
int P_GetSafeBlockY(int coord);

extern fixed_t opentop;
extern fixed_t openbottom;
extern fixed_t openrange;
extern fixed_t lowfloor;
extern divline_t trace;

#endif  /* __P_MAPUTL__ */

/* vi: set et ts=2 sw=2: */

