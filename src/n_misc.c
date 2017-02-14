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
#include "g_game.h"
#include "p_user.h"
#include "n_main.h"
#include "cl_main.h"

#define LOG_COMMANDS 0
#define LOG_POSITIONS 0

/* [CG] TODO: Add WAD fetching (waiting on libcurl) */
bool N_GetWad(const char *name) {
  return false;
}

const char* N_RunningStateName(void) {
  if (CLIENT) {
    if (CL_Predicting()) {
      return "predicting";
    }

    if (CL_RePredicting()) {
      return "re-predicting";
    }

    if (CL_Synchronizing()) {
      return "synchronizing";
    }
  }
  else if (SERVER) {
    return "server";
  }

  return "unknown!";
}

/* vi: set et ts=2 sw=2: */
