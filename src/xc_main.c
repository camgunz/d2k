
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

#include "x_main.h"

#include <readline.h>
#include <history.h>

#define CONSOLE_SHORTHAND_MARKER "/"

static GRegex *shorthand_regex = NULL;

static bool command_is_shorthand(const char *command) {
  return strncmp(
    CONSOLE_SHORTHAND_MARKER, command, strlen(CONSOLE_SHORTHAND_MARKER)
  ) == 0;
}

static const char* strip_shorthand_marker(const char *command) {
  while (command_is_shorthand(command))
    command += strlen(CONSOLE_SHORTHAND_MARKER);

  return command;
}

static const char* parse_shorthand_command(const char *short_command) {
  static GString *command = NULL;

  char **tokens;
  size_t token_count;
  bool wrote_func_name = false;
  bool wrote_first_argument = false;

  if (command == NULL)
    command = g_string_new("");

  short_command = strip_shorthand_marker(short_command);

  if (short_command == NULL)
    return "";

  tokens = g_regex_split(shorthand_regex, short_command, 0);
  token_count = g_strv_length(tokens);

  if (token_count == 0)
    return "";

  g_string_printf(command, "%s.Shortcuts.", X_NAMESPACE);

  for (size_t i = 0; i < token_count; i++) {
    char *token = tokens[i];

    if (token == NULL)
      break;

    if (!(*token))
      continue;

    if (!wrote_func_name) {
      g_string_append_printf(command, "%s(", token);
      wrote_func_name = true;
    }
    else if (!wrote_first_argument) {
      g_string_append(command, token);
      wrote_first_argument = true;
    }
    else {
      g_string_append_printf(command, ", %s", token);
    }
  }

  g_string_append(command, ")");

  return command->str;
}

static void process_console_input(input_widget_t *iw) {
  bool success;
  const char *command;
  char *error_message;
  char *input_text = HU_InputWidgetGetText(iw);

  if (command_is_shorthand(input_text))
    command = parse_shorthand_command(input_text);
  else
    command = input_text;

  success = X_Eval(X_GetState(), command);

  if (!success) {
    error_message = g_markup_escape_text(X_GetError(X_GetState()), -1);
    C_MPrintf("<span color='red'>Error: %s</span>\n", error_message);
    g_free(error_message);
  }

  HU_InputWidgetClear(iw);
}

static int XC_Initialize(lua_State *L) {
  const char *prompt = luaL_checkstring(L);

  rl_callback_handler_install(prompt, input_handler);
}

void XC_RegisterInterface(void) {
}

/* vi: set et ts=2 sw=2: */

