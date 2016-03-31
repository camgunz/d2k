
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

#include "c_eci.h"
#include "c_main.h"
#include "x_intern.h"
#include "x_main.h"

int XC_HandleInput(lua_State *L) {
  const char *input_text = luaL_checkstring(L, -1);

  C_HandleInput((char *)input_text);

  return 0;
}

void XC_RegisterInterface(void) {
  X_RegisterObjects("CommandInterface", 1,
    "handle_input", X_FUNCTION, XC_HandleInput
  );
}

/* vi: set et ts=2 sw=2: */

