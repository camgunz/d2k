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
#include "gl_opengl.h"
#include "gl_intern.h"
#include "lprintf.h"
#include "i_main.h"
#include "i_system.h"
#include "v_video.h"

#define CONSOLE_MAXHEIGHT (REAL_SCREENHEIGHT / 2)
#define CONSOLE_MAXWIDTH  (REAL_SCREENWIDTH)
#define CONSOLE_SCROLL_DOWN_TIME 356.25
#define CONSOLE_SCROLL_UP_TIME   213.75
#define CONSOLE_SCROLL_DOWN_RATE  (CONSOLE_MAXHEIGHT / CONSOLE_SCROLL_DOWN_TIME)
#define CONSOLE_SCROLL_UP_RATE   -(CONSOLE_MAXHEIGHT / CONSOLE_SCROLL_UP_TIME)
#define CONSOLE_CURSOR_THICKNESS 2
#define CONSOLE_PROMPT_THICKNESS 1
#define CONSOLE_MARGIN 4

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

typedef struct console_input_s {
  GString *gstr;
  size_t cursor;
  uint32_t cursor_active;
} console_input_t;

typedef struct console_s {
  buf_t *scrollback;
  console_input_t *input;
  int cursor;
  double scroll_rate;
  double height;
  int max_height;
  int max_width;
  const char *font_description;
  cairo_t *cairo_context;
  cairo_surface_t *cairo_surface;
  PangoContext *pango_context;
  PangoLayout *scrollback_layout;
  PangoLayout *input_layout;
  bool rebuild_scrollback_layout;
  bool rebuild_input_layout;
#ifdef GL_DOOM
  GLuint tex_id;
  bool repaint;
#endif
  unsigned char *pixels;
  bool owns_pixels;
} console_t;

static console_t console;

static bool input_cursor_at_start(void) {
  return console.input->cursor <= 0;
}

static bool input_cursor_at_end(void) {
  return console.input->cursor > (console.input->gstr->len - 1);
}

static bool move_input_cursor_left(void) {
  gchar *n_char;

  if (input_cursor_at_start())
    return false;

  n_char = g_utf8_find_prev_char(
    console.input->gstr->str,
    console.input->gstr->str + console.input->cursor
  );

  if (n_char == NULL)
    return false;

  console.input->cursor = n_char - console.input->gstr->str;

  console.input->cursor_active = I_GetTime();

  return true;
}

static bool move_input_cursor_right(void) {
  gchar *n_char;

  if (input_cursor_at_end())
    return false;

  n_char = g_utf8_find_next_char(
    console.input->gstr->str + console.input->cursor, NULL
  );

  if (n_char == NULL)
    return false;

  console.input->cursor = n_char - console.input->gstr->str;

  console.input->cursor_active = I_GetTime();

  return true;
}

static void normalize_input(void) {
  char *normalized_input = g_utf8_normalize(
    console.input->gstr->str, -1, G_NORMALIZE_DEFAULT
  );

  if (normalized_input == NULL)
    return;

  g_string_assign(console.input->gstr, normalized_input);

  g_free(normalized_input);
}

static void insert_input(const char *new_input) {
  g_string_insert(console.input->gstr, console.input->cursor, new_input);
  normalize_input();
  console.input->cursor += strlen(new_input);

  console.input->cursor_active = I_GetTime();

  console.rebuild_input_layout = true;
}

static void backspace_input(void) {
  gchar *p_char;
  size_t pos;
  size_t size;

  if (input_cursor_at_start())
    return;

  p_char = g_utf8_find_prev_char(
    console.input->gstr->str,
    console.input->gstr->str + console.input->cursor
  );

  if (p_char == NULL)
    return;

  if (p_char < console.input->gstr->str)
    I_Error("backspace_input: previous character exists before string");

  pos = p_char - console.input->gstr->str;

  if (pos >= console.input->cursor)
    I_Error("backspace_input: previous character exists after cursor");

  size = console.input->cursor - pos;

  console.input->cursor -= size;
  g_string_erase(console.input->gstr, pos, size);
  normalize_input();

  console.input->cursor_active = I_GetTime();

  console.rebuild_input_layout = true;
}

static void delete_input(void) {
  gchar *n_char;
  size_t pos;
  size_t size;

  if (input_cursor_at_end())
    return;

  n_char = g_utf8_find_next_char(
    console.input->gstr->str + console.input->cursor,
    NULL
  );

  if (n_char == NULL)
    I_Error("delete_input: no next character exists after cursor");

  if (n_char < console.input->gstr->str)
    I_Error("delete_input: next character exists before string");

  pos = n_char - console.input->gstr->str;

  if (pos < console.input->cursor)
    I_Error("delete_input: next character exists before cursor");

  size = pos - console.input->cursor;

  g_string_erase(console.input->gstr, console.input->cursor, size);
  normalize_input();

  console.input->cursor_active = I_GetTime();

  console.rebuild_input_layout = true;
}

static void clear_input(void) {
  g_string_erase(console.input->gstr, 0, -1);
  console.input->cursor = 0;
  normalize_input();

  console.input->cursor_active = I_GetTime();

  console.rebuild_input_layout = true;
}

static void process_input(void) {
  size_t command_length = console.input->gstr->len;
  char *command = calloc(command_length + 1, sizeof(char));

  if (command == NULL)
    I_Error("process_input: calloc'ing input command failed");

  strncpy(command, console.input->gstr->str, command_length);

  clear_input();

  printf("process_input: processing command [%s]\n", command);

  console.input->cursor_active = I_GetTime();

  free(command);
}

static void build_gl_texture(void) {
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
  PangoFontDescription *desc = pango_font_description_from_string(
    console.font_description
  );

  pango_layout_set_markup(
    console.scrollback_layout, M_BufferGetData(console.scrollback), -1
  );
  pango_layout_set_font_description(console.scrollback_layout, desc);
  pango_layout_set_wrap(console.scrollback_layout, PANGO_WRAP_WORD_CHAR);
  pango_cairo_update_layout(console.cairo_context, console.scrollback_layout);

  pango_font_description_free(desc);
#ifdef GL_DOOM
  console.repaint = true;
#endif
}

static void rebuild_input_layout(void) {
  PangoFontDescription *desc = pango_font_description_from_string(
    console.font_description
  );
  int input_width;
  int input_height;
  int console_input_width;

  pango_layout_get_size(console.input_layout, &input_width, &input_height);
  input_width /= PANGO_SCALE;
  input_height /= PANGO_SCALE;

  console_input_width = console.max_width - (
    CONSOLE_MARGIN +
    CONSOLE_MARGIN +
    (input_height + CONSOLE_MARGIN)
  ) * PANGO_SCALE;

  pango_layout_set_text(console.input_layout, console.input->gstr->str, -1);
  pango_layout_set_font_description(console.input_layout, desc);
  pango_layout_set_ellipsize(console.input_layout, PANGO_ELLIPSIZE_START);
  pango_layout_set_width(console.input_layout, console_input_width);
  pango_cairo_update_layout(console.cairo_context, console.input_layout);

  pango_font_description_free(desc);
#ifdef GL_DOOM
  console.repaint = true;
#endif
}

void C_Printf(const char *fmt, ...) {
  gchar *markup_string;
  va_list args;

  va_start(args, fmt);
  markup_string = g_markup_vprintf_escaped(fmt, args);
  M_BufferWriteString(
    console.scrollback, markup_string, strlen(markup_string)
  );
  g_free(markup_string);
  va_end(args);

  console.rebuild_scrollback_layout = true;
}

void C_MPrintf(const char *fmt, ...) {
  gint size, new_size;
  va_list args;

  va_start(args, fmt);
  size = g_vsnprintf(NULL, 0, fmt, args);
  if (size < 0) {
    perror("C_RawPrintf: g_vsnprintf returned an error: ");
    I_Error("");
  }
  M_BufferEnsureCapacity(console.scrollback, size + 1);
  new_size = g_vsnprintf(
    M_BufferGetDataAtCursor(console.scrollback), size + 1, fmt, args
  );
  if (size < 0) {
    perror(
      "C_RawPrintf: g_vsnprintf returned an error writing to scrollback: "
    );
    I_Error("");
  }
  va_end(args);

  M_BufferSeekForward(console.scrollback, new_size + 1);

  console.rebuild_scrollback_layout = true;
}

void C_Init(void) {
  console.scrollback = M_BufferNew();
  console.input = malloc(sizeof(console_input_t));
  console.input->gstr = g_string_new("");
  console.input->cursor = 0;
  console.input->cursor_active = 0;
  console.font_description = "FreeSans 8";

  C_Reset();

  clear_input();
  // insert_input(TEST_INPUT_TEXT);
  C_MPrintf(TEST_SCROLLBACK_TEXT);

  build_gl_texture();
}

void C_Reset(void) {
  cairo_status_t status;
  // cairo_font_options_t *font_options = cairo_font_options_create();

  console.scroll_rate = 0.0;
  console.height = 0.0;
  console.max_height = CONSOLE_MAXHEIGHT;
  console.max_width = CONSOLE_MAXWIDTH;

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

  if (console.scrollback_layout)
    g_object_unref(console.scrollback_layout);

  console.scrollback_layout = pango_layout_new(console.pango_context);
  pango_layout_set_width(
    console.scrollback_layout, console.max_width * PANGO_SCALE
  );
  pango_layout_set_wrap(console.scrollback_layout, PANGO_WRAP_WORD_CHAR);

  if (console.input_layout)
    g_object_unref(console.input_layout);

  console.input_layout = pango_layout_new(console.pango_context);
  pango_layout_set_width(
    console.input_layout, console.max_width * PANGO_SCALE
  );

  console.rebuild_scrollback_layout = true;
  console.rebuild_input_layout = true;

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
  int sb_width;
  int sb_height;
  int cursor_visible;
  uint32_t current_tic = I_GetTime();

  if (V_GetMode() == VID_MODE32 && SDL_MUSTLOCK(screen))
    SDL_LockSurface(screen);

#ifdef GL_DOOM
  if (V_GetMode() == VID_MODEGL)
    cairo_set_operator(console.cairo_context, CAIRO_OPERATOR_SOURCE);
#endif

  if (console.rebuild_scrollback_layout)
    rebuild_scrollback_layout();

  if (console.rebuild_input_layout)
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

  pango_layout_get_size(console.input_layout, &input_width, &input_height);
  input_width /= PANGO_SCALE;
  input_height /= PANGO_SCALE;
  input_height_frac = input_height / input_height_fracunit;
  input_height_fracedge = input_height_frac * (input_height_fracunit - 1.0);
  cairo_move_to(
    console.cairo_context,
    CONSOLE_MARGIN + (input_height + CONSOLE_MARGIN),
    console.height - (input_height + CONSOLE_MARGIN)
  );
  pango_cairo_show_layout(console.cairo_context, console.input_layout);

  cairo_move_to(
    console.cairo_context,
    CONSOLE_MARGIN + input_height_frac,
    console.height - ((input_height_fracedge) + CONSOLE_MARGIN)
  );

  cairo_line_to(
    console.cairo_context,
    CONSOLE_MARGIN + (input_height_fracedge),
    console.height - ((input_height >> 1) + CONSOLE_MARGIN)
  );
  cairo_line_to(
    console.cairo_context,
    CONSOLE_MARGIN + input_height_frac,
    console.height - (CONSOLE_MARGIN + input_height_frac)
  );
  cairo_set_line_width(console.cairo_context, CONSOLE_PROMPT_THICKNESS);
  cairo_stroke(console.cairo_context);

  current_tic = I_GetTime();

  cursor_visible = (
    ((current_tic % TICRATE) >= ((TICRATE / 2))) ||
    (console.input->cursor_active >= (current_tic - TICRATE))
  );

  if (cursor_visible) {
    PangoRectangle pos;
    int x;
    int y;
    int width;
    int height;

    pango_layout_index_to_pos(
      console.input_layout, console.input->cursor, &pos
    );

    x      = pos.x / PANGO_SCALE;
    y      = pos.y / PANGO_SCALE;
    width  = pos.width / PANGO_SCALE;
    height = pos.height / PANGO_SCALE;

    x += (CONSOLE_MARGIN + (input_height + CONSOLE_MARGIN));
    y += (console.height - (input_height + CONSOLE_MARGIN));

    cairo_set_source_rgba(console.cairo_context, 0.8f, 0.8f, 0.8f, 1.0f);
    cairo_move_to(console.cairo_context, x, y);
    cairo_line_to(console.cairo_context, x, y + height);
    cairo_set_line_width(console.cairo_context, CONSOLE_CURSOR_THICKNESS);
    cairo_stroke(console.cairo_context);

    cairo_set_source_rgba(console.cairo_context, 1.0f, 1.0f, 1.0f, 1.0f);
  }

  pango_layout_get_size(console.scrollback_layout, &sb_width, &sb_height);
  sb_width /= PANGO_SCALE;
  sb_height /= PANGO_SCALE;
  cairo_move_to(
    console.cairo_context,
    CONSOLE_MARGIN,
    console.height - ((input_height * 2) + sb_height)
  );

  pango_cairo_show_layout(console.cairo_context, console.scrollback_layout);

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

  if ((ev->type != ev_keydown) && (ev->type != ev_mouse))
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

/* vi: set et ts=2 sw=2: */

