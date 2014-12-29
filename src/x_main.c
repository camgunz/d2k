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

#include "d_event.h"
#include "c_main.h"
#include "i_main.h"
#include "i_system.h"
#include "lprintf.h"
#include "m_file.h"
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

/*
 * CG: This worked for Lua 5.2, not so much for 5.1
 */

#if 0
static void load_x_func(gpointer name, gpointer func, gpointer userdata) {
  luaL_Reg funcs[] = {
    {name, func},
    {NULL, NULL}
  };
  luaL_setfuncs(L, funcs, 0);
}
#endif

static void set_error(const char *message) {
  if (error_message)
    free(error_message);

  error_message = strdup(message);
}

static void register_xfuncs(void) {
  GHashTableIter iter;
  gpointer key;
  gpointer value;
  int func_index = 0;

  /*
   * CG: This worked for Lua 5.2, not so much for 5.1
   */
#if 0
  lua_createtable(L, g_hash_table_size(x_funcs), 0);
  lua_setglobal(L, X_NAMESPACE);
  lua_getglobal(L, X_NAMESPACE);
  g_hash_table_foreach(x_funcs, load_x_func, NULL);
#endif

  g_hash_table_iter_init(&iter, x_funcs);

  luaL_Reg *library = calloc(
    g_hash_table_size(x_funcs) + 1, sizeof(luaL_Reg)
  );

  while (g_hash_table_iter_next(&iter, &key, &value)) {
    const char *name = key;
    lua_CFunction func = value;

    library[func_index].name = name;
    library[func_index].func = func;

    func_index++;
  }

  library[func_index].name = NULL;
  library[func_index].func = NULL;

  luaL_register(L, X_NAMESPACE, library);
}

const char* X_StrError(void) {
  set_error(lua_tostring(L, -1));

  return error_message;
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

lua_State* X_GetState(void) {
  return L;
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
  bool script_load_failed;
  char *script_folder = M_PathJoin(I_DoomExeDir(), X_FOLDER_NAME);
  char *script_path = M_PathJoin(script_folder, "?.lua");
  char *init_script_file = M_PathJoin(script_folder, X_INIT_SCRIPT_NAME);

  if (!M_IsFolder(script_folder))
    I_Error("Script folder [%s] is missing", script_folder);

  if (!M_IsFile(init_script_file))
    I_Error("Initialization script [%s] is missing", init_script_file);

  X_RegisterFunc("quit", XF_Quit);
  X_RegisterFunc("exit", XF_Quit);

  L = luaL_newstate();
  luaL_openlibs(L);

  atexit(X_Close);

  register_xfuncs();

  lua_pushstring(L, script_path);
  lua_setfield(L, -2, "script_path");
  lua_pop(L, 1);

  script_load_failed = luaL_dofile(L, init_script_file);

  if (script_load_failed) {
    set_error(lua_tostring(L, -1));
    I_Error("Error loading initialization script [%s]: %s",
      init_script_file, X_GetError()
    );
  }

  free(script_folder);
  free(init_script_file);
}

/* vi: set et ts=2 sw=2: */

