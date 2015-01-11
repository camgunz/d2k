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

#include "d_event.h"
#include "c_main.h"
#include "i_main.h"
#include "i_system.h"
#include "lprintf.h"
#include "m_file.h"
#include "x_main.h"

static lua_State *x_main_interpreter = NULL;
static GHashTable *x_global_scope = NULL;
static GHashTable *x_scopes = NULL;

static gboolean x_objects_equal(gconstpointer a, gconstpointer b) {
  return a == b;
}

static bool push_x_object(lua_State *L, x_object_t *x_obj) {
  switch (x_obj->type) {
    case X_NIL:
      lua_pushnil(L);
    break;
    case X_BOOL:
      lua_pushboolean(L, x_obj->as.boolean);
    break;
    case X_LUDATA:
      lua_pushlightuserdata(L, x_obj->as.light_userdata);
    break;
    case X_NUM:
      lua_pushnumber(L, x_obj->as.number);
    break;
    case X_STR:
      lua_pushstring(L, x_obj->as.string);
    break;
    case X_FUNC:
      lua_pushcfunction(L, x_obj->as.function);
    break;
    default:
      return false;
    break;
  }

  return true;
}

void X_Init(void) {
  x_main_interpreter = X_NewState();
  luaL_openlibs(x_main_interpreter); /* CG: [TODO] Restrict clientside libs */

  x_global_scope = g_hash_table_new(g_str_hash, x_objects_equal);
  x_scopes = g_hash_table_new(g_str_hash, x_objects_equal);
}

void X_Start(void) {
  bool script_load_failed;
  char *script_folder = M_PathJoin(I_DoomExeDir(), X_FOLDER_NAME);
  char *script_path = M_PathJoin(script_folder, "?.lua");
  char *init_script_file = M_PathJoin(script_folder, X_INIT_SCRIPT_NAME);

  if (!M_IsFolder(script_folder))
    I_Error("Script folder [%s] is missing", script_folder);

  if (!M_IsFile(init_script_file))
    I_Error("Initialization script [%s] is missing", init_script_file);

  lua_pushstring(x_main_interpreter, script_path);
  lua_setfield(x_main_interpreter, -2, "script_path");
  lua_pop(x_main_interpreter, 1);

  script_load_failed = luaL_dofile(x_main_interpreter, init_script_file);

  if (script_load_failed) {
    I_Error("X_Start: Error loading initialization script [%s]: %s",
      init_script_file, X_GetError(x_main_interpreter)
    );
  }

  free(script_folder);
  free(script_path);
  free(init_script_file);
}

void X_RegisterObjects(const char *scope_name, unsigned int count, ...) {
  va_list args;
  GHashTable *scope;
  int bool_arg;
  char *name = NULL;
  x_object_t *x_obj;
  bool already_exists;

  if (scope_name != NULL) {
    scope = g_hash_table_lookup(x_scopes, scope_name);

    if (scope == NULL) {
      scope = g_hash_table_new(NULL, NULL);
      g_hash_table_insert(x_scopes, (gpointer)scope_name, scope);
    }
  }
  else {
    scope = x_global_scope;
  }

  va_start(args, count);
  while (count--) {
    x_obj = calloc(1, sizeof(x_object_t));
    x_obj->type = X_NONE;

    if (name == NULL)
      name = va_arg(args, char*);

    if (x_obj->type == X_NONE)
      x_obj->type = va_arg(args, x_type_e);

    switch (x_obj->type) {
      case X_NIL:
      break;
      case X_BOOL:
        bool_arg = va_arg(args, int);
        if (bool_arg)
          x_obj->as.boolean = true;
        else
          x_obj->as.boolean = false;
      break;
      case X_LUDATA:
        x_obj->as.light_userdata = va_arg(args, void *);
      break;
      case X_NUM:
        x_obj->as.number = va_arg(args, lua_Number);
      break;
      case X_STR:
        x_obj->as.string = va_arg(args, char *);
      break;
      case X_FUNC:
        x_obj->as.function = va_arg(args, lua_CFunction);
      break;
      default:
        I_Error("X_RegisterFunctions: Invalid type for %s: %d\n",
          name, x_obj->type
        );
      break;
    }

    already_exists = g_hash_table_insert(scope, name, x_obj);

    if (already_exists) {
      if (scope_name) {
        I_Error("X_RegisterObjects: Object %s.%s already registered",
          scope_name, name
        );
      }
      else {
        I_Error("X_RegisterObjects: Object %s already registered", name);
      }
    }
  }
  va_end(args);
  
}


lua_State* X_GetState(void) {
  return x_main_interpreter;
}

lua_State* X_NewState(void) {
  return luaL_newstate();
}

lua_State* X_NewRestrictedState(void) {
  lua_State *L = luaL_newstate();

  luaL_requiref(L, "base", luaopen_base, true);
  luaL_requiref(L, "package", luaopen_package, true);
  luaL_requiref(L, "coroutine", luaopen_coroutine, true);
  luaL_requiref(L, "string", luaopen_string, true);
  luaL_requiref(L, "table", luaopen_table, true);
  luaL_requiref(L, "math", luaopen_math, true);
  luaL_requiref(L, "bit32", luaopen_bit32, true);

  return L;
}

void X_ExposeInterfaces(lua_State *L) {
  GHashTableIter iter;
  gpointer key;
  gpointer value;

  lua_createtable(L, 0, g_hash_table_size(x_global_scope));
  lua_setglobal(L, X_NAMESPACE);

  lua_getglobal(L, X_NAMESPACE);
  g_hash_table_iter_init(&iter, x_global_scope);
  while (g_hash_table_iter_next(&iter, &key, &value)) {
    const char *name = (const char *)key;
    x_object_t *x_obj = (x_object_t *)value;

    if (!push_x_object(L, x_obj)) {
      I_Error("X_ExposeInterfaces: %s has invalid type %d\n",
        name, x_obj->type
      );
    }

    lua_setfield(L, -2, name);
  }

  g_hash_table_iter_init(&iter, x_scopes);
  while (g_hash_table_iter_next(&iter, &key, &value)) {
    const char *scope_name = (const char *)key;
    GHashTable *scope = (GHashTable *)value;
    GHashTableIter scope_iter;
    gpointer scoped_key;
    gpointer scoped_value;

    lua_createtable(L, 0, g_hash_table_size(scope));
    lua_setfield(L, -2, scope_name);
    lua_getfield(L, -1, scope_name);
    g_hash_table_iter_init(&scope_iter, scope);
    while (g_hash_table_iter_next(&scope_iter, &scoped_key, &scoped_value)) {
      const char *name = (const char *)scoped_key;
      x_object_t *x_obj = (x_object_t *)scoped_value;

      if (!push_x_object(L, x_obj)) {
        I_Error("X_ExposeInterfaces: %s.%s has invalid type %d\n",
          scope_name, name, x_obj->type
        );
      }

      lua_setfield(L, -2, name);
    }
    lua_pop(L, 1);
  }
  lua_pop(L, 1);
}

const char* X_GetError(lua_State *L) {
  return lua_tostring(L, -1);
}

bool X_Eval(lua_State *L, const char *code) {
  bool errors_occurred = luaL_dostring(L, code);

  return !errors_occurred;
}

bool X_Call(lua_State *L, const char *object, const char *fname,
                          int arg_count, int res_count, ...) {
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

/* vi: set et ts=2 sw=2: */

