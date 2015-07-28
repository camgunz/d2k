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
#include "m_file.h"
#include "x_main.h"

static lua_State  *x_main_interpreter = NULL;
static GHashTable *x_types = NULL;
static GHashTable *x_global_scope = NULL;
static GHashTable *x_scopes = NULL;
static bool        x_started = false;

static gboolean x_objects_equal(gconstpointer a, gconstpointer b) {
  return a == b;
}

static bool push_x_object(lua_State *L, x_object_t *x_obj) {
  switch (x_obj->type) {
    case X_NIL:
      lua_pushnil(L);
    break;
    case X_BOOLEAN:
      lua_pushboolean(L, x_obj->as.boolean);
    break;
    case X_POINTER:
      lua_pushlightuserdata(L, x_obj->as.light_userdata);
    break;
    case X_DECIMAL:
      lua_pushnumber(L, x_obj->as.decimal);
    break;
    case X_INTEGER:
      lua_pushinteger(L, x_obj->as.integer);
    break;
    case X_UINTEGER:
      lua_pushinteger(L, (lua_Integer)x_obj->as.uinteger);
    break;
    case X_STRING:
      lua_pushstring(L, x_obj->as.string);
    break;
    case X_FUNCTION:
      lua_pushcfunction(L, x_obj->as.function);
    break;
    default:
      return false;
    break;
  }

  return true;
}

/*
 * CG: This function is from the Lua source code, licensed under the MIT
 *     license.
 */
static int error_handler(lua_State *L) {
  const char *error_message = lua_tostring(L, -1);

  if (!error_message) {
    if (luaL_callmeta(L, 1, "__tostring") && lua_type(L, -1) == LUA_TSTRING)
      return 1;

    error_message = lua_pushfstring(L, "(error object is a %s value)",
      luaL_typename(L, 1)
    );
  }

  luaL_traceback(L, L, error_message, 1);

  return 1;
}

void X_Init(void) {
  x_main_interpreter = X_NewState();
  luaL_openlibs(x_main_interpreter); /* CG: [TODO] Restrict clientside libs */

  x_types = g_hash_table_new(g_str_hash, x_objects_equal);
  x_global_scope = g_hash_table_new(g_str_hash, x_objects_equal);
  x_scopes = g_hash_table_new(g_str_hash, x_objects_equal);
}

void X_Start(void) {
  bool script_load_failed;
  char *init_script_file;
  char *script_search_path;
  char *script_folder = M_PathJoin(I_DoomExeDir(), X_FOLDER_NAME);
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

  if (!M_IsFolder(script_folder)) {
    free(script_folder);
    script_folder = strdup(X_FOLDER_NAME);

    if (!M_IsFolder(script_folder))
      I_Error("Script folder [%s] is missing", script_folder);
  }

  init_script_file = M_PathJoin(script_folder, X_INIT_SCRIPT_NAME);
  script_search_path = M_PathJoin(script_folder, "?.lua");

  if (!M_IsFile(init_script_file))
    I_Error("Initialization script [%s] is missing", init_script_file);

  lua_getglobal(x_main_interpreter, X_NAMESPACE);
  lua_pushstring(x_main_interpreter, script_folder);
  lua_setfield(x_main_interpreter, -2, "script_folder");
  lua_pop(x_main_interpreter, 1);

  lua_getglobal(x_main_interpreter, X_NAMESPACE);
  lua_pushstring(x_main_interpreter, script_search_path);
  lua_setfield(x_main_interpreter, -2, "script_search_path");
  lua_pop(x_main_interpreter, 1);

  script_load_failed = luaL_dofile(x_main_interpreter, init_script_file);

  if (script_load_failed) {
    I_Error("X_Start: Error loading initialization script [%s]: %s",
      init_script_file, X_GetError(x_main_interpreter)
    );
  }

  free(script_folder);
  free(script_search_path);
  free(init_script_file);

  x_started = true;
}

bool X_Available(void) {
  return x_started;
}

void X_RegisterType(const char *type_name, unsigned int count, ...) {
  luaL_Reg *methods;
  va_list args;

  methods = calloc(count + 1, sizeof(luaL_Reg));

  if (!methods)
    I_Error("X_RegisterType: Allocating methods for %s failed", type_name);

  va_start(args, count);
  for (unsigned int i = 0; i < count; i++) {
    methods[i].name = va_arg(args, char *);
    methods[i].func = va_arg(args, lua_CFunction);
  }
  methods[count].name = NULL;
  methods[count].func = NULL;
  va_end(args);

  if (!g_hash_table_insert(x_types, (char *)type_name, methods))
    I_Error("X_RegisterType: Type '%s' already registered", type_name);
}

void X_RegisterObjects(const char *scope_name, unsigned int count, ...) {
  va_list args;
  GHashTable *scope;

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
    char *name;
    x_object_t *x_obj;
    int bool_arg;
    bool did_not_exist;

    x_obj = calloc(1, sizeof(x_object_t));

    if (!x_obj)
      I_Error("X_RegisterObjects: Error allocating new x_object_t");

    name = va_arg(args, char *);
    x_obj->type = va_arg(args, x_type_e);

    switch (x_obj->type) {
      case X_NIL:
      break;
      case X_BOOLEAN:
        bool_arg = va_arg(args, int);
        if (bool_arg)
          x_obj->as.boolean = true;
        else
          x_obj->as.boolean = false;
      break;
      case X_POINTER:
        x_obj->as.light_userdata = va_arg(args, void *);
      break;
      case X_DECIMAL:
        x_obj->as.decimal = va_arg(args, double);
      break;
      case X_INTEGER:
        x_obj->as.integer = va_arg(args, int);
      break;
      case X_UINTEGER:
        x_obj->as.uinteger = va_arg(args, unsigned int);
      break;
      case X_STRING:
        x_obj->as.string = va_arg(args, char *);
      break;
      case X_FUNCTION:
        x_obj->as.function = va_arg(args, lua_CFunction);
      break;
      default:
        I_Error("X_RegisterObjects: Invalid type for %s: %d\n",
          name, x_obj->type
        );
      break;
    }

    did_not_exist = g_hash_table_insert(scope, name, x_obj);

    if (!did_not_exist) {
      if (scope_name && name) {
        I_Error("X_RegisterObjects: Object %s.%s already registered",
          scope_name, name
        );
      }
      else if (scope_name) {
        I_Error("X_RegisterObjects: Scope %s already registered", scope_name);
      }
      else if (name) {
        I_Error("X_RegisterObjects: Object %s already registered", name);
      }
      else {
        I_Error("X_RegisterObjects: Object already registered");
      }
    }
  }

  va_end(args);
}

lua_State* X_GetState(void) {
  if (!X_Available())
    I_Error("X_GetState: scripting is unavailable");

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

  if (!L)
    L = x_main_interpreter;

  lua_createtable(L, 0, g_hash_table_size(x_global_scope));
  lua_setglobal(L, X_NAMESPACE);
  lua_getglobal(L, X_NAMESPACE);

  g_hash_table_iter_init(&iter, x_types);
  while (g_hash_table_iter_next(&iter, &key, &value)) {
    const char *type_name = (const char *)key;
    const struct luaL_Reg *methods = (luaL_Reg *)value;

    luaL_newmetatable(L, type_name);
    luaL_setfuncs(L, methods, 0);
    lua_pushvalue(L, -1);
    lua_setfield(L, -1, "__index");
    lua_setfield(L, -2, type_name);
  }

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
}

/*
 * CG: This function is from the Lua source code, licensed under the MIT
 *     license.
 */
char* X_GetError(lua_State *L) {
  const char *lua_error_msg = lua_tostring(L, -1);
  char *error_message;

  if (!lua_error_msg)
    return strdup("");

  error_message = strdup(lua_error_msg);
  lua_pop(L, -1);

  return error_message;
}

bool X_Eval(lua_State *L, const char *code) {
  return !luaL_dostring(L, code);
}

bool X_Call(lua_State *L, const char *object, const char *fname,
                          int arg_count, int res_count, ...) {
  va_list args;
  int args_remaining = arg_count;
  int error_handler_index = X_GetStackSize(L) - arg_count;
  int status;

  if (arg_count < 0)
    I_Error("X_CallFunc: arg_count < 0");

  if (res_count < 0)
    I_Error("X_CallFunc: res_count < 0");

  if (fname == NULL)
    I_Error("X_CallFunc: fname == NULL");

  lua_pushcfunction(L, error_handler);
  lua_insert(L, error_handler_index);

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

  va_start(args, res_count);
  while (args_remaining-- > 0) {
    int bool_arg;
    x_type_e arg_type = va_arg(args, x_type_e);

    switch(arg_type) {
      case X_NIL:
      break;
      case X_BOOLEAN:
        bool_arg = va_arg(args, int);

        if (bool_arg)
          lua_pushboolean(L, true);
        else
          lua_pushboolean(L, false);
      break;
      case X_POINTER:
        lua_pushlightuserdata(L, va_arg(args, void *));
      break;
      case X_DECIMAL:
        lua_pushnumber(L, va_arg(args, double));
      break;
      case X_INTEGER:
        lua_pushinteger(L, va_arg(args, int));
      break;
      case X_UINTEGER:
        lua_pushinteger(L, va_arg(args, int));
      break;
      case X_STRING:
        lua_pushstring(L, va_arg(args, const char *));
      break;
      case X_FUNCTION:
        lua_pushcfunction(L, va_arg(args, lua_CFunction));
      break;
      default:
        I_Error("X_CallFunc: Invalid argument type %d\n", arg_type);
      break;
    }
  }
  va_end(args);

  if (object)
    arg_count++;

  status = lua_pcall(L, arg_count, res_count, 0);

  lua_remove(L, error_handler_index);

  return status == 0;
}

int X_GetStackSize(lua_State *L) {
  return lua_gettop(L);
}

char* X_ToString(lua_State *L, int index) {
  GString *s = g_string_new("");
  char *out;

  if (lua_isboolean(L, index)) {
    bool bool_value = lua_toboolean(L, index);

    if (bool_value)
      g_string_printf(s, "true");
    else
      g_string_printf(s, "false");
  }
  else if (lua_iscfunction(L, index) || lua_isfunction(L, index)) {
    lua_Debug debug_info;
    int success;

    memset(&debug_info, 0, sizeof(lua_Debug));

    lua_rotate(L, index, index);
    success = lua_getinfo(L, ">n", &debug_info);

    if (!success)
      I_Error("X_ToString: Error getting debug information");

    if (debug_info.what && *debug_info.what)
      g_string_printf(s, "%s ", debug_info.what);
    else
      g_string_printf(s, "[unknown type] ");

    if (debug_info.namewhat && *debug_info.namewhat)
      g_string_append_printf(s, "%s ", debug_info.namewhat);
    else
      g_string_append_printf(s, "[unknown name type] ");

    if (debug_info.name && *debug_info.name)
      g_string_append_printf(s, "%s", debug_info.name);
    else
      g_string_append_printf(s, "[unknown function name]");

    lua_rotate(L, index, -index);
  }
  else if (lua_isinteger(L, index)) {
#ifdef _WIN32
    g_string_printf(s, "%I64d", lua_tointeger(L, index));
#else
    g_string_printf(s, "%lld", lua_tointeger(L, index));
#endif
  }
  else if (lua_islightuserdata(L, index)) {
    if (luaL_callmeta(L, index, "__tostring") &&
        lua_type(L, -1) == LUA_TSTRING) {
      g_string_printf(s, "%s", lua_tostring(L, 1));
      lua_pop(L, 1);
    }
    else {
      g_string_printf(s, "Light userdata: %p", lua_topointer(L, index));
    }
  }
  else if (lua_isnoneornil(L, index)) {
    g_string_printf(s, "nil");
  }
  else if (lua_isnumber(L, index)) {
    g_string_printf(s, LUA_NUMBER_FMT, lua_tonumber(L, index));
  }
  else if (lua_isstring(L, index)) {
    g_string_printf(s, "%s", lua_tostring(L, index));
  }
  else if (lua_istable(L, index)) {
    if (luaL_callmeta(L, index, "__tostring") &&
        lua_type(L, -1) == LUA_TSTRING) {
      g_string_printf(s, "%s", lua_tostring(L, 1));
      lua_pop(L, 1);
    }
    else {
      g_string_printf(s, "Table: %p", lua_topointer(L, index));
    }
  }
  else if (lua_isthread(L, index)) {
    if (luaL_callmeta(L, index, "__tostring") &&
        lua_type(L, -1) == LUA_TSTRING) {
      g_string_printf(s, "%s", lua_tostring(L, 1));
      lua_pop(L, 1);
    }
    else {
      g_string_printf(s, "Thread: %p", lua_topointer(L, index));
    }
  }
  else if (lua_isuserdata(L, index)) {
    if (luaL_callmeta(L, index, "__tostring") &&
        lua_type(L, -1) == LUA_TSTRING) {
      g_string_printf(s, "%s", lua_tostring(L, 1));
      lua_pop(L, 1);
    }
    else {
      g_string_printf(s, "Userdata: %p", lua_topointer(L, index));
    }
  }
  else {
    int type = lua_type(L, index);

    I_Error("X_ToString: Unknown type %s [%d]\n", lua_typename(L, type), type);
  }

  out = strdup(s->str);

  g_string_free(s, true);

  return out;
}

void X_PrintStack(lua_State *L) {
  int stack_size = X_GetStackSize(L);

  for (int i = -1; -i <= stack_size; i--) {
    char *stack_member = X_ToString(L, i);

    if (stack_member) {
      printf("%s\n", stack_member);
      free(stack_member);
    }
  }
}

void X_PopStackMembers(lua_State *L, int count) {
  lua_pop(L, count);
}

/* vi: set et ts=2 sw=2: */

