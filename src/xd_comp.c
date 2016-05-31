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
#include "doomstat.h"
#include "g_game.h"
#include "x_intern.h"
#include "x_main.h"

void XD_CompatibilityRegisterInterface(void) {
  X_RegisterObjects("Compatibility", 21,
    "doom_12",            X_INTEGER, doom_12_compatibility,
    "doom_1666",          X_INTEGER, doom_1666_compatibility,
    "doom2_19",           X_INTEGER, doom2_19_compatibility,
    "ultdoom",            X_INTEGER, ultdoom_compatibility,
    "finaldoom",          X_INTEGER, finaldoom_compatibility,
    "dosdoom",            X_INTEGER, dosdoom_compatibility,
    "tasdoom",            X_INTEGER, tasdoom_compatibility,
    "boom_compatibility", X_INTEGER, boom_compatibility_compatibility,
    "boom_201",           X_INTEGER, boom_201_compatibility,
    "boom_202",           X_INTEGER, boom_202_compatibility,
    "lxdoom_1",           X_INTEGER, lxdoom_1_compatibility,
    "mbf",                X_INTEGER, mbf_compatibility,
    "prboom_1",           X_INTEGER, prboom_1_compatibility,
    "prboom_2",           X_INTEGER, prboom_2_compatibility,
    "prboom_3",           X_INTEGER, prboom_3_compatibility,
    "prboom_4",           X_INTEGER, prboom_4_compatibility,
    "prboom_5",           X_INTEGER, prboom_5_compatibility,
    "prboom_6",           X_INTEGER, prboom_6_compatibility,
    "max",                X_INTEGER, MAX_COMPATIBILITY_LEVEL,
    "boom",               X_INTEGER, boom_compatibility,
    "best",               X_INTEGER, best_compatibility
  );
}

/* vi: set et ts=2 sw=2: */

