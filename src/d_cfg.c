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

#include "d_cfg.h"
#include "d_msg.h"
#include "i_system.h"
#include "m_file.h"
#include "x_main.h"
#include "x_intern.h"

#define DEFAULT_CONFIG_FILE_NAME PACKAGE_TARNAME "_config.lua"

void D_ConfigInit(void) {
  char *config_path;
  bool success;
  
  if (!X_LoadFile(X_CONFIG_SCRIPT_NAME)) {
    I_Error("D_ConfigInit: Error loading configuration script: %s",
      X_GetError(X_GetState())
    );
  }

  config_path = M_PathJoin(I_DoomExeDir(), DEFAULT_CONFIG_FILE_NAME);

  if (!config_path) {
    I_Error("Error joining %s and %s: %s\n",
      I_DoomExeDir(),
      DEFAULT_CONFIG_FILE_NAME,
      M_GetFileError()
    );
  }

  if (!M_IsFile(config_path)) {
    const char *default_config_contents = NULL;

    if (!X_Call(X_GetState(), "config", "get_default", 0, 1)) {
      I_Error("Error generating default config: %s\n",
        X_GetError(X_GetState())
      );
    }

    default_config_contents = X_PopString(X_GetState());

    success = M_WriteFile(
      config_path,
      default_config_contents,
      strlen(default_config_contents)
    );

    if (!success) {
      I_Error("Error writing default config file: [%s]\n", M_GetFileError());
    }
  }

  if (!X_Call(X_GetState(), "config", "validate", 0, 0)) {
    I_Error("Error validating config: %s\n", X_GetError(X_GetState()));
  }
}

bool D_ConfigLoad(void) {
  char *config_path = M_PathJoin(I_DoomExeDir(), DEFAULT_CONFIG_FILE_NAME);
  char *config_data = NULL;
  size_t config_size = 0;
  bool success = false;

  if (!config_path) {
    I_Error("Error joining %s and %s: %s\n",
      I_DoomExeDir(),
      DEFAULT_CONFIG_FILE_NAME,
      M_GetFileError()
    );
  }

  if (!M_ReadFile(config_path, &config_data, &config_size)) {
    I_Error("Error reading config from [%s]: %s\n",
      config_path, M_GetFileError()
    );
  }

  success = X_Call(X_GetState(), "config", "deserialize", 1, 0,
    X_STRING, config_data
  );

  if (!success) {
    I_Error("Error deserializing config: %s\n", X_GetError(X_GetState()));
  }

  return true;
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

  printf("Saving config to %s\n", config_path);
  if (!M_WriteFile(config_path, config_data, strlen(config_data))) {
    D_Msg(MSG_WARN, "Failed to write config: %s\n", M_GetFileError());
    return false;
  }

  return true;
}

/* vi: set et ts=2 sw=2: */

