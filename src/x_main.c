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

#include <girepository.h>

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

#if LUA_VERSION_NUM > 501
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
#if LUA_VERSION_NUM > 501
  lua_createtable(L, g_hash_table_size(x_funcs), 0);
  lua_setglobal(L, X_NAMESPACE);
  lua_getglobal(L, X_NAMESPACE);
  g_hash_table_foreach(x_funcs, load_x_func, NULL);
#else
  GHashTableIter iter;
  gpointer key;
  gpointer value;
  int func_index = 0;

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
#endif
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

lua_State* X_GetState(void) {
  return L;
}

bool X_RunCode(const char *code) {
  bool errors = luaL_dostring(L, code);

  if (errors) {
    set_error(lua_tostring(L, -1));
    return false;
  }

  return true;
}

bool X_CallFunc(const char *object, const char *fname, int arg_count,
                                                       int res_count, ...) {
  va_list args;
  bool have_type = false;
  x_type_e arg_type = X_NONE;
  int bool_arg;
  int args_remaining = arg_count;

  if (arg_count < 0)
    I_Error("X_CallFunc: arg_count < 0");

  if (res_count < 0)
    I_Error("X_CallFunc: res_count < 0");

  if (fname == NULL)
    I_Error("X_CallFunc: fname == NULL");

  lua_getglobal(L, X_NAMESPACE);
  if (object) {
    lua_getfield(L, -1, object);
    lua_remove(L, -2);
  }
  lua_getfield(L, -1, fname);
  lua_remove(L, -2);
  if (object) {
    lua_getglobal(L, X_NAMESPACE);
    lua_getfield(L, -1, object);
    lua_remove(L, -2);
  }

  if (object)
    args_remaining--;

  va_start(args, res_count);
  if (args_remaining > 0) {
    if (!have_type) {
      arg_type = va_arg(args, x_type_e);
      if (arg_type == X_NIL)
        lua_pushnil(L);
    }
    else if (arg_type) {
      switch(arg_type) {
        case X_NIL:
        break;
        case X_BOOL:
          bool_arg = va_arg(args, int);

          if (bool_arg)
            lua_pushboolean(L, true);
          else
            lua_pushboolean(L, false);
        break;
        case X_LUDATA:
          lua_pushlightuserdata(L, va_arg(args, void *));
        break;
        case X_NUM:
          lua_pushnumber(L, va_arg(args, lua_Number));
        break;
        case X_STR:
          lua_pushstring(L, va_arg(args, const char *));
        break;
        case X_FUNC:
          lua_pushcfunction(L, va_arg(args, lua_CFunction));
        break;
        default:
          I_Error("X_CallFunc: Invalid argument type %d\n", arg_type);
        break;
      }
    }
    else {
      I_Error("X_CallFunc: Invalid argument order");
    }

    args_remaining--;
  }
  va_end(args);

  if (object)
    arg_count++;

  return lua_pcall(L, arg_count, res_count, 0) == 0;
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
  L = luaL_newstate();
  luaL_openlibs(L); /* CG: TODO: Restrict clientside libs */
}

void X_RegisterFunctions(void) {
  X_RegisterFunc("quit", XF_Quit);
  X_RegisterFunc("exit", XF_Quit);

  register_xfuncs();
}

void X_Start(void) {
  bool script_load_failed;
  char *script_folder = M_PathJoin(I_DoomExeDir(), X_FOLDER_NAME);
  char *script_path = M_PathJoin(script_folder, "?.lua");
  char *init_script_file = M_PathJoin(script_folder, X_INIT_SCRIPT_NAME);
  GITypelib *typelib = g_irepository_require(NULL, "GObject", NULL, 0, NULL);

  if (!typelib) {
    char *typelib_folder = M_PathJoin(I_DoomExeDir(), X_TYPELIB_FOLDER_NAME);

    if (!M_IsFolder(typelib_folder)) {
      I_Error(
        "X_Start: Could not locate typelib folder (%s) for scripting",
        typelib_folder
      );
    }

    g_irepository_prepend_search_path(typelib_folder);
    typelib = g_irepository_require(NULL, "GObject", NULL, 0, NULL);
  }

  if (!typelib) {
    GSList *typelib_search_paths = g_irepository_get_search_path();

    puts("Typelib search path:");
    while (typelib_search_paths) {
      printf("  %s\n", (char *)typelib_search_paths->data);
      typelib_search_paths = g_slist_next(typelib_search_paths);
    }

    I_Error("X_Start: Could not locate typelib files for scripting");
  }

  if (!M_IsFolder(script_folder))
    I_Error("Script folder [%s] is missing", script_folder);

  if (!M_IsFile(init_script_file))
    I_Error("Initialization script [%s] is missing", init_script_file);

  lua_pushstring(L, script_path);
  lua_setfield(L, -2, "script_path");
  lua_pop(L, 1);

  script_load_failed = luaL_dofile(L, init_script_file);

  if (script_load_failed) {
    I_Error("Error loading initialization script [%s]: %s",
      init_script_file, X_StrError()
    );
  }

  free(script_folder);
  free(script_path);
  free(init_script_file);
}

/* vi: set et ts=2 sw=2: */

