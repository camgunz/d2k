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
  lua_State *L = X_GetState();

  lua_getglobal(L, X_NAMESPACE);
  lua_getfield(L, -1, "hud");
  lua_remove(L, -2);
  lua_getfield(L, -1, "clear");
  lua_remove(L, -2);
  lua_getglobal(L, X_NAMESPACE);
  lua_getfield(L, -1, "hud");
  lua_remove(L, -2);
  if (lua_pcall(L, 1, 0, 0) != LUA_OK)
    I_Error("Error clearing overlay: %s", X_StrError());

  printf("V_ClearOverlay: Stack size: %d\n", lua_gettop(L));
}

void V_ClearOverlay(void) {
  lua_State *L = X_GetState();

  lua_getglobal(L, X_NAMESPACE);
  lua_getfield(L, -1, "hud");
  lua_remove(L, -2);
  lua_getfield(L, -1, "clear");
  lua_remove(L, -2);
  lua_getglobal(L, X_NAMESPACE);
  lua_getfield(L, -1, "hud");
  lua_remove(L, -2);
  if (lua_pcall(L, 1, 0, 0) != LUA_OK)
    I_Error("Error clearing overlay: %s", X_StrError());

  printf("V_ClearOverlay: Stack size: %d\n", lua_gettop(L));
}

unsigned int* V_GetOverlayPixels(void) {
  unsigned int *overlay_pixels;
  lua_State *L = X_GetState();

  lua_getglobal(L, X_NAMESPACE);
  lua_getfield(L, -1, "hud");
  lua_remove(L, -2);
  lua_getfield(L, -1, "get_pixels");
  lua_remove(L, -2);
  lua_getglobal(L, X_NAMESPACE);
  lua_getfield(L, -1, "hud");
  lua_remove(L, -2);
  if (lua_pcall(L, 1, 1, 0) != LUA_OK)
    I_Error("Error getting overlay data: %s", X_StrError());
  if (!lua_islightuserdata(L, -1))
    I_Error("xf.hud.get_pixels did not return light userdata");

  overlay_pixels = lua_touserdata(L, -1);
  lua_pop(L, 1);
  printf("V_GetOverlayPixels: Stack size: %d\n", lua_gettop(L));
}

void V_MarkOverlayDirty(void) {
  lua_State *L = X_GetState();

  lua_getglobal(L, X_NAMESPACE);
  lua_getfield(L, -1, "hud");
  lua_remove(L, -2);
  lua_getfield(L, -1, "mark_dirty");
  lua_remove(L, -2);
  lua_getglobal(L, X_NAMESPACE);
  lua_getfield(L, -1, "hud");
  lua_remove(L, -2);
  if (lua_pcall(L, 1, 0, 0) != LUA_OK)
    I_Error("Error marking overlay dirty: %s", X_StrError());

  printf("V_MarkOverlayDirty: Stack size: %d\n", lua_gettop(L));
}

/* vi: set et ts=2 sw=2: */

