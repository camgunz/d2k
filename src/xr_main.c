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

#include "z_zone.h"

#include "p_setup.h"
#include "p_mobj.h"
#include "r_defs.h"
#include "tables.h"
#include "w_wad.h"
#include "r_demo.h"
#include "r_draw.h"
#include "r_patch.h"
#include "r_things.h"
#include "v_video.h"
#include "x_intern.h"
#include "x_main.h"

#include "gl_opengl.h"
#include "gl_struct.h"

void XR_RegisterInterface(void) {
  X_RegisterObjects("Renderer", 39,
    "filter_point",                     X_INTEGER, RDRAW_FILTER_POINT,
    "filter_linear",                    X_INTEGER, RDRAW_FILTER_LINEAR,
    "filter_rounded",                   X_INTEGER, RDRAW_FILTER_ROUNDED,
    "gl_filter_nearest",                X_INTEGER, filter_nearest,
    "gl_filter_linear",                 X_INTEGER, filter_linear,
    "gl_filter_nearest_mipmap_nearest", X_INTEGER, filter_nearest_mipmap_nearest,
    "gl_filter_nearest_mipmap_linear",  X_INTEGER, filter_nearest_mipmap_linear,
    "gl_filter_linear_mipmap_nearest",  X_INTEGER, filter_linear_mipmap_nearest,
    "gl_filter_linear_mipmap_linear",   X_INTEGER, filter_linear_mipmap_linear,
    "masked_column_edge_square",        X_INTEGER, RDRAW_MASKEDCOLUMNEDGE_SQUARE,
    "masked_column_edge_sloped",        X_INTEGER, RDRAW_MASKEDCOLUMNEDGE_SLOPED,
    "patch_stretch_16x10",              X_INTEGER, patch_stretch_16x10,
    "patch_stretch_4x3",                X_INTEGER, patch_stretch_4x3,
    "patch_stretch_full",               X_INTEGER, patch_stretch_full,
    "sprite_order_static",              X_INTEGER, DOOM_ORDER_STATIC,
    "sprite_order_dynamic",             X_INTEGER, DOOM_ORDER_DYNAMIC,
    "sprite_order_last",                X_INTEGER, DOOM_ORDER_LAST,
    "anisotropic_off",                  X_INTEGER, gl_anisotropic_off,
    "anisotropic_2x",                   X_INTEGER, gl_anisotropic_2x,
    "anisotropic_4x",                   X_INTEGER, gl_anisotropic_4x,
    "anisotropic_8x",                   X_INTEGER, gl_anisotropic_8x,
    "anisotropic_16x",                  X_INTEGER, gl_anisotropic_16x,
    "skytype_auto",                     X_INTEGER, skytype_auto,
    "skytype_none",                     X_INTEGER, skytype_none,
    "skytype_standard",                 X_INTEGER, skytype_standard,
    "skytype_skydome",                  X_INTEGER, skytype_skydome,
    "skytype_screen",                   X_INTEGER, skytype_screen,
    "spriteclip_const",                 X_INTEGER, spriteclip_const,
    "spriteclip_always",                X_INTEGER, spriteclip_always,
    "spriteclip_smart",                 X_INTEGER, spriteclip_smart,
    "hq_scale_none",                    X_INTEGER, hq_scale_none,
    "hq_scale_2x",                      X_INTEGER, hq_scale_2x,
    "hq_scale_3x",                      X_INTEGER, hq_scale_3x,
    "hq_scale_4x",                      X_INTEGER, hq_scale_4x,
    "lightmode_glboom",                 X_INTEGER, gl_lightmode_glboom,
    "lightmode_gzdoom",                 X_INTEGER, gl_lightmode_gzdoom,
    "lightmode_fogbased",               X_INTEGER, gl_lightmode_fogbased,
    "lightmode_shaders",                X_INTEGER, gl_lightmode_shaders,
    "max_gl_gamma",                     X_INTEGER, MAX_GLGAMMA
  );
}

/* vi: set et ts=2 sw=2: */

