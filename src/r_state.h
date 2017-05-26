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


#ifndef R_STATE_H__
#define R_STATE_H__

struct spritedef_s;
typedef struct spritedef_s spritedef_t;

struct sector_s;
typedef struct sector_s sector_t;

struct side_s;
typedef struct side_s side_t;

struct line_s;
typedef struct line_s line_t;

struct ssline_s;
typedef struct ssline_s ssline_t;

struct visplane_s;
typedef struct visplane_s visplane_t;

struct player_s;
typedef struct player_s player_t;

//
// Refresh internal data structures,
//  for rendering.
//

// needed for texture pegging
extern fixed_t *textureheight;

extern int scaledviewwidth;

extern int firstflat, numflats;

// for global animation
extern int *flattranslation;
extern int *texturetranslation;

// Sprite....
extern int firstspritelump;
extern int lastspritelump;
extern int numspritelumps;

//
// Lookup tables for map data.
//
extern int              numsprites;
extern spritedef_t      *sprites;

//
// POV data.
//
extern fixed_t          viewx;
extern fixed_t          viewy;
extern fixed_t          viewz;
extern angle_t          viewangle;
extern player_t         *viewplayer;
extern angle_t          clipangle;
extern int              viewangletox[FINEANGLES/2];

// e6y: resolution limitation is removed
extern angle_t          *xtoviewangle;  // killough 2/8/98

extern int              FieldOfView;

extern fixed_t          rw_distance;
extern angle_t          rw_normalangle;

// angle to line origin
extern int              rw_angle1;

extern visplane_t       *floorplane;
extern visplane_t       *ceilingplane;

#endif

/* vi: set et ts=2 sw=2: */

