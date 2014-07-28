/* Emacs style mode select -*- C++ -*-
 *-----------------------------------------------------------------------------
 *
 *
 *  PrBoom: a Doom port merged with LxDoom and LSDLDoom
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 1999 by
 *  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
 *  Copyright (C) 1999-2000 by
 *  Jess Haas, Nicolas Kalkhof, Colin Phipps, Florian Schulze
 *  Copyright 2005, 2006 by
 *  Florian Schulze, Colin Phipps, Neil Stevens, Andrey Budko
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 *  02111-1307, USA.
 *
 * DESCRIPTION:
 *   Console
 *
 *-----------------------------------------------------------------------------
 */

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
#include "hu_msg.h"
#include "i_main.h"
#include "i_system.h"
#include "lprintf.h"
#include "v_video.h"
#include "x_main.h"

#define CONSOLE_MAXHEIGHT (REAL_SCREENHEIGHT / 2)
#define CONSOLE_MAXWIDTH  (REAL_SCREENWIDTH)
#define CONSOLE_SCROLL_DOWN_TIME 150.0
#define CONSOLE_SCROLL_UP_TIME   150.0
#define CONSOLE_SCROLL_DOWN_RATE  (CONSOLE_MAXHEIGHT / CONSOLE_SCROLL_DOWN_TIME)
#define CONSOLE_SCROLL_UP_RATE   -(CONSOLE_MAXHEIGHT / CONSOLE_SCROLL_UP_TIME)
#define CONSOLE_MARGIN 8
#define CONSOLE_SHORTHAND_MARKER "/"
#define CONSOLE_SCROLLBACK_AMOUNT (CONSOLE_MAXHEIGHT / 4)

#define TEST_SCROLLBACK_TEXT \
"&lt;[UD]Ladna&gt; line 1\n" \
"&lt;[UD]Ladna&gt; line 2\n" \
"&lt;[UD]Ladna&gt; line 3\n" \
"&lt;[UD]Ladna&gt; line 4\n" \
"&lt;[UD]Ladna&gt; line 5\n" \
"&lt;[UD]Ladna&gt; line 6\n" \
"&lt;[UD]Ladna&gt; line 7\n" \
"&lt;[UD]Ladna&gt; line 8\n" \
"&lt;[UD]Ladna&gt; line 9\n" \
"&lt;[UD]Ladna&gt; line 10\n" \
"&lt;[UD]Ladna&gt; line 11\n" \
"&lt;[UD]Ladna&gt; line 12\n" \
"&lt;[UD]Ladna&gt; line 13\n" \
"&lt;[UD]Ladna&gt; line 14\n" \
"&lt;[UD]Ladna&gt; line 14\n" \
"&lt;JKist3[NS]&gt; go?\n" \
"&lt;icereg&gt; 開発メモ 技術系の作業メモおよびアイデアの記録\n" \
"[UD]Ladna joined the game\n" \
"JKist3[NS] joined the game\n" \
"[UD]Ladna was splattered by JKist3[NS]'s super shotgun\n" \
"icereg joined the game\n" \
"&lt;[UD]Ladna&gt; icereg <b>spec</b>\n" \
"&lt;icereg&gt; Photoshop等で実装されている\n" \
"*icereg was kicked from the server (Message: spec)*\n" \
"&lt;[UD]Ladna&gt; This is a super long message, well well well, who knew I could " \
    "type such a long line eh????  Apparently it wasn't long enough to " \
    "start with, so I've now made it even longer.  Let's hope this is long " \
    "enough to wrap now\n" \

#define TEST_INPUT_TEXT \
  "type such a long line eh????  Apparently it wasn't long enough to " \
  "blaaaaah 技術系の作業メモおよびアイデアの記録" \
  "start with"

extern SDL_Surface *screen;

typedef struct console_s {
  message_widget_t *scrollback;
  input_widget_t *input;
  double scroll_rate;
  double height;
  int max_height;
  int max_width;
  cairo_t *cairo_context;
  cairo_surface_t *cairo_surface;
#ifdef GL_DOOM
  GLuint tex_id;
  bool repaint;
#endif
  unsigned char *pixels;
  bool owns_pixels;
} console_t;

static console_t console;
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
}

static void build_gl_texture(void) {
  if (nodrawers)
    return;

  if (console.tex_id)
    glDeleteTextures(1, &console.tex_id);

  glGenTextures(1, &console.tex_id);

  glBindTexture(GL_TEXTURE_2D, console.tex_id);

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GLEXT_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GLEXT_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
}

static void update_gl_texture(void) {
  if (nodrawers)
    return;

  glBindTexture(GL_TEXTURE_2D, console.tex_id);
  glTexImage2D(
    GL_TEXTURE_2D,
    0,
    GL_RGBA,
    console.max_width,
    console.max_height,
    0,
    GL_RGBA,
    GL_UNSIGNED_BYTE,
    console.pixels
  );
}

static void reset_rendering_context(void) {
  cairo_status_t status;

  if (nodrawers)
    return;

  if (V_GetMode() == VID_MODE32) {
    console.pixels = screen->pixels;
    console.owns_pixels = false;
  }
#ifdef GL_DOOM
  else if (V_GetMode() == VID_MODEGL) {
    if (console.owns_pixels && console.pixels)
      free(console.pixels);

    console.pixels = malloc(
      console.max_width * console.max_height * sizeof(unsigned int)
    );

    console.owns_pixels = true;

    if (console.tex_id)
      glDeleteTextures(1, &console.tex_id);

    glGenTextures(1, &console.tex_id);
  }
#endif
  else {
    I_Error("Invalid video mode %d\n", V_GetMode());
  }

  if (console.cairo_surface)
    cairo_surface_destroy(console.cairo_surface);

  console.cairo_surface = cairo_image_surface_create_for_data(
    console.pixels,
    CAIRO_FORMAT_ARGB32,
    console.max_width,
    console.max_height,
    cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, console.max_width)
  );

  status = cairo_surface_status(console.cairo_surface);

  if (status != CAIRO_STATUS_SUCCESS)
    I_Error("C_Init: Error creating cairo surface (error %d)", status);

  if (console.cairo_context)
    cairo_destroy(console.cairo_context);

  console.cairo_context = cairo_create(console.cairo_surface);

  status = cairo_status(console.cairo_context);

  if (status != CAIRO_STATUS_SUCCESS)
    I_Error("C_Init: Error creating cairo context (error %d)", status);
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
  int input_width;
  int input_height;

  console.scroll_rate = 0.0;
  console.height = 0.0;
  console.max_height = CONSOLE_MAXHEIGHT;
  console.max_width = CONSOLE_MAXWIDTH;

  reset_rendering_context();

  console.scrollback = HU_MessageWidgetNew(
    console.cairo_context,
    CONSOLE_MARGIN,
    0,
    console.max_width,
    console.max_height,
    CONSOLE_SCROLLBACK_AMOUNT
  );
  console.input = HU_InputWidgetNew(
    console.cairo_context,
    CONSOLE_MARGIN,
    0,
    console.max_width,
    console.max_height,
    process_console_input
  );

  HU_InputWidgetSetFont(console.input, "Sans 9");
  HU_InputWidgetGetLayoutSize(console.input, &input_width, &input_height);
  HU_InputWidgetSetSize(
    console.input,
    console.max_width - (CONSOLE_MARGIN + CONSOLE_MARGIN),
    input_height
  );

  HU_MessageWidgetSetFont(console.scrollback, "Sans 9");
  HU_MessageWidgetSetSize(
    console.scrollback,
    console.max_width,
    console.max_height - (CONSOLE_MARGIN + input_height + input_height)
  );

  C_Reset();

  build_gl_texture();

  shorthand_regex = g_regex_new(
    "([^\"]\\S*|\".+?\"|'.+?'|)\\s*",
    G_REGEX_OPTIMIZE,
    G_REGEX_MATCH_NOTEMPTY,
    &error
  );

  if (error)
    I_Error("C_Init: Error compiling shorthand regex: %s", error->message);

  X_RegisterFunc("echo", XF_Echo);
  X_RegisterFunc("mecho", XF_MEcho);

  // C_MPrintf(TEST_SCROLLBACK_TEXT);
}

void C_Reset(void) {
  int input_width;
  int input_height;

  console.scroll_rate = 0.0;
  console.height = 0.0;
  console.max_height = CONSOLE_MAXHEIGHT;
  console.max_width = CONSOLE_MAXWIDTH;

  reset_rendering_context();
  HU_InputWidgetReset(console.input, console.cairo_context);
  HU_MessageWidgetReset(console.scrollback, console.cairo_context);
  HU_MessageWidgetSetScrollAmount(
    console.scrollback, CONSOLE_SCROLLBACK_AMOUNT
  );

  HU_InputWidgetGetLayoutSize(console.input, &input_width, &input_height);
  HU_InputWidgetSetSize(
    console.input,
    console.max_width - (CONSOLE_MARGIN + CONSOLE_MARGIN),
    input_height
  );

  HU_MessageWidgetSetSize(
    console.scrollback,
    console.max_width,
    console.max_height - (CONSOLE_MARGIN + input_height + input_height)
  );
}

void C_Ticker(void) {
  static int last_run_ms = 0;

  int ms_elapsed;
  int current_ms = I_GetTicks();

  if (last_run_ms == 0)
    last_run_ms = current_ms;

  ms_elapsed = current_ms - last_run_ms;
  last_run_ms = current_ms;

#ifdef GL_DOOM
  if (console.scroll_rate)
    console.repaint = true;
#endif

  console.height += (console.scroll_rate * ms_elapsed);

  if (console.height <= 0.0) {
    console.height = 0.0;
    console.scroll_rate = 0.0;
  }
  else if (console.height >= console.max_height) {
    console.height = console.max_height;
    console.scroll_rate = 0.0;
  }
}

void C_Drawer(void) {
  int sb_width;
  int sb_height;
  int input_width;
  int input_height;

  if (V_GetMode() == VID_MODE32 && SDL_MUSTLOCK(screen))
    SDL_LockSurface(screen);

#ifdef GL_DOOM
  if (V_GetMode() == VID_MODEGL)
    cairo_set_operator(console.cairo_context, CAIRO_OPERATOR_SOURCE);
#endif

  cairo_reset_clip(console.cairo_context);
  cairo_new_path(console.cairo_context);
  cairo_rectangle(console.cairo_context, 0, 0, console.max_width, console.max_height);
  cairo_clip(console.cairo_context);
  cairo_set_source_rgba(console.cairo_context, 0.0f, 0.0f, 0.0f, 0.0f);
  cairo_paint(console.cairo_context);

  cairo_reset_clip(console.cairo_context);
  cairo_new_path(console.cairo_context);
  cairo_rectangle(console.cairo_context, 0, 0, console.max_width, console.height);
  cairo_clip(console.cairo_context);
  cairo_set_source_rgba(console.cairo_context, 0.0f, 0.0f, 0.0f, 0.8f);
  cairo_paint(console.cairo_context);

  HU_MessageWidgetGetLayoutSize(console.scrollback, &sb_width, &sb_height);
  HU_InputWidgetGetLayoutSize(console.input, &input_width, &input_height);

  HU_InputWidgetSetPosition(
    console.input,
    CONSOLE_MARGIN,
    console.height - (input_height + CONSOLE_MARGIN)
  );

  HU_MessageWidgetSetPosition(console.scrollback, CONSOLE_MARGIN, 0);

  HU_MessageWidgetDrawer(console.scrollback, console.cairo_context);
  HU_InputWidgetDrawer(console.input, console.cairo_context);

#ifdef GL_DOOM
  if (V_GetMode() == VID_MODEGL) {
    cairo_set_operator(console.cairo_context, CAIRO_OPERATOR_OVER);
    cairo_surface_flush(console.cairo_surface);

    if (console.repaint) {
      update_gl_texture();

      glBindTexture(GL_TEXTURE_2D, console.tex_id);

      glBegin(GL_TRIANGLE_STRIP);
      glTexCoord2f(0.0f, 0.0f);
      glVertex2f(0.0f, 0.0f);
      glTexCoord2f(0.0f, 1.0f);
      glVertex2f(0.0f, console.max_height);
      glTexCoord2f(1.0f, 0.0f);
      glVertex2f(console.max_width, 0.0f);
      glTexCoord2f(1.0f, 1.0f);
      glVertex2f(console.max_width, console.max_height);
      glEnd();
    }
  }
#endif

  if (V_GetMode() == VID_MODE32 && SDL_MUSTLOCK(screen))
    SDL_UnlockSurface(screen);
}

bool C_Responder(event_t *ev) {
  if (ev->type == ev_keydown && ev->data1 == KEYD_BACKQUOTE) {
    C_ToggleScroll();
    return true;
  }

  if (!C_Active())
    return false;

  if (HU_MessageWidgetResponder(console.scrollback, ev))
    return true;

  if (HU_InputWidgetResponder(console.input, ev))
    return true;

  return true;
}

void C_ScrollDown(void) {
  console.scroll_rate = CONSOLE_SCROLL_DOWN_RATE;
}

void C_ScrollUp(void) {
  console.scroll_rate = CONSOLE_SCROLL_UP_RATE;
}

void C_ToggleScroll(void) {
  if (console.height == console.max_height)
    C_ScrollUp();
  else if (console.height == 0)
    C_ScrollDown();
  else if (console.scroll_rate < 0.0)
    C_ScrollDown();
  else if (console.scroll_rate > 0.0)
    C_ScrollUp();
}

void C_Summon(void) {
  console.height = console.max_height;
  console.scroll_rate = 0.0;
}

void C_Banish(void) {
  console.height = 0;
  console.scroll_rate = 0.0;
}

void C_SetFullScreen(void) {
  console.height = REAL_SCREENHEIGHT;
  console.scroll_rate = 0.0;
}

bool C_Active(void) {
  return (console.height > 0.0);
}

void C_Printf(const char *fmt, ...) {
  va_list args;

  va_start(args, fmt);
  HU_MessageWidgetVPrintf(console.scrollback, fmt, args);
  va_end(args);
}

/*
 * CG: TODO: Move printing to stdout from hu_msg to here
 *     TODO: Add D_Log logging
 */

void C_MPrintf(const char *fmt, ...) {
  va_list args;

  va_start(args, fmt);
  HU_MessageWidgetMVPrintf(console.scrollback, fmt, args);
  va_end(args);
}

void C_Echo(const char *message) {
  HU_MessageWidgetEcho(console.scrollback, message);
}

void C_MEcho(const char *message) {
  HU_MessageWidgetMEcho(console.scrollback, message);
}

void C_Write(const char *message) {
  HU_MessageWidgetWrite(console.scrollback, message);
}

void C_MWrite(const char *message) {
  HU_MessageWidgetMWrite(console.scrollback, message);
}

/* vi: set et ts=2 sw=2: */

