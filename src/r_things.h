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


#ifndef R_THINGS_H__
#define R_THINGS_H__

#define MINZ (FRACUNIT*4)

struct rcolumn_s;
typedef struct rcolumn_s rcolumn_t;

struct rpatch_s;
typedef struct rpatch_s rpatch_t;

extern int r_near_clip_plane;

/* Constant arrays used for psprite clipping and initializing clipping. */

// e6y: resolution limitation is removed
extern int *negonearray;       /* killough 2/8/98: */ // dropoff overflow
extern int *screenheightarray; /* change to MAX_*  */ // dropoff overflow

/* Vars for R_DrawMaskedColumn */

extern int     *mfloorclip;    // dropoff overflow
extern int     *mceilingclip;  // dropoff overflow
extern fixed_t spryscale;
extern fixed_t sprtopscreen;
extern fixed_t pspriteiscale;
/* proff 11/06/98: Added for high-res */
extern fixed_t pspritexscale;
extern fixed_t pspriteyscale;
//e6y: added for GL
extern float pspritexscale_f;
extern float pspriteyscale_f;

typedef enum {
  DOOM_ORDER_NONE,
  DOOM_ORDER_STATIC,
  DOOM_ORDER_DYNAMIC,
  DOOM_ORDER_LAST
} sprite_doom_order_t;
extern int sprites_doom_order;

extern int health_bar;
extern int health_bar_full_length;
extern int health_bar_red;
extern int health_bar_yellow;
extern int health_bar_green;

void R_DrawMaskedColumn(const rpatch_t *patch,
                        R_DrawColumn_f colfunc,
                        draw_column_vars_t *dcvars,
                        const rcolumn_t *column,
                        const rcolumn_t *prevcolumn,
                        const rcolumn_t *nextcolumn);
void R_SortVisSprites(void);
void R_AddSprites(subsector_t* subsec, int lightlevel);
void R_AddAllAliveMonstersSprites(void);
void R_DrawPlayerSprites(void);
void R_InitSpritesRes(void);
void R_InitSprites(const char * const * namelist);
void R_ClearSprites(void);
void R_DrawMasked(void);

void R_SetClipPlanes(void);

#endif

/* vi: set et ts=2 sw=2: */

