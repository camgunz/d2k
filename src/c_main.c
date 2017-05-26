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

#include "c_eci.h"
#include "c_main.h"
#include "g_game.h"
#include "x_main.h"

/* CG TODO: Put these in a configuration file */
#define CONSOLE_SHORTHAND_MARKER "/"
#define CONSOLE_TEXT_PROMPT "> "

/* [CG] [TODO] Use GScanner instead of a regex */
static GRegex  *shorthand_regex = NULL;
static GString *buffered_console_messages = NULL;
static bool     buffered_console_messages_updated = false;

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

  if (!command) {
    command = g_string_new("");
  }

  short_command = strip_shorthand_marker(short_command);

  if (!short_command) {
    return "";
  }

  tokens = g_regex_split(shorthand_regex, short_command, 0);
  token_count = g_strv_length(tokens);

  if (!token_count) {
    return "";
  }

  g_string_printf(command, "%s.Shortcuts.", X_NAMESPACE);

  for (size_t i = 0; i < token_count; i++) {
    char *token = tokens[i];

    if (!token) {
      break;
    }

    if (!(*token)) {
      continue;
    }

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

  if (error) {
    I_Error("C_Init: Error compiling shorthand regex: %s\n", error->message);
  }

#ifdef G_OS_UNIX
  if (SERVER) {
    C_ECIInit();
  }
#endif
}

bool C_HandleInput(char *input_text) {
  bool success;
  const char *command;

  if (command_is_shorthand(input_text)) {
    command = parse_shorthand_command(input_text);
  }
  else {
    command = input_text;
  }

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

  return success;
}

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

void C_Write(const char *msg) {
  gchar *emsg;

  if (!msg)
    return;
  
  emsg = g_markup_escape_text(msg, -1);

  if (!buffered_console_messages)
    buffered_console_messages = g_string_new(emsg);
  else
    g_string_append(buffered_console_messages, emsg);

  buffered_console_messages_updated = true;

  g_free(emsg);
}

void C_MWrite(const char *msg) {
  if (!msg)
    return;

  if (!buffered_console_messages)
    buffered_console_messages = g_string_new(msg);
  else
    g_string_append(buffered_console_messages, msg);

  buffered_console_messages_updated = true;
}

GString* C_GetMessages(void) {
  return buffered_console_messages;
}

bool C_MessagesUpdated(void) {
  return buffered_console_messages_updated;
}

void C_ClearMessagesUpdated(void) {
  buffered_console_messages_updated = false;
  g_string_erase(buffered_console_messages, 0, -1);
}

/* vi: set et ts=2 sw=2: */

