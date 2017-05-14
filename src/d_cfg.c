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

#include "doomdef.h"
#include "d_cfg.h"
#include "d_msg.h"
#include "i_system.h"
#include "m_file.h"
#include "x_main.h"
#include "x_intern.h"
#include "p_user.h"
#include "g_game.h"

#define DEFAULT_CONFIG_FILE_NAME PACKAGE_TARNAME "_config.lua"

void D_ConfigInit(void) {
  char *config_path = M_PathJoin(I_DoomExeDir(), DEFAULT_CONFIG_FILE_NAME);

  if (!config_path) {
    I_Error("Error joining %s and %s: %s\n",
      I_DoomExeDir(),
      DEFAULT_CONFIG_FILE_NAME,
      M_GetFileError()
    );
  }

  if (!M_IsFile(config_path)) {
    if (!D_ConfigSave()) {
      I_Error("Error writing default config file\n");
    }
  }
  else {
    char *data = NULL;
    size_t size = 0;

    if (!M_ReadFile(config_path, &data, &size)) {
      I_Error("Error reading config file: %s\n", M_GetFileError());
    }

    if (!X_Eval(X_GetState(), data)) {
      I_Error("Error loading config file: %s\n", X_GetError(X_GetState()));
    }

    g_free(data);
  }
}

bool D_ConfigSave(void) {
  char *config_path = M_PathJoin(I_DoomExeDir(), DEFAULT_CONFIG_FILE_NAME);
  char *config_data = NULL;

  if (!config_path) {
    I_Error("Error joining %s and %s: %s\n",
      I_DoomExeDir(),
      DEFAULT_CONFIG_FILE_NAME,
      M_GetFileError()
    );
  }

  if (!X_Call(X_GetState(), "config", "serialize", 0, 1)) {
    I_Error("Error serializing config: %s\n", X_GetError(X_GetState()));
  }

  config_data = X_PopString(X_GetState());

  P_Printf(consoleplayer, "Saving config to %s\n", config_path);

  if (!M_WriteFile(config_path, config_data, strlen(config_data))) {
    D_Msg(MSG_WARN, "Failed to write config: %s\n", M_GetFileError());
    return false;
  }

  P_Printf(consoleplayer, "Config saved\n");

  return true;
}

bool D_ConfigGetBool(const char *path) {
  if (!X_Call(X_GetState(), "config", "get_cvar", 1, 1, X_STRING, path)) {
    I_Error("Error getting cvar \"%s\": %s\n", path, X_GetError(X_GetState()));
  }

  if (!lua_isboolean(X_GetState(), 1)) {
    I_Error("cvar [%s] is not a boolean\n", path);
  }

  return X_PopBoolean(X_GetState());
}

int32_t D_ConfigGetInt(const char *path) {
  if (!X_Call(X_GetState(), "config", "get_cvar", 1, 1, X_STRING, path)) {
    I_Error("Error getting cvar \"%s\": %s\n", path, X_GetError(X_GetState()));
  }

  if (!lua_isnumber(X_GetState(), 1)) {
    I_Error("cvar [%s] is not a number\n", path);
  }

  return X_PopInteger(X_GetState());
}

uint32_t D_ConfigGetUInt(const char *path) {
  if (!X_Call(X_GetState(), "config", "get_cvar", 1, 1, X_STRING, path)) {
    I_Error("Error getting cvar \"%s\": %s\n", path, X_GetError(X_GetState()));
  }

  if (!lua_isnumber(X_GetState(), 1)) {
    I_Error("cvar [%s] is not a number\n", path);
  }

  return X_PopUInteger(X_GetState());
}

double D_ConfigGetFloat(const char *path) {
  if (!X_Call(X_GetState(), "config", "get_cvar", 1, 1, X_STRING, path)) {
    I_Error("Error getting cvar \"%s\": %s\n", path, X_GetError(X_GetState()));
  }

  if (!lua_isnumber(X_GetState(), 1)) {
    I_Error("cvar [%s] is not a number\n", path);
  }

  return X_PopDecimal(X_GetState());
}

const char* D_ConfigGetString(const char *path) {
  if (!X_Call(X_GetState(), "config", "get_cvar", 1, 1, X_STRING, path)) {
    I_Error("Error getting cvar \"%s\": %s\n", path, X_GetError(X_GetState()));
  }

  if (!lua_isstring(X_GetState(), 1)) {
    I_Error("cvar [%s] is not a string\n", path);
  }

  return X_PopString(X_GetState());
}

/* vi: set et ts=2 sw=2: */

