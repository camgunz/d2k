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
#include "x_intern.h"
#include "x_main.h"

/* CG: [TODO] Add a member for metatable, in the case of light userdata */
typedef struct x_object_s {
  x_type_e type;
  union x_object_data_u {
    bool           boolean;
    void          *light_userdata;
    lua_Integer    integer;
    lua_Unsigned   uinteger;
    lua_Number     decimal;
    char          *string;
    lua_CFunction  function;
  } as;
} x_object_t;

static lua_State  *x_main_interpreter = NULL;
static GHashTable *x_types = NULL;
static GHashTable *x_global_scope = NULL;
static GHashTable *x_scopes = NULL;
static bool        x_initialized = false;
static bool        x_started = false;

static gboolean x_objects_equal(gconstpointer a, gconstpointer b) {
  return a == b;
}

static bool push_x_object(x_engine_t xe, x_object_t *x_obj) {
  lua_State *L = (lua_State *)xe;

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
    if (luaL_callmeta(L, 1, "__tostring") && lua_type(L, -1) == LUA_TSTRING) {
      return 1;
    }

    error_message = lua_pushfstring(L, "(error object is a %s value)",
      luaL_typename(L, 1)
    );
  }

  luaL_traceback(L, L, error_message, 1);

  return 1;
}

static char* get_script_folder(void) {
  char *script_folder = M_PathJoin(I_DoomExeDir(), X_FOLDER_NAME);

  if (!M_IsFolder(script_folder)) {
    free(script_folder);
    script_folder = strdup(X_FOLDER_NAME);

    if (!M_IsFolder(script_folder)) {
      return NULL;
    }
  }

  return script_folder;
}

void X_Init(void) {
  char      *script_search_path;
  char      *script_folder = get_script_folder();
  GITypelib *typelib;

  if (!script_folder) {
    I_Error("Script folder [%s] is missing", X_FOLDER_NAME);
  }

  x_main_interpreter = X_NewState();
  luaL_openlibs(x_main_interpreter); /* CG: [TODO] Restrict clientside libs */

  x_types = g_hash_table_new(g_str_hash, x_objects_equal);
  x_global_scope = g_hash_table_new(g_str_hash, x_objects_equal);
  x_scopes = g_hash_table_new(g_str_hash, x_objects_equal);

  typelib = g_irepository_require(NULL, "GObject", NULL, 0, NULL);

  if (!typelib) {
    char *typelib_folder = M_PathJoin(I_DoomExeDir(), X_TYPELIB_FOLDER_NAME);

    if (!M_IsFolder(typelib_folder)) {
      I_Error(
        "X_Init: Could not locate typelib folder (%s) for scripting",
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

    I_Error("X_Init: Could not locate typelib files for scripting");
  }

  script_search_path = M_PathJoin(script_folder, "?.lua");

  lua_newtable(x_main_interpreter);
  lua_setglobal(x_main_interpreter, X_NAMESPACE);

  lua_getglobal(x_main_interpreter, X_NAMESPACE);
  lua_pushstring(x_main_interpreter, script_folder);
  lua_setfield(x_main_interpreter, 1, "script_folder");
  lua_pop(x_main_interpreter, 1);

  lua_getglobal(x_main_interpreter, X_NAMESPACE);
  lua_pushstring(x_main_interpreter, script_search_path);
  lua_setfield(x_main_interpreter, 1, "script_search_path");
  lua_pop(x_main_interpreter, 1);

  if (!X_EvalScript(x_main_interpreter, X_INIT_SCRIPT_NAME)) {
    I_Error("X_Init: Error loading initialization script: %s",
      X_GetError(x_main_interpreter)
    );
  }

  free(script_folder);
  free(script_search_path);

  x_initialized = true;
}

void X_Start(void) {
  if (!X_EvalScript(X_GetState(), X_START_SCRIPT_NAME)) {
    I_Error("X_Start: Error running start script: %s",
      X_GetError(X_GetState())
    );
  }

  x_started = true;
}

bool X_Available(void) {
  return x_initialized;
}

bool X_Started(void) {
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

  if (g_hash_table_contains(x_types, (char *)type_name)) {
    I_Error("X_RegisterType: Type '%s' already registered", type_name);
  }

  g_hash_table_insert(x_types, (char *)type_name, methods);
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
    bool already_existed;

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

    already_existed = g_hash_table_contains(scope, name);

    if (already_existed) {
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
    else {
      g_hash_table_insert(scope, name, x_obj);
    }

  }

  va_end(args);
}

x_engine_t X_GetState(void) {
  if (!X_Available())
    I_Error("X_GetState: scripting is unavailable");

  return x_main_interpreter;
}

x_engine_t X_NewState(void) {
  return luaL_newstate();
}

x_engine_t X_NewRestrictedState(void) {
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

void X_ExposeInterfaces(x_engine_t xe) {
  lua_State *L = (lua_State *)xe;
  GHashTableIter iter;
  gpointer key;
  gpointer value;

  if (!L) {
    L = x_main_interpreter;
  }

  if (L != x_main_interpreter) {
    lua_createtable(L, 0, g_hash_table_size(x_global_scope));
    lua_setglobal(L, X_NAMESPACE);
  }

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
char* X_GetError(x_engine_t xe) {
  lua_State *L = (lua_State *)xe;
  const char *lua_error_msg = lua_tostring(L, -1);
  char *error_message;

  if (!lua_error_msg)
    return strdup("");

  error_message = strdup(lua_error_msg);
  lua_pop(L, -1);

  return error_message;
}

bool X_Eval(x_engine_t xe, const char *code) {
  lua_State *L = (lua_State *)xe;
  return !luaL_dostring(L, code);
}

bool X_Call(x_engine_t xe, const char *object, const char *fname,
                          int arg_count, int res_count, ...) {
  lua_State *L = (lua_State *)xe;
  va_list args;
  int args_remaining = arg_count;
  int error_handler_index = X_GetStackSize(L) - arg_count;
  int status;

  if (arg_count < 0)
    I_Error("X_Call: arg_count < 0");

  if (res_count < 0)
    I_Error("X_Call: res_count < 0");

  if (fname == NULL)
    I_Error("X_Call: fname == NULL");

  lua_pushcfunction(L, error_handler);
  lua_insert(L, error_handler_index);

  lua_getglobal(L, X_NAMESPACE);

  if (object) {
    lua_getfield(L, -1, object);
    if (!lua_istable(L, -1)) {
      I_Error("X_Call: %s.%s not found\n", X_NAMESPACE, object);
    }

    lua_remove(L, -2);
  }

  lua_getfield(L, -1, fname);
  if (!lua_isfunction(L, -1)) {
    if (object) {
      I_Error("X_Call: %s.%s.%s not found\n", X_NAMESPACE, object, fname);
    }
    else {
      I_Error("X_Call: %s.%s not found\n", X_NAMESPACE, fname);
    }
  }

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
        I_Error("X_Call: Invalid argument type %d\n", arg_type);
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

bool X_EvalFile(x_engine_t xe, const char *file_path) {
  if (!M_IsFile(file_path)) {
    I_Error("Script [%s] is missing", file_path);
  }

  return !luaL_dofile(xe, file_path);
}

bool X_EvalScript(x_engine_t xe, const char *script_name) {
  char *script_folder = get_script_folder();
  char *script_path;
  bool  success;

  if (!script_folder) {
    I_Error("Script folder [%s] is missing", X_FOLDER_NAME);
  }

  script_path = M_PathJoin(script_folder, script_name);

  free(script_folder);

  success = X_EvalFile(xe, script_path);

  free(script_path);

  return success;
}

int X_GetStackSize(x_engine_t xe) {
  lua_State *L = (lua_State *)xe;
  return lua_gettop(L);
}

char* X_ToString(x_engine_t xe, int index) {
  lua_State *L = (lua_State *)xe;
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
#if LUA_VERSION_NUM >= 503
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
#else
    g_string_printf(s, "<function>");
#endif
  }
#if LUA_VERSION_NUM >= 503
  else if (lua_isinteger(L, index)) {
#ifdef _WIN32
    // g_string_printf(s, "%I64d", lua_tointeger(L, index));
    g_string_printf(s, LUA_INTEGER_FMT, lua_tointeger(L, index));
#else
    // g_string_printf(s, "%lld", lua_tointeger(L, index));
    g_string_printf(s, LUA_INTEGER_FMT, lua_tointeger(L, index));
#endif
  }
#endif
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

void X_PrintStack(x_engine_t xe) {
  lua_State *L = (lua_State *)xe;
  int stack_size = X_GetStackSize(L);

  for (int i = -1; -i <= stack_size; i--) {
    char *stack_member = X_ToString(L, i);

    if (stack_member) {
      printf("%s\n", stack_member);
      free(stack_member);
    }
  }
}

void X_PopStackMembers(x_engine_t xe, int count) {
  lua_State *L = (lua_State *)xe;
  lua_pop(L, count);
}

void X_RunGC(x_engine_t xe) {
  lua_State *L = (lua_State *)xe;
  lua_gc(L, LUA_GCCOLLECT, 0);
}

bool X_PopBoolean(x_engine_t xe) {
  lua_State *L = (lua_State *)xe;
  bool value = lua_toboolean(L, -1);

  lua_pop(L, 1);

  return value;
}

int32_t X_PopInteger(x_engine_t xe) {
  lua_State *L = (lua_State *)xe;
  int32_t value = lua_toboolean(L, -1);

  lua_pop(L, 1);

  return value;
}

uint32_t X_PopUInteger(x_engine_t xe) {
  lua_State *L = (lua_State *)xe;
  uint32_t value = (uint32_t)lua_tointeger(L, -1);

  lua_pop(L, 1);

  return value;
}

double X_PopDecimal(x_engine_t xe) {
  lua_State *L = (lua_State *)xe;
  double value = lua_tonumber(L, -1);

  lua_pop(L, 1);

  return value;
}

char* X_PopString(x_engine_t xe) {
  lua_State *L = (lua_State *)xe;
  const char* value = lua_tostring(L, -1);

  lua_pop(L, 1);

  return (char *)value;
}

void* X_PopUserdata(x_engine_t xe) {
  lua_State *L = (lua_State *)xe;
  void *value = lua_touserdata(L, -1);

  lua_pop(L, 1);

  return value;
}


/* vi: set et ts=2 sw=2: */

