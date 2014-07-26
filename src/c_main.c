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
#include "lprintf.h"
#include "i_main.h"
#include "i_system.h"
#include "v_video.h"
#include "x_main.h"

#define CONSOLE_MAXHEIGHT (REAL_SCREENHEIGHT / 2)
#define CONSOLE_MAXWIDTH  (REAL_SCREENWIDTH)
#define CONSOLE_SCROLL_DOWN_TIME 150.0
#define CONSOLE_SCROLL_UP_TIME   150.0
#define CONSOLE_SCROLLBACK_AMOUNT (CONSOLE_MAXHEIGHT / 4)
#define CONSOLE_SCROLL_DOWN_RATE  (CONSOLE_MAXHEIGHT / CONSOLE_SCROLL_DOWN_TIME)
#define CONSOLE_SCROLL_UP_RATE   -(CONSOLE_MAXHEIGHT / CONSOLE_SCROLL_UP_TIME)
#define CONSOLE_CURSOR_THICKNESS 2
#define CONSOLE_PROMPT_THICKNESS 1
#define CONSOLE_MARGIN 8
#define CONSOLE_SHORTHAND_MARKER "/"

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
"&lt;[UD]Ladna&gt; icereg spec\n" \
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

typedef struct console_scrollback_s {
  GString *buf;
  PangoLayout *layout;
  bool rebuild;
  int offset;
} console_scrollback_t;

typedef struct console_input_s {
  GString *buf;
  size_t cursor;
  uint32_t cursor_active;
  PangoLayout *layout;
  bool rebuild;
  int height;
} console_input_t;

typedef struct console_s {
  console_scrollback_t scrollback;
  console_input_t input;
  int cursor;
  double scroll_rate;
  double height;
  int max_height;
  int max_width;
  char *font_description;
  cairo_t *cairo_context;
  cairo_surface_t *cairo_surface;
  PangoContext *pango_context;
#ifdef GL_DOOM
  GLuint tex_id;
  bool repaint;
#endif
  unsigned char *pixels;
  bool owns_pixels;
} console_t;

static console_t console;
static GRegex *shorthand_regex = NULL;

static void get_scrollback_width_and_height(int *scrollback_width,
                                            int *scrollback_height) {
  int sw;
  int sh;

  pango_layout_get_size(console.scrollback.layout, &sw, &sh);

  *scrollback_width = sw / PANGO_SCALE;
  *scrollback_height = sh / PANGO_SCALE;
}

static void get_input_width_and_height(int *input_width, int *input_height) {
  int iw;
  int ih;

  pango_layout_get_size(console.input.layout, &iw, &ih);

  *input_width = iw / PANGO_SCALE;
  *input_height = ih / PANGO_SCALE;
}

static bool input_cursor_at_start(void) {
  return console.input.cursor <= 0;
}

static bool input_cursor_at_end(void) {
  return console.input.cursor >= console.input.buf->len;
}

static void activate_cursor(void) {
  console.input.cursor_active = I_GetTime();
}

static bool move_input_cursor_left(void) {
  gchar *n_char;

  if (input_cursor_at_start())
    return false;

  n_char = g_utf8_find_prev_char(
    console.input.buf->str,
    console.input.buf->str + console.input.cursor
  );

  if (n_char == NULL)
    return false;

  console.input.cursor = n_char - console.input.buf->str;

  activate_cursor();

  return true;
}

static bool move_input_cursor_right(void) {
  gchar *n_char;

  if (input_cursor_at_end())
    return false;

  n_char = g_utf8_find_next_char(
    console.input.buf->str + console.input.cursor, NULL
  );

  if (n_char == NULL)
    return false;

  console.input.cursor = n_char - console.input.buf->str;

  activate_cursor();

  return true;
}

static void normalize_input(void) {
  char *normalized_input = g_utf8_normalize(
    console.input.buf->str, -1, G_NORMALIZE_DEFAULT
  );

  if (normalized_input == NULL)
    return;

  g_string_assign(console.input.buf, normalized_input);

  g_free(normalized_input);
}

static void insert_input(const char *new_input) {
  g_string_insert(console.input.buf, console.input.cursor, new_input);
  normalize_input();
  console.input.cursor += strlen(new_input);

  activate_cursor();

  console.input.rebuild = true;
}

static void backspace_input(void) {
  gchar *p_char;
  size_t pos;
  size_t size;

  if (input_cursor_at_start())
    return;

  p_char = g_utf8_find_prev_char(
    console.input.buf->str,
    console.input.buf->str + console.input.cursor
  );

  if (p_char == NULL)
    return;

  if (p_char < console.input.buf->str)
    I_Error("backspace_input: previous character exists before string");

  pos = p_char - console.input.buf->str;

  if (pos >= console.input.cursor)
    I_Error("backspace_input: previous character exists after cursor");

  size = console.input.cursor - pos;

  console.input.cursor -= size;
  g_string_erase(console.input.buf, pos, size);
  normalize_input();

  activate_cursor();

  console.input.rebuild = true;
}

static void delete_input(void) {
  gchar *n_char;
  size_t pos;
  size_t size;

  if (input_cursor_at_end())
    return;

  n_char = g_utf8_find_next_char(
    console.input.buf->str + console.input.cursor,
    NULL
  );

  if (n_char == NULL)
    I_Error("delete_input: no next character exists after cursor");

  if (n_char < console.input.buf->str)
    I_Error("delete_input: next character exists before string");

  pos = n_char - console.input.buf->str;

  if (pos < console.input.cursor)
    I_Error("delete_input: next character exists before cursor");

  size = pos - console.input.cursor;

  g_string_erase(console.input.buf, console.input.cursor, size);
  normalize_input();

  activate_cursor();

  console.input.rebuild = true;
}

static void clear_input(void) {
  g_string_erase(console.input.buf, 0, -1);
  console.input.cursor = 0;
  normalize_input();

  activate_cursor();

  console.input.rebuild = true;
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

static void process_input(void) {
  bool success;
  const char *command;
  char *error_message;

  if (command_is_shorthand(console.input.buf->str))
    command = parse_shorthand_command(console.input.buf->str);
  else
    command = console.input.buf->str;

  success = X_RunCode(command);

  if (!success) {
    error_message = g_markup_escape_text(X_GetError(), -1);
    C_MPrintf("<span color='red'>Error: %s</span>\n", error_message);
    g_free(error_message);
  }

  clear_input();
  activate_cursor();
}

static void load_previous_history_line(void) {
  puts("Loading previous history line");
}

static void load_next_history_line(void) {
  puts("Loading next history line");
}

static void scroll_scrollback_up(void) {
  console.scrollback.offset += CONSOLE_SCROLLBACK_AMOUNT;
}

static void scroll_scrollback_down(void) {
  console.scrollback.offset -= CONSOLE_SCROLLBACK_AMOUNT;

  if (console.scrollback.offset <= 0)
    console.scrollback.offset = 0;
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

static void rebuild_scrollback_layout(void) {
  PangoFontDescription *desc;

  if (nodrawers)
    return;

  desc = pango_font_description_from_string(console.font_description);

  pango_layout_set_markup(
    console.scrollback.layout, console.scrollback.buf->str, -1
  );
  pango_layout_set_font_description(console.scrollback.layout, desc);
  pango_layout_set_width(console.scrollback.layout, (console.max_width - (
      CONSOLE_MARGIN + CONSOLE_MARGIN
  )) * PANGO_SCALE);
  pango_layout_set_wrap(console.scrollback.layout, PANGO_WRAP_WORD_CHAR);
  pango_cairo_update_layout(console.cairo_context, console.scrollback.layout);

  pango_font_description_free(desc);
#ifdef GL_DOOM
  console.repaint = true;
#endif
}

static void rebuild_input_layout(void) {
  PangoFontDescription *desc;
  int input_width;
  int input_height;
  int console_input_width;

  if (nodrawers)
    return;

  desc = pango_font_description_from_string(console.font_description);

  get_input_width_and_height(&input_width, &input_height);

  console_input_width = (console.max_width - (
    CONSOLE_MARGIN +
    CONSOLE_MARGIN +
    input_height
  )) * PANGO_SCALE;

  pango_layout_set_text(console.input.layout, console.input.buf->str, -1);
  pango_layout_set_font_description(console.input.layout, desc);
  pango_layout_set_width(console.input.layout, console_input_width);
  pango_layout_set_height(
    console.input.layout, console.input.height * PANGO_SCALE
  );
  pango_cairo_update_layout(console.cairo_context, console.input.layout);

  pango_font_description_free(desc);
#ifdef GL_DOOM
  console.repaint = true;
#endif
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

  console.scrollback.buf = g_string_new("");
  console.input.buf = g_string_new("");
  console.input.cursor = 0;
  console.input.cursor_active = 0;
  console.font_description = "Sans 9";

  C_Reset();

  clear_input();
  // insert_input(TEST_INPUT_TEXT);
  C_MPrintf(TEST_SCROLLBACK_TEXT);

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
}

void C_Reset(void) {
  int input_width;
  int input_height;
  cairo_status_t status;
  // cairo_font_options_t *font_options = cairo_font_options_create();

  console.scroll_rate = 0.0;
  console.height = 0.0;
  console.max_height = CONSOLE_MAXHEIGHT;
  console.max_width = CONSOLE_MAXWIDTH;

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
    I_Error("C_Init: Error creating cairo surface (error %d)\n", status);

  if (console.cairo_context)
    cairo_destroy(console.cairo_context);

  console.cairo_context = cairo_create(console.cairo_surface);

  status = cairo_status(console.cairo_context);

  if (status != CAIRO_STATUS_SUCCESS)
    I_Error("C_Init: Error creating cairo context (error %d)\n", status);

  if (console.pango_context)
    g_object_unref(console.pango_context);

  console.pango_context = pango_cairo_create_context(console.cairo_context);

  if (console.scrollback.layout)
    g_object_unref(console.scrollback.layout);

  console.scrollback.layout = pango_layout_new(console.pango_context);
  pango_layout_set_wrap(console.scrollback.layout, PANGO_WRAP_WORD_CHAR);

  if (console.input.layout)
    g_object_unref(console.input.layout);

  console.input.layout = pango_layout_new(console.pango_context);

  if (console.input.buf->len <= 0) {
    g_string_append(console.input.buf, "DOOM");
    rebuild_input_layout();
    get_input_width_and_height(&input_width, &input_height);
    console.input.height = input_height;
    g_string_erase(console.input.buf, 0, -1);
  }
  else {
    get_input_width_and_height(&input_width, &input_height);
    console.input.height = input_height;
  }

  console.scrollback.rebuild = true;
  console.input.rebuild = true;

  // cairo_font_options_destroy(font_options);
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
  int input_width;
  int input_height;
  double input_height_fracunit = 4.0;
  double input_height_frac;
  double input_height_fracedge;
  int input_offset;
  int sb_width;
  int sb_height;
  int cursor_visible;
  PangoRectangle cpos;
  int cx;
  int cy;
  int cheight;
  uint32_t current_tic = I_GetTime();

  if (V_GetMode() == VID_MODE32 && SDL_MUSTLOCK(screen))
    SDL_LockSurface(screen);

#ifdef GL_DOOM
  if (V_GetMode() == VID_MODEGL)
    cairo_set_operator(console.cairo_context, CAIRO_OPERATOR_SOURCE);
#endif

  if (console.scrollback.rebuild)
    rebuild_scrollback_layout();

  if (console.input.rebuild)
    rebuild_input_layout();

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

  cairo_set_source_rgba(console.cairo_context, 1.0f, 1.0f, 1.0f, 1.0f);

  get_input_width_and_height(&input_width, &input_height);
  input_height_frac = input_height / input_height_fracunit;
  input_height_fracedge = input_height_frac * (input_height_fracunit - 1.0);

  get_scrollback_width_and_height(&sb_width, &sb_height);
  if (console.scrollback.offset > sb_height)
    console.scrollback.offset = sb_height;

  cairo_reset_clip(console.cairo_context);
  cairo_new_path(console.cairo_context);
  cairo_rectangle(
    console.cairo_context,
    CONSOLE_MARGIN,
    0,
    console.max_width,
    console.height - (input_height * 2)
  );
  cairo_clip(console.cairo_context);

  cairo_move_to(
    console.cairo_context,
    CONSOLE_MARGIN,
    console.height - (
      ((input_height * 2) + sb_height) - console.scrollback.offset
    )
  );

  pango_cairo_show_layout(console.cairo_context, console.scrollback.layout);

  cairo_reset_clip(console.cairo_context);
  cairo_new_path(console.cairo_context);

  cairo_move_to(
    console.cairo_context,
    CONSOLE_MARGIN,
    console.height - (CONSOLE_MARGIN + input_height_fracedge)
  );

  cairo_line_to(
    console.cairo_context,
    CONSOLE_MARGIN + (input_height >> 1),
    console.height - (CONSOLE_MARGIN + (input_height >> 1))
  );
  cairo_line_to(
    console.cairo_context,
    CONSOLE_MARGIN,
    console.height - (CONSOLE_MARGIN + input_height_frac)
  );
  cairo_set_line_width(console.cairo_context, CONSOLE_PROMPT_THICKNESS);
  cairo_stroke(console.cairo_context);

  current_tic = I_GetTime();

  cursor_visible = (
    ((current_tic % TICRATE) >= ((TICRATE / 2))) ||
    (console.input.cursor_active >= (current_tic - TICRATE))
  );

  pango_layout_index_to_pos(console.input.layout, console.input.cursor, &cpos);

  cx = (cpos.x / PANGO_SCALE) + CONSOLE_MARGIN + input_height;
  cy = (cpos.y / PANGO_SCALE) + (
    console.height - (input_height + CONSOLE_MARGIN)
  );
  cheight = cpos.height / PANGO_SCALE;

  if (cx < (CONSOLE_MARGIN + input_height))
    input_offset = (CONSOLE_MARGIN + input_height) - cx;
  else if (cx > (console.max_width - CONSOLE_MARGIN))
    input_offset = (console.max_width - CONSOLE_MARGIN) - cx;
  else
    input_offset = 0;

  cx += input_offset;

  if (cursor_visible) {
    cairo_set_source_rgba(console.cairo_context, 0.8f, 0.8f, 0.8f, 1.0f);
    cairo_move_to(console.cairo_context, cx, cy);
    cairo_line_to(console.cairo_context, cx, cy + cheight);
    cairo_set_line_width(console.cairo_context, CONSOLE_CURSOR_THICKNESS);
    cairo_stroke(console.cairo_context);
    cairo_set_source_rgba(console.cairo_context, 1.0f, 1.0f, 1.0f, 1.0f);
  }

  cairo_reset_clip(console.cairo_context);
  cairo_new_path(console.cairo_context);
  cairo_rectangle(
    console.cairo_context,
    CONSOLE_MARGIN + input_height,
    0,
    console.max_width - (CONSOLE_MARGIN + CONSOLE_MARGIN + input_height),
    console.max_height
  );
  cairo_clip(console.cairo_context);

  cairo_move_to(
    console.cairo_context,
    CONSOLE_MARGIN + input_height + input_offset,
    console.height - (input_height + CONSOLE_MARGIN)
  );
  pango_cairo_show_layout(console.cairo_context, console.input.layout);

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
  char *us;
  size_t us_len;
  GError *error = NULL;

  if (!((ev->type == ev_keydown) || (ev->type == ev_mouse)))
    return false;

  /*
   * Mouse keys
   *
   * KEYD_MOUSE1
   * KEYD_MOUSE2
   * KEYD_MOUSE3
   * KEYD_MWHEELUP
   * KEYD_MWHEELDOWN
   */

  if (ev->data1 == KEYD_BACKQUOTE) {
    C_ToggleScroll();
    return true;
  }

  if (console.height <= 0.0)
    return false;

  if (ev->data1 == KEYD_ENTER) {
    process_input();
    return true;
  }

  if (ev->data1 == KEYD_DEL) {
    delete_input();
    return true;
  }

  if (ev->data1 == KEYD_BACKSPACE) {
    backspace_input();
    return true;
  }

  if (ev->data1 == KEYD_LEFTARROW) {
    move_input_cursor_left();
    return true;
  }

  if (ev->data1 == KEYD_RIGHTARROW) {
    move_input_cursor_right();
    return true;
  }

  if (ev->data1 == KEYD_HOME) {
    console.input.cursor = 0;
    return true;
  }

  if (ev->data1 == KEYD_END) {
    console.input.cursor = console.input.buf->len;
    return true;
  }

  if (ev->data1 == KEYD_UPARROW) {
    load_previous_history_line();
    return true;
  }

  if (ev->data1 == KEYD_DOWNARROW) {
    load_next_history_line();
    return true;
  }

  if (ev->data1 == KEYD_PAGEUP && keybindings.shiftdown) {
    scroll_scrollback_up();
    return true;
  }

  if (ev->data1 == KEYD_PAGEDOWN && keybindings.shiftdown) {
    scroll_scrollback_down();
    return true;
  }

  us = g_locale_to_utf8(
    (const char *)&ev->wchar, sizeof(uint16_t), NULL, &us_len, &error
  );

  if (us == NULL) {
    I_Error("C_Responder: Error converting UTF-16 input to UTF-8: %s (%d)\n",
      error->message, error->code
    );
  }

  insert_input(us);

  g_free(us);

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
  gchar *markup_string;
  va_list args;

  if (fmt == NULL)
    return;

  va_start(args, fmt);
  markup_string = g_markup_vprintf_escaped(fmt, args);
  g_string_append(console.scrollback.buf, markup_string);
  g_free(markup_string);
  va_end(args);

  console.scrollback.rebuild = true;
}

void C_MPrintf(const char *fmt, ...) {
  va_list args;

  if (fmt == NULL)
    return;

  va_start(args, fmt);
  g_string_append_vprintf(console.scrollback.buf, fmt, args);
  va_end(args);

  console.scrollback.rebuild = true;
}

void C_Echo(const char *message) {
  gchar *markup_message = g_markup_escape_text(message, -1);

  g_string_append(console.scrollback.buf, markup_message);
  g_string_append(console.scrollback.buf, "\n");

  g_free(markup_message);
}

void C_MEcho(const char *message) {
  g_string_append(console.scrollback.buf, message);
  g_string_append(console.scrollback.buf, "\n");
}

/* vi: set et ts=2 sw=2: */

