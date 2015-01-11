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
#define X_RegisterObject(sn, n, t, d) X_RegisterObjects(sn, 1, n, t, d)

typedef enum {
  X_NONE = -1,
  X_NIL,
  X_BOOLEAN,
  X_POINTER,
  X_DECIMAL,
  X_INTEGER,
  X_UINTEGER,
  X_STRING,
  X_FUNCTION
} x_type_e;

union x_object_data_u {
  bool           boolean;
  void          *light_userdata;
  lua_Integer    integer;
  lua_Unsigned   uinteger;
  lua_Number     decimal;
  char          *string;
  lua_CFunction  function;
};

/* CG: [TODO] Add a member for metatable, in the case of light userdata */
typedef struct x_object_s {
  x_type_e type;
  union x_object_data_u as;
} x_object_t;

void        X_Init(void);
void        X_Start(void);
void        X_RegisterObjects(const char *scope_name, unsigned int count, ...);
lua_State*  X_GetState(void);
lua_State*  X_NewState(void);
lua_State*  X_NewRestrictedState(void);

void        X_ExposeInterfaces(lua_State *L);
const char* X_GetError(lua_State *L);
bool        X_Eval(lua_State *L, const char *code);
bool        X_Call(lua_State *L, const char *object, const char *fname,
                                 int arg_count, int res_count, ...);

#endif

/* vi: set et ts=2 sw=2: */

