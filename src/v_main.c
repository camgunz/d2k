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

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include "lprintf.h"
#include "x_main.h"

void V_InitOverlay(void) {
  if (!X_CallFunc("screen", "clear", 0, 0))
    I_Error("Error initializing overlay: %s", X_StrError());

#if 0
  lua_State *L = X_GetState();

  lua_getglobal(L, X_NAMESPACE);
  lua_getfield(L, -1, "screen");
  lua_remove(L, -2);
  lua_getfield(L, -1, "clear");
  lua_remove(L, -2);
  lua_getglobal(L, X_NAMESPACE);
  lua_getfield(L, -1, "screen");
  lua_remove(L, -2);
  if (lua_pcall(L, 1, 0, 0) != 0)
    I_Error("Error clearing overlay: %s", X_StrError());

  printf("V_ClearOverlay: Stack size: %d\n", lua_gettop(L));
#endif
}

void V_ClearOverlay(void) {
  if (!X_CallFunc("screen", "clear", 0, 0))
    I_Error("Error clearing overlay: %s", X_StrError());

#if 0
  lua_State *L = X_GetState();

  lua_getglobal(L, X_NAMESPACE);
  lua_getfield(L, -1, "screen");
  lua_remove(L, -2);
  lua_getfield(L, -1, "clear");
  lua_remove(L, -2);
  lua_getglobal(L, X_NAMESPACE);
  lua_getfield(L, -1, "screen");
  lua_remove(L, -2);
  if (lua_pcall(L, 1, 0, 0) != 0)
    I_Error("Error clearing overlay: %s", X_StrError());

  printf("V_ClearOverlay: Stack size: %d\n", lua_gettop(L));
#endif
}

/* vi: set et ts=2 sw=2: */

