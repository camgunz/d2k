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


#ifndef R_MAIN_H__
#define R_MAIN_H__

#include "d_player.h"
#include "r_data.h"

#ifdef __GNUG__
#pragma interface
#endif

extern int r_frame_count;

//
// POV related.
//

extern fixed_t  viewcos;
extern fixed_t  viewsin;
extern int      viewwidth;
extern int      viewheight;
extern int      viewwindowx;
extern int      viewwindowy;
extern int      centerx;
extern int      centery;
extern fixed_t  centerxfrac;
extern fixed_t  centeryfrac;
extern fixed_t  viewheightfrac; //e6y: for correct cliping of things
extern fixed_t  projection;
extern fixed_t  skyiscale;
// e6y: wide-res
extern int wide_centerx;
extern int wide_offsetx;
extern int wide_offset2x;
extern int wide_offsety;
extern int wide_offset2y;
#define RMUL (1.6f/1.333333f)

// proff 11/06/98: Added for high-res
extern fixed_t  projectiony;
extern int      validcount;
// e6y: Added for more precise flats drawing
extern float viewfocratio;

//
// Rendering stats
//

extern int rendered_visplanes, rendered_segs, rendered_vissprites;
extern bool rendering_stats;

//
// Lighting LUT.
// Used for z-depth cuing per column/row,
//  and other lighting effects (sector ambient, flash).
//

// Lighting constants.

// SoM: I am really speechless at this... just... why?
// Lighting in doom was originally clamped off to just 16 brightness levels
// for sector lighting. Simply changing the constants is enough to change this
// it seriously bottles the mind why this wasn't done in doom from the start 
// except for maybe memory usage savings. 
#define LIGHTLEVELS_MAX   32

extern int LIGHTSEGSHIFT;
extern int LIGHTBRIGHT;
extern int LIGHTLEVELS;
extern int render_doom_lightmaps;

#define MAXLIGHTSCALE     48
#define LIGHTSCALESHIFT   12
#define MAXLIGHTZ        128
#define LIGHTZSHIFT       20

/* cph - allow crappy fake contrast to be disabled */
extern int fake_contrast;

// killough 3/20/98: Allow colormaps to be dynamic (e.g. underwater)
extern const lighttable_t *(*scalelight)[MAXLIGHTSCALE];
extern const lighttable_t *(*c_zlight)[LIGHTLEVELS_MAX][MAXLIGHTZ];
extern const lighttable_t *(*zlight)[MAXLIGHTZ];
extern const lighttable_t *fullcolormap;
extern int numcolormaps;    // killough 4/4/98: dynamic number of maps
extern const lighttable_t **colormaps;
// killough 3/20/98, 4/4/98: end dynamic colormaps

//e6y: for Boom colormaps in OpenGL mode
extern bool use_boom_cm;
extern int  boom_cm;         // current colormap
extern int  frame_fixedcolormap;

extern int          extralight;
extern const lighttable_t *fixedcolormap;

// Number of diminishing brightness levels.
// There a 0-31, i.e. 32 LUT in the COLORMAP lump.

#define NUMCOLORMAPS 32
// Index of the special effects (INVUL inverse) map.
#define INVERSECOLORMAP 32

//
// Utility functions.
//

PUREFUNC int R_PointOnSide(fixed_t x, fixed_t y, const node_t *node);
PUREFUNC int R_PointOnSegSide(fixed_t x, fixed_t y, const seg_t *line);
angle_t R_PointToAngle(fixed_t x, fixed_t y);
angle_t R_PointToAngle2(fixed_t x1, fixed_t y1, fixed_t x2, fixed_t y2);
subsector_t *R_PointInSubsector(fixed_t x, fixed_t y);

//e6y: made more precise
angle_t R_PointToAngleEx(fixed_t x, fixed_t y);
//e6y: caching
angle_t R_GetVertexViewAngle(vertex_t *v);
angle_t R_GetVertexViewAngleGL(vertex_t *v);
angle_t R_PointToPseudoAngle(fixed_t x, fixed_t y);

//e6y
typedef enum { render_precise_speed, render_precise_quality } render_precise_t;
extern render_precise_t render_precise;
extern const char *render_precises[];

extern int r_have_internal_hires;

//
// REFRESH - the actual rendering functions.
//

void R_RenderPlayerView(player_t *player);   // Called by G_Drawer.
void R_Init(void);                           // Called by startup code.
void R_SetViewSize(int blocks);              // Called by M_Responder.
void R_ExecuteSetViewSize(void);             // cph - called by D_Display to complete a view resize

void R_ShowStats(void);
void R_ClearStats(void);

#define Pi 3.14159265358979323846f
#define DEG2RAD(a) ((a * Pi) / 180.0f)
#define RAD2DEG(a) ((a / Pi) * 180.0f)
#define MAP_COEFF 128.0f
#define MAP_SCALE (MAP_COEFF*(float)FRACUNIT)

extern int viewport[4];
extern float modelMatrix[16];
extern float projMatrix[16];
int R_Project(float objx, float objy, float objz, float *winx, float *winy, float *winz);

#endif

/* vi: set et ts=2 sw=2: */

