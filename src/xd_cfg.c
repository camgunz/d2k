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

#include "d_cfg.h"
#include "x_intern.h"
#include "x_main.h"

static int XD_ConfigGetBool(lua_State *L) {
  const char *section_name = luaL_checkstring(L, 1);
  const char *value_name = luaL_checkstring(L, 2);
  bool success;
  bool value;
  
  success = D_ConfigSafeGetBool(section_name, value_name, &value);

  lua_pushboolean(L, success);

  if (success) {
    lua_pushboolean(L, value);
    return 2;
  }

  return 1;
}

static int XD_ConfigSetBool(lua_State *L) {
  const char *section_name = luaL_checkstring(L, 1);
  const char *value_name = luaL_checkstring(L, 2);
  bool value = luaL_checkint(L, 3);
  bool success = D_ConfigSetBool(section_name, value_name, value);

  lua_pushboolean(L, success);

  return 1;
}

static int XD_ConfigGetStr(lua_State *L) {
  const char *section_name = luaL_checkstring(L, 1);
  const char *value_name = luaL_checkstring(L, 2);
  bool success;
  char *value;
  
  success = D_ConfigSafeGetStr(section_name, value_name, &value);

  lua_pushboolean(L, success);

  if (success) {
    lua_pushstring(L, value);
    return 2;
  }

  return 1;
}

static int XD_ConfigSetStr(lua_State *L) {
  const char *section_name = luaL_checkstring(L, 1);
  const char *value_name = luaL_checkstring(L, 2);
  const char *value = luaL_checkstring(L, 3);
  bool success = D_ConfigSetStr(section_name, value_name, value);

  lua_pushboolean(L, success);

  return 1;
}

static int XD_ConfigGetInt(lua_State *L) {
  const char *section_name = luaL_checkstring(L, 1);
  const char *value_name = luaL_checkstring(L, 2);
  bool success;
  int64_t value;
  
  success = D_ConfigSafeGetInt(section_name, value_name, &value);

  lua_pushboolean(L, success);

  if (success) {
    lua_pushinteger(L, value);
    return 2;
  }

  return 1;
}

static int XD_ConfigSetInt(lua_State *L) {
  const char *section_name = luaL_checkstring(L, 1);
  const char *value_name = luaL_checkstring(L, 2);
  int value = luaL_checkint(L, 3);
  bool success = D_ConfigSetInt(section_name, value_name, value);

  lua_pushboolean(L, success);

  return 1;
}

static int XD_ConfigGetDec(lua_State *L) {
  const char *section_name = luaL_checkstring(L, 1);
  const char *value_name = luaL_checkstring(L, 2);
  bool success;
  double value;
  
  success = D_ConfigSafeGetDec(section_name, value_name, &value);

  lua_pushboolean(L, success);

  if (success) {
    lua_pushnumber(L, value);
    return 2;
  }

  return 1;
}

static int XD_ConfigSetDec(lua_State *L) {
  const char *section_name = luaL_checkstring(L, 1);
  const char *value_name = luaL_checkstring(L, 2);
  double value = luaL_checknumber(L, 3);
  bool success = D_ConfigSetDec(section_name, value_name, value);

  lua_pushboolean(L, success);

  return 1;
}

static int XD_ConfigWrite(lua_State *L) {
  const char *config_contents = luaL_checkstring(L, 1);
  bool success = D_ConfigWrite(config_contents);

  lua_pushboolean(L, success);

  return 1;
}

void XD_ConfigRegisterInterface(void) {
  X_RegisterObjects("Config", 9,
    "get_boolean", X_FUNCTION, XD_ConfigGetBool,
    "set_boolean", X_FUNCTION, XD_ConfigSetBool,
    "get_string",  X_FUNCTION, XD_ConfigGetStr,
    "set_string",  X_FUNCTION, XD_ConfigSetStr,
    "get_integer", X_FUNCTION, XD_ConfigGetInt,
    "set_integer", X_FUNCTION, XD_ConfigSetInt,
    "get_decimal", X_FUNCTION, XD_ConfigSetDec,
    "set_decimal", X_FUNCTION, XD_ConfigGetDec,
    "write",       X_FUNCTION, XD_ConfigWrite
  );
}

/* vi: set et ts=2 sw=2: */

