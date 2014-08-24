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
#include <glib.h>

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
#include "v_video.h"
#include "x_main.h"

#define CONSOLE_SHORTHAND_MARKER "/"

typedef struct console_s {
  message_widget_t *scrollback;
  input_widget_t *input;
  double scroll_rate;
  double height;
  int max_height;
  int max_width;
} console_t;

static console_widget_t *cons;
static GString *tempbuf = NULL;
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

  g_string_printf(command, "%s.", X_NAMESPACE);

  for (size_t i = 0; i < token_count; i++) {
    char *token = tokens[i];

    if (token == NULL)
      break;

    if (!strlen(token))
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

  success = X_RunCode(command);

  if (!success) {
    error_message = g_markup_escape_text(X_GetError(), -1);
    C_MPrintf("<span color='red'>Error: %s</span>\n", error_message);
    g_free(error_message);
  }

  HU_InputWidgetClear(iw);
}

static void tempbuf_vprintf(const char *fmt, va_list args) {
  gchar *markup_string;

  if (fmt == NULL)
    return;

  if (tempbuf == NULL)
    tempbuf = g_string_new("");

  markup_string = g_markup_vprintf_escaped(fmt, args);

  g_string_append(tempbuf, markup_string);
  g_free(markup_string);
}

static void tempbuf_mvprintf(const char *fmt, va_list args) {
  if (fmt == NULL)
    return;

  if (tempbuf == NULL)
    tempbuf = g_string_new("");

  g_string_append_vprintf(tempbuf, fmt, args);
}

static void tempbuf_echo(const char *message) {
  gchar *markup_message;

  if (message == NULL)
    return;
  
  if (tempbuf == NULL)
    tempbuf = g_string_new("");

  markup_message = g_markup_escape_text(message, -1);

  g_string_append(tempbuf, markup_message);
  g_string_append(tempbuf, "\n");

  g_free(markup_message);
}

static void tempbuf_mecho(const char *message) {
  if (message == NULL)
    return;

  if (tempbuf == NULL)
    tempbuf = g_string_new("");

  g_string_append(tempbuf, message);
  g_string_append(tempbuf, "\n");
}

static void tempbuf_write(const char *message) {
  gchar *markup_message;

  if (message == NULL)
    return;
  
  if (tempbuf == NULL)
    tempbuf = g_string_new("");

  markup_message = g_markup_escape_text(message, -1);

  g_string_append(tempbuf, markup_message);

  g_free(markup_message);
}

static void tempbuf_mwrite(const char *message) {
  if (message == NULL)
    return;

  if (tempbuf == NULL)
    tempbuf = g_string_new("");

  g_string_append(tempbuf, message);
}

static void command_line_vprintf(const char *fmt, va_list args) {
  gchar *message = g_strdup_vprintf(fmt, args);
  gchar *escaped_message = g_markup_escape_text(message, -1);

  printf("%s", escaped_message);

  g_free(escaped_message);
  g_free(message);
}

static void command_line_mvprintf(const char *fmt, va_list args) {
  vprintf(fmt, args);
}

static void command_line_echo(const char *message) {
  gchar *escaped_message = g_markup_escape_text(message, -1);

  printf("%s", escaped_message);

  g_free(escaped_message);
}

static void command_line_mecho(const char *message) {
  puts(message);
}

static void command_line_write(const char *message) {
  gchar *escaped_message = g_markup_escape_text(message, -1);

  printf("%s", escaped_message);

  g_free(escaped_message);
}

static void command_line_mwrite(const char *message) {
  printf("%s", message);
}

int XF_Echo(lua_State *L) {
  const char *message = luaL_checkstring(L, 1);

  if (message)
    C_Echo(message);

  return 0;
}

int XF_MEcho(lua_State *L) {
  const char *markup_message = luaL_checkstring(L, 1);

  if (markup_message)
    C_MEcho(markup_message);

  return 0;
}

void C_Init(void) {
  GError *error = NULL;

  if (!nodrawers) {
    cons = HU_ConsoleWidgetNew(
      I_GetRenderContext(),
      0,
      0,
      REAL_SCREENWIDTH, REAL_SCREENHEIGHT >> 1,
      process_console_input
    );
  }

  shorthand_regex = g_regex_new(
    "([^\"]\\S*|\".+?\"|'.+?'|)\\s*",
    G_REGEX_OPTIMIZE,
    G_REGEX_MATCH_NOTEMPTY,
    &error
  );

  if (error)
    I_Error("C_Init: Error compiling shorthand regex: %s", error->message);

  if (tempbuf) {
    C_MWrite(tempbuf->str);
    g_string_free(tempbuf, true);
  }

  X_RegisterFunc("echo", XF_Echo);
  X_RegisterFunc("mecho", XF_MEcho);
}

void C_Reset(void) {
  if (!nodrawers)
    HU_ConsoleWidgetReset(cons, I_GetRenderContext());
}

void C_Ticker(void) {
  if (!nodrawers)
    HU_ConsoleWidgetTicker(cons);
}

void C_Drawer(void) {
  if (!nodrawers)
    HU_ConsoleWidgetDrawer(cons, I_GetRenderContext());
}

bool C_Responder(event_t *ev) {
  return false;

  if (nodrawers)
    return false;

  return HU_ConsoleWidgetResponder(cons, ev);
}

void C_ScrollDown(void) {
  if (!nodrawers)
    HU_ConsoleWidgetScrollDown(cons);
}

void C_ScrollUp(void) {
  if (!nodrawers)
    HU_ConsoleWidgetScrollUp(cons);
}

void C_ToggleScroll(void) {
  if (!nodrawers)
    HU_ConsoleWidgetToggleScroll(cons);
}

void C_Summon(void) {
  if (!nodrawers)
    HU_ConsoleWidgetSummon(cons);
}

void C_Banish(void) {
  if (!nodrawers)
    HU_ConsoleWidgetBanish(cons);
}

void C_SetFullScreen(void) {
  if (!nodrawers)
    HU_ConsoleWidgetSetFullScreen(cons);
}

bool C_Active(void) {
  if (!nodrawers)
    return HU_ConsoleWidgetActive(cons);

  return false;
}

/*
 * CG: TODO: Move printing to stdout from hu_msg to here
 *     TODO: Add D_Log logging
 */

void C_Printf(const char *fmt, ...) {
  va_list args;

  if (CL_RePredicting())
    return;

  va_start(args, fmt);
  if (nodrawers)
    command_line_vprintf(fmt, args);
  else if (cons)
    HU_ConsoleWidgetVPrintf(cons, fmt, args);
  else
    tempbuf_vprintf(fmt, args);
  va_end(args);
}

void C_VPrintf(const char *fmt, va_list args) {
  if (CL_RePredicting())
    return;

  if (nodrawers)
    command_line_vprintf(fmt, args);
  else if (cons)
    HU_ConsoleWidgetVPrintf(cons, fmt, args);
  else
    tempbuf_vprintf(fmt, args);
}

void C_MPrintf(const char *fmt, ...) {
  va_list args;

  if (CL_RePredicting())
    return;

  va_start(args, fmt);
  if (nodrawers)
    command_line_mvprintf(fmt, args);
  else if (cons)
    HU_ConsoleWidgetMVPrintf(cons, fmt, args);
  else
    tempbuf_mvprintf(fmt, args);
  va_end(args);
}

void C_MVPrintf(const char *fmt, va_list args) {
  if (CL_RePredicting())
    return;

  if (nodrawers)
    command_line_mvprintf(fmt, args);
  else if (cons)
    HU_ConsoleWidgetMVPrintf(cons, fmt, args);
  else
    tempbuf_mvprintf(fmt, args);
}

void C_Echo(const char *message) {
  if (CL_RePredicting())
    return;

  if (nodrawers)
    command_line_echo(message);
  else if (cons)
    HU_ConsoleWidgetEcho(cons, message);
  else
    tempbuf_echo(message);
}

void C_MEcho(const char *message) {
  if (CL_RePredicting())
    return;

  if (nodrawers)
    command_line_mecho(message);
  else if (cons)
    HU_ConsoleWidgetMEcho(cons, message);
  else
    tempbuf_mecho(message);
}

void C_Write(const char *message) {
  if (CL_RePredicting())
    return;

  if (nodrawers)
    command_line_write(message);
  else if (cons)
    HU_ConsoleWidgetWrite(cons, message);
  else
    tempbuf_write(message);
}

void C_MWrite(const char *message) {
  if (CL_RePredicting())
    return;

  if (nodrawers)
    command_line_mwrite(message);
  else if (cons)
    HU_ConsoleWidgetMWrite(cons, message);
  else
    tempbuf_mwrite(message);
}

/* vi: set et ts=2 sw=2: */

