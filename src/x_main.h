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


#ifndef X_MAIN_H__
#define X_MAIN_H__

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#define X_NAMESPACE "d2k"
#define X_FOLDER_NAME "scripts"
#define X_INIT_SCRIPT_NAME "init.lua"

typedef enum {
  X_NONE   = LUA_TNONE,          // ...
  X_NIL    = LUA_TNIL,           // lua_pushnil
  X_BOOL   = LUA_TBOOLEAN,       // lua_pushboolean
  X_LUDATA = LUA_TLIGHTUSERDATA, // lua_pushlightuserdata
  X_NUM    = LUA_TNUMBER,        // lua_pushnumber
  X_STR    = LUA_TSTRING,        // lua_pushstring
  X_TABLE  = LUA_TTABLE,         // ...
  X_FUNC   = LUA_TFUNCTION,      // lua_pushcfunction
  X_UDATA  = LUA_TUSERDATA,      // ...
  X_THREAD = LUA_TTHREAD         // ...
} x_type_e;

typedef struct x_object_s {
  void *pointer;
  char need_unref;
} x_object_t;

void        X_Init(void);
const char* X_StrError(void);
const char* X_GetError(void);
lua_State*  X_GetState(void);
bool        X_RunCode(const char *code);
bool        X_CallFunc(const char *object, const char *fname,
                       int arg_count, int res_count, ...);
void        X_RegisterFunc(const char *name, lua_CFunction func);

#endif

/* vi: set et ts=2 sw=2: */

