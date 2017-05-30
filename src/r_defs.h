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


#ifndef R_DEFS_H__
#define R_DEFS_H__

struct seg_s;
typedef struct seg_s seg_t;

struct mobj_s;
typedef struct mobj_s mobj_t;

// This could be wider for >8 bit display.
// Indeed, true color support is posibble
// precalculating 24bpp lightmap/colormap LUT.
// from darkening PLAYPAL to all black.
// Could use even more than 32 levels.

typedef unsigned char lighttable_t;

// Silhouette, needed for clipping Segs (mainly)
// and sprites representing things.
#define SIL_NONE    0
#define SIL_BOTTOM  1
#define SIL_TOP     2
#define SIL_BOTH    3

#define MAXDRAWSEGS   256

//
// INTERNAL MAP TYPES
//  used by play and refresh
//

//
// Masked 2s linedefs
//

typedef struct drawseg_s
{
  seg_t *curline;
  short x1, x2;
  fixed_t scale1, scale2, scalestep;
  int silhouette;                       // 0=none, 1=bottom, 2=top, 3=both
  fixed_t bsilheight;                   // do not clip sprites above this
  fixed_t tsilheight;                   // do not clip sprites below this

  // Added for filtering (fractional texture u coord) support - POPE
  fixed_t rw_offset, rw_distance, rw_centerangle; 
  
  // Pointers to lists for sprite clipping,
  // all three adjusted so [x1] is first value.

  int *sprtopclip, *sprbottomclip, *maskedtexturecol; // dropoff overflow
} drawseg_t;

// proff: Added for OpenGL
typedef struct patchnum_s {
  int width;
  int height;
  int leftoffset;
  int topoffset;
  int lumpnum;
} patchnum_t;

//
// A vissprite_t is a thing that will be drawn during a refresh.
// i.e. a sprite object that is partly visible.
//

typedef struct vissprite_s
{
  short x1, x2;
  fixed_t gx, gy;              // for line side calculation
  fixed_t gz, gzt;             // global bottom / top for silhouette clipping
  fixed_t startfrac;           // horizontal position of x1
  fixed_t scale;
  fixed_t xiscale;             // negative if flipped
  fixed_t texturemid;
  int patch;
  uint64_t mobjflags;

  // for color translation and shadow draw, maxbright frames as well
  const lighttable_t *colormap;

  // killough 3/27/98: height sector for underwater/fake ceiling support
  int heightsec;
} vissprite_t;

//
// Sprites are patches with a special naming convention
//  so they can be recognized by R_InitSprites.
// The base name is NNNNFx or NNNNFxFx, with
//  x indicating the rotation, x = 0, 1-7.
// The sprite and frame specified by a thing_t
//  is range checked at run time.
// A sprite is a patch_t that is assumed to represent
//  a three dimensional object and may have multiple
//  rotations pre drawn.
// Horizontal flipping is used to save space,
//  thus NNNNF2F5 defines a mirrored patch.
// Some sprites will only have one picture used
// for all views: NNNNF0
//

typedef struct
{
  // If false use 0 for any position.
  // Note: as eight entries are available,
  //  we might as well insert the same name eight times.
  int rotate;

  // Lump to use for view angles 0-7.
  short lump[16];

  // Flip bit (1 = flip) to use for view angles 0-15.
  unsigned short flip;

} spriteframe_t;

//
// A sprite definition:
//  a number of animation frames.
//

typedef struct spritedef_s
{
  int numframes;
  spriteframe_t *spriteframes;
} spritedef_t;

//
// Now what is a visplane, anyway?
//
// Go to http://classicgaming.com/doom/editing/ to find out -- killough
//

typedef struct visplane_s
{
  struct visplane_s *next;      // Next visplane in hash chain -- killough
  int picnum, lightlevel, minx, maxx;
  fixed_t height;
  fixed_t xoffs, yoffs;         // killough 2/28/98: Support scrolling flats
  // e6y: resolution limitation is removed
  // bottom and top arrays are dynamically
  // allocated immediately after the visplane
  unsigned short *bottom;
  unsigned short pad1;          // leave pads for [minx-1]/[maxx+1]
  unsigned short top[3];
} visplane_t;

#endif

/* vi: set et ts=2 sw=2: */
