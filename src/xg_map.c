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

#include "g_map.h"
#include "x_main.h"

int XG_MapLoadPrevious(lua_State *L) {
  bool success = G_MapLoadPrevious();

  lua_pushboolean(L, success);

  return 1;
}

int XG_MapLoadNext(lua_State *L) {
  bool success = G_MapLoadNext();

  lua_pushboolean(L, success);

  return 1;
}

int XG_MapReload(lua_State *L) {
  bool success = G_MapReload();

  lua_pushboolean(L, success);

  return 1;
}

int XG_MapLoad(lua_State *L) {
  int index = luaL_checkinteger(L, -1);
  bool success = G_MapLoad(index);

  lua_pushboolean(L, success);

  return 1;
}

int XG_MapListLength(lua_State *L) {
  size_t len = G_MapListLength();

  lua_pushunsigned(L, len);

  return 1;
}

static int XG_MapListSet(lua_State *L) {
  const char *map_list_contents = luaL_checkstring(L, -1);
  bool success = G_MapListSet(map_list_contents);

  lua_pushboolean(L, success);

  return 1;
}

static int XG_MapListGet(lua_State *L) {
  char *map_list_contents = G_MapListGet();

  if (!map_list_contents) {
    lua_pushnil(L);
  }
  else {
    lua_pushstring(L, map_list_contents);
    free(map_list_contents);
  }

  return 1;
}

void XD_MsgRegisterInterface(void) {
  X_RegisterObjects("MapList", 8,
    "load_previous_map", X_FUNCTION, XG_MapListLoadPreviousMap,
    "load_next_map",     X_FUNCTION, XG_MapListLoadNextMap,
    "reload_map",        X_FUNCTION, XG_MapListReloadMap,
    "load_map",          X_FUNCTION, XG_MapListLoadMap,
    "get_length",        X_FUNCTION, XG_MapListLength,
    "set",               X_FUNCTION, XG_MapListSet,
    "get",               X_FUNCTION, XG_MapListGet,
    "save_to_file",      X_FUNCTION, XG_MapListToFile
  );
}

/* vi: set et ts=2 sw=2: */

