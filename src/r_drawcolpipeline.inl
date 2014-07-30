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

// no color mapping
#define R_DRAWCOLUMN_FUNCNAME R_DRAWCOLUMN_FUNCNAME_COMPOSITE(_PointUV)
#define R_DRAWCOLUMN_PIPELINE (R_DRAWCOLUMN_PIPELINE_BASE | RDC_NOCOLMAP)
#include "r_drawcolumn.inl"

// simple depth color mapping
#define R_DRAWCOLUMN_FUNCNAME R_DRAWCOLUMN_FUNCNAME_COMPOSITE(_PointUV_PointZ)
#define R_DRAWCOLUMN_PIPELINE R_DRAWCOLUMN_PIPELINE_BASE
#include "r_drawcolumn.inl"

// z-dither
#define R_DRAWCOLUMN_FUNCNAME R_DRAWCOLUMN_FUNCNAME_COMPOSITE(_PointUV_LinearZ)
#define R_DRAWCOLUMN_PIPELINE (R_DRAWCOLUMN_PIPELINE_BASE | RDC_DITHERZ)
#include "r_drawcolumn.inl"

// bilinear with no color mapping
#define R_DRAWCOLUMN_FUNCNAME R_DRAWCOLUMN_FUNCNAME_COMPOSITE(_LinearUV)
#define R_DRAWCOLUMN_PIPELINE (R_DRAWCOLUMN_PIPELINE_BASE | RDC_BILINEAR | RDC_NOCOLMAP)
#include "r_drawcolumn.inl"

// bilinear with simple depth color mapping
#define R_DRAWCOLUMN_FUNCNAME R_DRAWCOLUMN_FUNCNAME_COMPOSITE(_LinearUV_PointZ)
#define R_DRAWCOLUMN_PIPELINE (R_DRAWCOLUMN_PIPELINE_BASE | RDC_BILINEAR)
#include "r_drawcolumn.inl"

// bilinear + z-dither
#define R_DRAWCOLUMN_FUNCNAME R_DRAWCOLUMN_FUNCNAME_COMPOSITE(_LinearUV_LinearZ)
#define R_DRAWCOLUMN_PIPELINE (R_DRAWCOLUMN_PIPELINE_BASE | RDC_BILINEAR | RDC_DITHERZ)
#include "r_drawcolumn.inl"

// rounded with no color mapping
#define R_DRAWCOLUMN_FUNCNAME R_DRAWCOLUMN_FUNCNAME_COMPOSITE(_RoundedUV)
#define R_DRAWCOLUMN_PIPELINE (R_DRAWCOLUMN_PIPELINE_BASE | RDC_ROUNDED | RDC_NOCOLMAP)
#include "r_drawcolumn.inl"

// rounded with simple depth color mapping
#define R_DRAWCOLUMN_FUNCNAME R_DRAWCOLUMN_FUNCNAME_COMPOSITE(_RoundedUV_PointZ)
#define R_DRAWCOLUMN_PIPELINE (R_DRAWCOLUMN_PIPELINE_BASE | RDC_ROUNDED)
#include "r_drawcolumn.inl"

// rounded + z-dither
#define R_DRAWCOLUMN_FUNCNAME R_DRAWCOLUMN_FUNCNAME_COMPOSITE(_RoundedUV_LinearZ)
#define R_DRAWCOLUMN_PIPELINE (R_DRAWCOLUMN_PIPELINE_BASE | RDC_ROUNDED | RDC_DITHERZ)
#include "r_drawcolumn.inl"

#undef R_FLUSHWHOLE_FUNCNAME
#undef R_FLUSHHEADTAIL_FUNCNAME
#undef R_FLUSHQUAD_FUNCNAME
#undef R_DRAWCOLUMN_FUNCNAME_COMPOSITE
#undef R_DRAWCOLUMN_PIPELINE_BITS

