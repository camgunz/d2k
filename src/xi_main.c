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

#include "i_main.h"
#include "i_system.h"
#include "x_intern.h"
#include "x_main.h"

static int XF_Quit(lua_State *L) {
  I_SafeExit(0);

  return 0;
}

static int XF_GetTicks(lua_State *L) {
  lua_pushinteger(L, I_GetTicks());

  return 1;
}

static int XF_Print(lua_State *L) {
  const char *s = luaL_checkstring(L, -1);

  printf("%s", s);

  return 0;
}

void XI_RegisterInterface(void) {
  X_RegisterObjects("System", 4,
    "get_ticks", X_FUNCTION, XF_GetTicks,
    "quit",      X_FUNCTION, XF_Quit,
    "exit",      X_FUNCTION, XF_Quit,
    "print",     X_FUNCTION, XF_Print
  );
}

/* vi: set et ts=2 sw=2: */
