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

#define X_NAMESPACE "d2k"
#define X_FOLDER_NAME "scripts"
#define X_TYPELIB_FOLDER_NAME "typelibs"
#define X_INIT_SCRIPT_NAME "init.lua"
#define X_CONFIG_SCRIPT_NAME "config.lua"
#define X_START_SCRIPT_NAME "start.lua"
#define X_RegisterObject(sn, n, t, d) X_RegisterObjects(sn, 1, n, t, d)

typedef void* x_engine_t;

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

bool       X_LoadFile(const char *script_name);
void       X_Init(void);
void       X_Start(void);
bool       X_Available(void);
void       X_RegisterType(const char *type_name, unsigned int count, ...);
void       X_RegisterObjects(const char *scope_name, unsigned int count, ...);
x_engine_t X_GetState(void);
x_engine_t X_NewState(void);
x_engine_t X_NewRestrictedState(void);

void       X_ExposeInterfaces(x_engine_t xe);
char*      X_GetError(x_engine_t xe);
bool       X_Eval(x_engine_t xe, const char *code);
bool       X_Call(x_engine_t xe, const char *object, const char *fname,
                                  int arg_count, int res_count, ...);
int        X_GetStackSize(x_engine_t xe);
char*      X_ToString(x_engine_t xe, int index);
void       X_PrintStack(x_engine_t xe);
void       X_PopStackMembers(x_engine_t xe, int count);
void       X_RunGC(x_engine_t xe);

bool       X_PopBoolean(x_engine_t xe);
int32_t    X_PopInteger(x_engine_t xe);
uint32_t   X_PopUInteger(x_engine_t xe);
double     X_PopDecimal(x_engine_t xe);
char*      X_PopString(x_engine_t xe);
void*      X_PopUserdata(x_engine_t xe);

#endif

/* vi: set et ts=2 sw=2: */

