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
#include <glib.h>

#include "d_event.h"
#include "c_main.h"
#include "i_main.h"
#include "lprintf.h"
#include "x_main.h"

static lua_State *L = NULL;
static GHashTable *x_funcs = NULL;
static char *error_message = NULL;

static gboolean x_funcs_equal(gconstpointer a, gconstpointer b) {
  return a == b;
}

static void check_x_funcs(void) {
  if (x_funcs == NULL)
    x_funcs = g_hash_table_new(g_str_hash, x_funcs_equal);
}

static void load_x_func(gpointer name, gpointer func, gpointer userdata) {
  luaL_Reg funcs[] = {
    {name, func},
    {NULL, NULL}
  };
  luaL_setfuncs(L, funcs, 0);
}

static void set_error(const char *message) {
  if (error_message)
    free(error_message);

  error_message = strdup(message);
}

const char* X_GetError(void) {
  if (error_message)
    return error_message;

  return "";
}

bool X_RunCode(const char *code) {
  bool errors = luaL_dostring(L, code);

  if (errors) {
    set_error(lua_tostring(L, -1));
    return false;
  }

  return true;
}

static void X_Close(void) {
  lua_close(L);
}

void X_RegisterFunc(const char *name, lua_CFunction func) {
  bool inserted_new;

  check_x_funcs();

  inserted_new = g_hash_table_insert(x_funcs, (gpointer)name, func);

  if (!inserted_new)
    I_Error("X_RegisterFunc: Function %s already registered", name);
}

int XF_Quit(lua_State *L) {
  I_SafeExit(0);
  return 0;
}

void X_Init(void) {
  X_RegisterFunc("quit", XF_Quit);
  X_RegisterFunc("exit", XF_Quit);

  L = luaL_newstate();
  luaL_openlibs(L);

  atexit(X_Close);

  lua_createtable(L, g_hash_table_size(x_funcs), 0);
  lua_setglobal(L, X_NAMESPACE);
  lua_getglobal(L, X_NAMESPACE);
  g_hash_table_foreach(x_funcs, load_x_func, NULL);
}

/* vi: set et ts=2 sw=2: */

