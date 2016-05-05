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
    if (!X_Call(X_GetState(), "config", "get_default", 0, 0)) {
      I_Error("Error generating default config: %s\n",
        X_GetError(X_GetState())
      );
    }
  }

  if (!X_Call(X_GetState(), "config", "validate", 0, 0)) {
    I_Error("Error validating config: %s\n", X_GetError(X_GetState()));
  }
}

bool D_ConfigLoad(const char *contents) {
  char *config_path = M_PathJoin(I_DoomExeDir(), DEFAULT_CONFIG_FILE_NAME);
  char *config_data = NULL;
  size_t config_size = 0;

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

  lua_pushstring(X_GetState(), config_data);
  if (!X_Call(X_GetState(), "config", "deserialize", 1, 0)) {
    I_Error("Error deserializing config: %s\n", X_GetError(X_GetState()));
  }

  return true;
}

bool D_ConfigSave(const char *contents) {
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

  if (!M_WriteFile(config_path, config_data, strlen(config_data))) {
    D_Msg(MSG_WARN, "Failed to write config: %s\n", M_GetFileError());
    return false;
  }

  return true;
}

/*
 * /set server.spectate_password "whodat"
 *
 * if (strcmp(C_CVarGetString("server", "spectate_password"), password) == 0) {
 *   // blah blah blah
 * }
 */

bool D_ConfigSafeGetBool(const char *section_name, const char *value_name,
                                                   bool *stored_value) {
  json_t *section;
  json_t *value;

  section = json_object_get(config, section_name);

  if (!section) {
    D_Msg(MSG_WARN, "Config section %s not found\n", section_name);

    return false;
  }

  value = json_object_get(section, value_name);

  if (!value) {
    D_Msg(MSG_WARN, "Value %s.%s not found\n", section_name, value_name);

    return false;
  }

  if (!json_is_boolean(value)) {
    D_Msg(MSG_WARN, "Value %s.%s is not a boolean\n",
      section_name, value_name
    );

    return false;
  }

  if (json_is_true(value)) {
    *stored_value = true;
  }
  else {
    *stored_value = false;
  }

  return true;
} 

bool D_ConfigGetBool(const char *section_name, const char *value_name) {
  bool stored_value = false;

  if (!D_ConfigSafeGetBool(section_name, value_name, &stored_value)) {
    I_Error("Failed to get %s.%s as boolean\n", section_name, value_name);
  }

  return stored_value;
}

bool D_ConfigSetBool(const char *section_name, const char *value_name,
                                               bool new_value) {
  json_t *section;
  json_t *value;
  int     res;

  section = json_object_get(config, section_name);

  if (!section) {
    D_Msg(MSG_WARN, "Config section %s not found\n", section_name);

    return false;
  }

  value = json_object_get(section, value_name);

  if (value) {
    if (!json_is_boolean(value)) {
      D_Msg(MSG_WARN, "Value %s.%s is not a boolean\n",
        section_name, value_name
      );

      return false;
    }
  }

  if (new_value) {
    res = json_object_set(section, value_name, json_true());
  }
  else {
    res = json_object_set(section, value_name, json_false());
  }

  if (res != 0) {
    if (new_value) {
      D_Msg(MSG_WARN, "Failed to set %s.%s to true\n",
        section_name, value_name
      );
    }
    else {
      D_Msg(MSG_WARN, "Failed to set %s.%s to false\n",
        section_name, value_name
      );
    }

    return false;
  }

  return true;
}

bool D_ConfigSafeGetStr(const char *section_name, const char *value_name,
                                                  char **stored_value) {
  json_t *section;
  json_t *value;

  section = json_object_get(config, section_name);

  if (!section) {
    D_Msg(MSG_WARN, "Config section %s not found\n", section_name);

    return false;
  }

  value = json_object_get(section, value_name);

  if (!value) {
    D_Msg(MSG_WARN, "Value %s.%s not found\n", section_name, value_name);

    return false;
  }

  if (!json_is_string(value)) {
    D_Msg(MSG_WARN, "Value %s.%s is not a string\n", section_name, value_name);

    return false;
  }

  *stored_value = (char *)json_string_value(value);

  return true;
} 

char* D_ConfigGetStr(const char *section_name, const char *value_name) {
  char *stored_value = NULL;

  if (!D_ConfigSafeGetStr(section_name, value_name, &stored_value)) {
    I_Error("Failed to get %s.%s as boolean\n", section_name, value_name);
  }

  return stored_value;
}

bool D_ConfigSetStr(const char *section_name, const char *value_name,
                                              const char *new_value) {
  json_t *section;
  json_t *value;
  int     res;

  section = json_object_get(config, section_name);

  if (!section) {
    D_Msg(MSG_WARN, "Config section %s not found\n", section_name);

    return false;
  }

  value = json_object_get(section, value_name);

  if (value) {
    if (!json_is_string(value)) {
      D_Msg(MSG_WARN, "Value %s.%s is not a string\n",
        section_name, value_name
      );

      return false;
    }

    res = json_string_set(value, new_value);

    if (res != 0) {
      D_Msg(MSG_WARN, "Failed to set %s.%s to %s\n",
        section_name, value_name, new_value
      );

      return false;
    }

    return true;
  }

  res = json_object_set_new(section, value_name, json_string(new_value));

  if (res != 0) {
    D_Msg(MSG_WARN, "Failed to set %s.%s to %s\n",
      section_name, value_name, new_value
    );

    return false;
  }

  return true;
}

bool D_ConfigSafeGetInt(const char *section_name, const char *value_name,
                                                  int64_t *stored_value) {
  json_t *section;
  json_t *value;

  section = json_object_get(config, section_name);

  if (!section) {
    D_Msg(MSG_WARN, "Config section %s not found\n", section_name);

    return false;
  }

  value = json_object_get(section, value_name);

  if (!value) {
    D_Msg(MSG_WARN, "Value %s.%s not found\n", section_name, value_name);

    return false;
  }

  if (!json_is_integer(value)) {
    D_Msg(MSG_WARN, "Value %s.%s is not an integer\n",
      section_name, value_name
    );

    return false;
  }

  *stored_value = json_integer_value(value);

  return true;
} 

int64_t D_ConfigGetInt(const char *section_name, const char *value_name) {
  int64_t stored_value = 0;

  if (!D_ConfigSafeGetInt(section_name, value_name, &stored_value)) {
    I_Error("Failed to get %s.%s as boolean\n", section_name, value_name);
  }

  return stored_value;
}

bool D_ConfigSetInt(const char *section_name, const char *value_name,
                                              int64_t new_value) {
  json_t *section;
  json_t *value;
  int     res;

  section = json_object_get(config, section_name);

  if (!section) {
    D_Msg(MSG_WARN, "Config section %s not found\n", section_name);

    return false;
  }

  value = json_object_get(section, value_name);

  if (value) {
    if (!json_is_integer(value)) {
      D_Msg(MSG_WARN, "Value %s.%s is not an integer\n",
        section_name, value_name
      );

      return false;
    }

    res = json_integer_set(value, new_value);

    if (res != 0) {
      D_Msg(MSG_WARN, "Failed to set %s.%s to %" PRId64 "\n",
        section_name, value_name, new_value
      );

      return false;
    }

    return true;
  }

  res = json_object_set_new(section, value_name, json_integer(new_value));

  if (res != 0) {
    D_Msg(MSG_WARN, "Failed to set %s.%s to %" PRId64 "\n",
      section_name, value_name, new_value
    );

    return false;
  }

  return true;
}

bool D_ConfigSafeGetDec(const char *section_name, const char *value_name,
                                                  double *stored_value) {
  json_t *section;
  json_t *value;

  section = json_object_get(config, section_name);

  if (!section) {
    D_Msg(MSG_WARN, "Config section %s not found\n", section_name);

    return false;
  }

  value = json_object_get(section, value_name);

  if (!value) {
    D_Msg(MSG_WARN, "Value %s.%s not found\n", section_name, value_name);

    return false;
  }

  if (!json_is_real(value)) {
    D_Msg(MSG_WARN, "Value %s.%s is not an real\n",
      section_name, value_name
    );

    return false;
  }

  *stored_value = json_real_value(value);

  return true;
} 

double D_ConfigGetDec(const char *section_name, const char *value_name) {
  double stored_value = 0;

  if (!D_ConfigSafeGetDec(section_name, value_name, &stored_value)) {
    I_Error("Failed to get %s.%s as boolean\n", section_name, value_name);
  }

  return stored_value;
}

bool D_ConfigSetDec(const char *section_name, const char *value_name,
                                              double new_value) {
  json_t *section;
  json_t *value;
  int     res;

  section = json_object_get(config, section_name);

  if (!section) {
    D_Msg(MSG_WARN, "Config section %s not found\n", section_name);

    return false;
  }

  value = json_object_get(section, value_name);

  if (value) {
    if (!json_is_real(value)) {
      D_Msg(MSG_WARN, "Value %s.%s is not a decimal\n",
        section_name, value_name
      );

      return false;
    }

    res = json_real_set(value, new_value);

    if (res != 0) {
      D_Msg(MSG_WARN, "Failed to set %s.%s to %f\n",
        section_name, value_name, new_value
      );

      return false;
    }

    return true;
  }

  res = json_object_set_new(section, value_name, json_real(new_value));

  if (res != 0) {
    D_Msg(MSG_WARN, "Failed to set %s.%s to %f\n",
      section_name, value_name, new_value
    );

    return false;
  }

  return true;
}

/* vi: set et ts=2 sw=2: */

