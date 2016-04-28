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

#include "doomdef.h"
#include "d_main.h"
#include "i_sound.h"
#include "p_mobj.h"
#include "s_sound.h"
#include "x_intern.h"
#include "x_main.h"

void XS_RegisterInterface(void) {
  X_RegisterObjects("Sound", 3,
    "sound_disabled", X_BOOLEAN, SOUND_DISABLED,
    "music_disabled", X_BOOLEAN, MUSIC_DISABLED,
    "max_channels",   X_INTEGER, MAX_SOUND_CHANNELS
  );
}

/* vi: set et ts=2 sw=2: */

