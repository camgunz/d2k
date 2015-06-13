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

#include <pango/pangocairo.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "doomdef.h"
#include "doomstat.h"
#include "d_event.h"
#include "c_main.h"
#include "d_main.h"
#include "gl_opengl.h"
#include "gl_intern.h"
#include "hu_lib.h"
#include "hu_input.h"
#include "hu_cons.h"
#include "hu_msg.h"
#include "hu_stuff.h"
#include "i_main.h"
#include "i_system.h"
#include "i_video.h"
#include "lprintf.h"
#include "n_net.h"
#include "n_main.h"
#include "cl_main.h"
#include "v_video.h"
#include "x_main.h"

/* CG TODO: Put these in a configuration file */
#define CONSOLE_SHORTHAND_MARKER "/"
#define CONSOLE_TEXT_PROMPT "> "

typedef struct buffered_console_message_s {
  bool is_markup;
  GString *contents;
} buffered_console_message_t;

static GRegex *shorthand_regex = NULL;
static GPtrArray *buffered_console_messages = NULL;

static void free_buffered_message(gpointer data) {
  buffered_console_message_t *message = (buffered_console_message_t *)data;

  g_string_free(message->contents, true);

  free(data);
}

static void print_buffered_message(gpointer data, gpointer user_data) {
  buffered_console_message_t *msg = (buffered_console_message_t *)data;

  if (msg->is_markup)
    X_Call(X_GetState(), "console", "mwrite", 1, 0, X_STRING, msg->contents);
  else
    X_Call(X_GetState(), "console", "write", 1, 0, X_STRING, msg->contents);
}

static bool save_output_if_scripting_unavailable(const char *message,
                                                 bool is_markup) {
  buffered_console_message_t *msg;

  if (X_Available()) {
    if (buffered_console_messages) {
      g_ptr_array_foreach(
        buffered_console_messages, print_buffered_message, NULL
      );

      g_ptr_array_free(buffered_console_messages, true);

      buffered_console_messages = NULL;
    }

    return false;
  }

  if (!buffered_console_messages) {
    buffered_console_messages = g_ptr_array_new_with_free_func(
      free_buffered_message
    );
  }

  msg = calloc(1, sizeof(buffered_console_message_t));
  msg->is_markup = is_markup;
  msg->contents = g_string_new(message);

  g_ptr_array_add(buffered_console_messages, msg);

  return true;
}

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

void C_Init(void) {
  GError *error = NULL;

  shorthand_regex = g_regex_new(
    "([^\"]\\S*|\".+?\"|'.+?'|)\\s*",
    G_REGEX_OPTIMIZE,
    G_REGEX_MATCH_NOTEMPTY,
    &error
  );

  if (error)
    I_Error("C_Init: Error compiling shorthand regex: %s", error->message);

  if (nodrawers)
    rl_callback_handler_install(CONSOLE_TEXT_PROMPT, C_HandleInput);
}

void C_Reset(void) {
  if (!nodrawers)
    X_Call(X_GetState(), "console", "reset", 0, 0);
}

void C_ScrollDown(void) {
  if (!nodrawers)
    X_Call(X_GetState(), "console", "scroll_down", 0, 0);
}

void C_ScrollUp(void) {
  if (!nodrawers)
    X_Call(X_GetState(), "console", "scroll_up", 0, 0);
}

void C_ToggleScroll(void) {
  if (!nodrawers)
    X_Call(X_GetState(), "console", "toggle_scroll", 0, 0);
}

void C_Summon(void) {
  if (!nodrawers)
    X_Call(X_GetState(), "console", "summon", 0, 0);
}

void C_Banish(void) {
  if (!nodrawers)
    X_Call(X_GetState(), "console", "banish", 0, 0);
}

void C_SetFullscreen(void) {
  if (!nodrawers)
    X_Call(X_GetState(), "console", "set_fullscreen", 0, 0);
}

bool C_Active(void) {
  lua_State *L;
  bool is_active = false;

  if (!nodrawers) {
    L = X_GetState();

    X_Call(L, "console", "is_active", 0, 1);
    is_active = lua_toboolean(L, -1);
    lua_pop(L, 1);
  }

  return is_active;
}

void C_HandleInput(char *input_text) {
  bool success;
  const char *command;

  if (command_is_shorthand(input_text))
    command = parse_shorthand_command(input_text);
  else
    command = input_text;

  success = X_Eval(X_GetState(), command);

  if (!success) {
    char *error_message = X_GetError(X_GetState());

    if (nodrawers) {
      C_Printf("Error: %s\n", error_message);
    }
    else {
      error_message = g_markup_escape_text(error_message, -1);
      C_MPrintf("<span color='red'>Error: %s</span>\n", error_message);
      g_free(error_message);
    }
  }
}

/* CG: TODO: Add D_Log logging */

void C_Printf(const char *fmt, ...) {
  va_list args;

  va_start(args, fmt);
  C_VPrintf(fmt, args);
  va_end(args);
}

void C_VPrintf(const char *fmt, va_list args) {
  gchar *message;

  if (!fmt)
    return;

  message = g_strdup_vprintf(fmt, args);
  C_Write(message);
  g_free(message);
}

void C_MPrintf(const char *fmt, ...) {
  va_list args;

  if (!fmt)
    return;

  va_start(args, fmt);
  C_MVPrintf(fmt, args);
  va_end(args);
}

void C_MVPrintf(const char *fmt, va_list args) {
  gchar *message;
  
  if (!fmt)
    return;

  message = g_strdup_vprintf(fmt, args);
  C_MWrite(message);
  g_free(message);
}

void C_Echo(const char *message) {
  gchar *message_with_newline;

  if (!message)
    return;

  message_with_newline = g_strdup_printf("%s\n", message);
  C_Write(message_with_newline);
  g_free(message_with_newline);
}

void C_MEcho(const char *message) {
  gchar *message_with_newline;

  if (!message)
    return;

  message_with_newline = g_strdup_printf("%s\n", message);
  C_MWrite(message_with_newline);
  g_free(message_with_newline);
}

void C_Write(const char *message) {
  gchar *escaped_message;

  if (!message)
    return;
  
  escaped_message = g_markup_escape_text(message, -1);

  if (nodrawers)
    printf("%s", escaped_message);
  else if (!save_output_if_scripting_unavailable(escaped_message, false))
    X_Call(X_GetState(), "console", "write", 1, 0, X_STRING, escaped_message);

  g_free(escaped_message);
}

void C_MWrite(const char *message) {
  if (!message)
    return;

  if (nodrawers)
    printf("%s", message);
  else if (!save_output_if_scripting_unavailable(message, true))
    X_Call(X_GetState(), "console", "mwrite", 1, 0, X_STRING, message);
}

/* vi: set et ts=2 sw=2: */

