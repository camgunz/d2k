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


#ifndef R_PATCH_H__
#define R_PATCH_H__

// Used to specify the sloping of the top and bottom of a column post
typedef enum {
  RDRAW_EDGESLOPE_TOP_UP   = (1<<0),
  RDRAW_EDGESLOPE_TOP_DOWN = (1<<1),
  RDRAW_EDGESLOPE_BOT_UP   = (1<<2),
  RDRAW_EDGESLOPE_BOT_DOWN = (1<<3),
  RDRAW_EDGESLOPE_TOP_MASK = 0x3,
  RDRAW_EDGESLOPE_BOT_MASK = 0xc,  
} edgeslope_t;

//e6y
typedef enum {
  PATCH_ISNOTTILEABLE   = 0x00000001,
  PATCH_REPEAT          = 0x00000002,
  PATCH_HASHOLES        = 0x00000004,
} rpatch_flag_t;

typedef struct {
  int topdelta;
  int length;
  edgeslope_t slope;
} rpost_t;

typedef struct rcolumn_s {
  int numPosts;
  rpost_t *posts;
  unsigned char *pixels;
} rcolumn_t;

typedef struct rpatch_s {
  int width;
  int height;
  unsigned  widthmask;
    
  int leftoffset;
  int topoffset;
  
  // this is the single malloc'ed/free'd array 
  // for this patch
  unsigned char *data;
  
  // these are pointers into the data array
  unsigned char *pixels;
  rcolumn_t *columns;
  rpost_t *posts;

#ifdef TIMEDIAG
  int locktic;
#endif
  unsigned int locks;
  unsigned int flags;//e6y
} rpatch_t;


const rpatch_t *R_CachePatchNum(int id);
void R_UnlockPatchNum(int id);
#define R_CachePatchName(name) R_CachePatchNum(W_GetNumForName(name))
#define R_UnlockPatchName(name) R_UnlockPatchNum(W_GetNumForName(name))

const rpatch_t *R_CacheTextureCompositePatchNum(int id);
void R_UnlockTextureCompositePatchNum(int id);


// Size query funcs
int R_NumPatchWidth(int lump) ;
int R_NumPatchHeight(int lump);
#define R_NamePatchWidth(name) R_NumPatchWidth(W_GetNumForName(name))
#define R_NamePatchHeight(name) R_NumPatchHeight(W_GetNumForName(name))


const rcolumn_t *R_GetPatchColumnWrapped(const rpatch_t *patch, int columnIndex);
const rcolumn_t *R_GetPatchColumnClamped(const rpatch_t *patch, int columnIndex);


// returns R_GetPatchColumnWrapped for square, non-holed textures
// and R_GetPatchColumnClamped otherwise
const rcolumn_t *R_GetPatchColumn(const rpatch_t *patch, int columnIndex);


void R_InitPatches();
void R_FlushAllPatches();

#endif

/* vi: set et ts=2 sw=2: */

