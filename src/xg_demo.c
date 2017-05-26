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

#include "tables.h"
#include "g_demo.h"
#include "p_mobj.h"
#include "p_setup.h"
#include "r_defs.h"
#include "v_video.h"
#include "w_wad.h"
#include "x_intern.h"
#include "x_main.h"

#include "gl_opengl.h"
#include "gl_struct.h"

void XR_DemoRegisterInterface(void) {
  X_RegisterObjects("Demo", 1,
    "smooth_playing_max_factor", X_INTEGER, SMOOTH_PLAYING_MAXFACTOR
  );
}

/* vi: set et ts=2 sw=2: */

