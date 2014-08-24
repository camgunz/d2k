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
#include <SDL.h>

#include "d_event.h"
#include "hu_lib.h"
#include "hu_input.h"
#include "hu_stuff.h"
#include "i_main.h"
#include "lprintf.h"

#define INPUT_CURSOR_THICKNESS 2
#define INPUT_PROMPT_THICKNESS 1

struct input_widget_s {
  GString *buf;
  PangoContext *layout_context;
  PangoLayout *layout;
  const char *font_description;
  int x;
  int y;
  int width;
  int height;
  hu_color_t fg_color;
  hu_color_t bg_color;
  hu_color_t curs_color;
  int layout_width;
  int layout_height;
  size_t cursor;
  uint32_t cursor_active;
  InputHandler handleInput;
  bool needs_rebuilding;
};

static bool cursor_at_start(input_widget_t *iw) {
  return iw->cursor <= 0;
}

static bool cursor_at_end(input_widget_t *iw) {
  return iw->cursor >= iw->buf->len;
}

static void activate_cursor(input_widget_t *iw) {
  iw->cursor_active = I_GetTime();
}

static bool move_cursor_left(input_widget_t *iw) {
  gchar *n_char;

  if (cursor_at_start(iw))
    return false;

  n_char = g_utf8_find_prev_char(
    iw->buf->str,
    iw->buf->str + iw->cursor
  );

  if (n_char == NULL)
    return false;

  iw->cursor = n_char - iw->buf->str;

  activate_cursor(iw);

  iw->needs_rebuilding = true;

  return true;
}

static bool move_cursor_right(input_widget_t *iw) {
  gchar *n_char;

  if (cursor_at_end(iw))
    return false;

  n_char = g_utf8_find_next_char(iw->buf->str + iw->cursor, NULL);

  if (n_char == NULL)
    return false;

  iw->cursor = n_char - iw->buf->str;

  activate_cursor(iw);

  iw->needs_rebuilding = true;

  return true;
}

static void normalize(input_widget_t *iw) {
  char *normalized_string = g_utf8_normalize(
    iw->buf->str, -1, G_NORMALIZE_DEFAULT
  );

  if (normalized_string == NULL)
    return;

  g_string_assign(iw->buf, normalized_string);

  g_free(normalized_string);
}

static void insert_text(input_widget_t *iw, const char *new_text) {
  g_string_insert(iw->buf, iw->cursor, new_text);
  normalize(iw);
  iw->cursor += strlen(new_text);

  activate_cursor(iw);

  iw->needs_rebuilding = true;
}

static void backspace(input_widget_t *iw) {
  gchar *p_char;
  size_t pos;
  size_t size;

  if (cursor_at_start(iw))
    return;

  p_char = g_utf8_find_prev_char(iw->buf->str, iw->buf->str + iw->cursor);

  if (p_char == NULL)
    return;

  if (p_char < iw->buf->str)
    I_Error("backspace: previous character exists before string");

  pos = p_char - iw->buf->str;

  if (pos >= iw->cursor)
    I_Error("backspace: previous character exists after cursor");

  size = iw->cursor - pos;

  iw->cursor -= size;
  g_string_erase(iw->buf, pos, size);
  normalize(iw);

  activate_cursor(iw);

  iw->needs_rebuilding = true;
}

static void delete(input_widget_t *iw) {
  gchar *n_char;
  size_t pos;
  size_t size;

  if (cursor_at_end(iw))
    return;

  n_char = g_utf8_find_next_char(iw->buf->str + iw->cursor, NULL);

  if (n_char == NULL)
    I_Error("delete: no next character exists after cursor");

  if (n_char < iw->buf->str)
    I_Error("delete: next character exists before string");

  pos = n_char - iw->buf->str;

  if (pos < iw->cursor)
    I_Error("delete: next character exists before cursor");

  size = pos - iw->cursor;

  g_string_erase(iw->buf, iw->cursor, size);
  normalize(iw);

  activate_cursor(iw);

  iw->needs_rebuilding = true;
}

static void clear(input_widget_t *iw) {
  g_string_erase(iw->buf, 0, -1);
  iw->cursor = 0;
  normalize(iw);

  activate_cursor(iw);

  iw->needs_rebuilding = true;
}

static void load_previous_history_line(input_widget_t *iw) {
  puts("Loading previous history line");
}

static void load_next_history_line(input_widget_t *iw) {
  puts("Loading next history line");
}

input_widget_t* HU_InputWidgetNew(void *render_context, int x, int y,
                                                        int w, int h,
                                                        InputHandler handler) {
  hu_color_t white = {1.0f, 1.0f, 1.0f, 1.0f};
  hu_color_t clear = {0.0f, 0.0f, 0.0f, 0.0f};
  hu_color_t grey  = {0.8f, 0.8f, 0.8f, 1.0f};
  input_widget_t *iw = calloc(1, sizeof(input_widget_t));

  if (iw == NULL)
    I_Error("HU_InputWidgetNew: calloc failed");

  iw->buf = g_string_new("");
  iw->layout_context = NULL;
  iw->layout = NULL;
  iw->font_description = HU_FONT;
  iw->x = x;
  iw->y = y;
  iw->width = w;
  iw->height = h;
  iw->fg_color   = white;
  iw->bg_color   = clear;
  iw->curs_color = grey;
  iw->cursor = 0;
  iw->cursor_active = 0;
  iw->handleInput = handler;

  HU_InputWidgetReset(iw, render_context);

  return iw;
}

void HU_InputWidgetReset(input_widget_t *iw, void *render_context) {
  int layout_width;
  int layout_height;
  cairo_font_options_t *font_options = cairo_font_options_create();
  PangoFontMap *fm = pango_cairo_font_map_get_default();

  if (iw->layout)
    g_object_unref(iw->layout);

  if (iw->layout_context)
    g_object_unref(iw->layout_context);

  iw->layout_context = pango_font_map_create_context(fm);
  pango_cairo_context_set_resolution(iw->layout_context, 96.0);

  cairo_font_options_set_hint_style(font_options, CAIRO_HINT_STYLE_FULL);
  cairo_font_options_set_hint_metrics(font_options, CAIRO_HINT_METRICS_ON);
  cairo_font_options_set_antialias(font_options, CAIRO_ANTIALIAS_SUBPIXEL);
  pango_cairo_context_set_font_options(iw->layout_context, font_options);
  cairo_font_options_destroy(font_options);

  pango_cairo_update_context(render_context, iw->layout_context);

  iw->layout = pango_layout_new(iw->layout_context);

  if (iw->buf->len <= 0) {
    g_string_append(iw->buf, "DOOM");
    HU_InputWidgetRebuild(iw, render_context);
    HU_InputWidgetGetLayoutSize(iw, &layout_width, &layout_height);
    iw->layout_width = 0;
    iw->layout_height = layout_height;
    g_string_erase(iw->buf, 0, -1);
  }
  else {
    HU_InputWidgetGetLayoutSize(iw, &layout_width, &layout_height);
    iw->layout_width = 0;
    iw->layout_height = layout_height;
  }

  iw->needs_rebuilding = true;
}

void HU_InputWidgetGetSize(input_widget_t *iw, int *width, int *height) {
  *width = iw->width;
  *height = iw->height;
}

void HU_InputWidgetSetSize(input_widget_t *iw, int width, int height) {
  iw->width = width;
  iw->height = height;
  iw->needs_rebuilding = true;
}

void HU_InputWidgetGetPosition(input_widget_t *iw, int *x, int *y) {
  *x = iw->x;
  *y = iw->y;
}

void HU_InputWidgetSetPosition(input_widget_t *iw, int x, int y) {
  iw->x = x;
  iw->y = y;
}

void HU_InputWidgetGetLayoutSize(input_widget_t *iw, int *width, int *height) {
  int input_width;
  int input_height;

  pango_layout_get_size(iw->layout, &input_width, &input_height);

  *width  = input_width  / PANGO_SCALE;
  *height = input_height / PANGO_SCALE;
}

void HU_InputWidgetSetFGColor(input_widget_t *iw, hu_color_t fg_color) {
  iw->fg_color = fg_color;
}

void HU_InputWidgetSetBGColor(input_widget_t *iw, hu_color_t bg_color) {
  iw->bg_color = bg_color;
}

void HU_InputWidgetSetCursorColor(input_widget_t *iw, hu_color_t curs_color) {
  iw->curs_color = curs_color;
}

void HU_InputWidgetSetFont(input_widget_t *iw, const char *font) { 
  iw->font_description = font;
  iw->needs_rebuilding = true;
}

void HU_InputWidgetSetHeightByLines(input_widget_t *iw, int lines) {
  int layout_width;
  int layout_height;
  int widget_width;
  int widget_height;
  guint line_count = g_slist_length(pango_layout_get_lines_readonly(
    iw->layout
  ));
  bool was_blank = false;

  if (line_count == 0) {
    g_string_append(iw->buf, "Doom");
    line_count = 1;
    was_blank = true;
  }

  HU_InputWidgetGetLayoutSize(iw, &layout_width, &layout_height);
  HU_InputWidgetGetSize(iw, &widget_width, &widget_height);
  HU_InputWidgetSetSize(
    iw,
    widget_width,
    ((float)layout_height / (float)line_count) * lines
  );

  if (was_blank)
    g_string_erase(iw->buf, 0, -1);
}

void HU_InputWidgetRebuild(input_widget_t *iw, void *render_context) {
  PangoFontDescription *desc;
  int width;
  int height;

  desc = pango_font_description_from_string(iw->font_description);

  HU_InputWidgetGetSize(iw, &width, &height);

  pango_layout_set_attributes(iw->layout, NULL);
  pango_layout_set_text(iw->layout, iw->buf->str, -1);
  pango_layout_set_font_description(iw->layout, desc);
  pango_layout_set_width(iw->layout, -1);
  pango_layout_set_height(iw->layout, iw->height * PANGO_SCALE);
  pango_cairo_update_layout(render_context, iw->layout);

  pango_font_description_free(desc);
}

void HU_InputWidgetDrawer(input_widget_t *iw, void *render_context) {
  cairo_t *cr = (cairo_t *)render_context;
  hu_color_t *bg = &iw->bg_color;
  hu_color_t *fg = &iw->fg_color;
  hu_color_t *cfg = &iw->curs_color;
  double ih = iw->layout_height;
  double ih_fracunit = 4.0;
  double ih_frac = ih / ih_fracunit;
  double ih_half = ih / 2.0;
  PangoRectangle cpos;
  int cx;
  int cy;
  int cheight;
  int offset;
  uint32_t current_time = I_GetTime();
  bool cursor_visible;

  if (iw->needs_rebuilding)
    HU_InputWidgetRebuild(iw, cr);

  cairo_save(cr);

  cairo_set_operator(cr, CAIRO_OPERATOR_OVER);

  cairo_reset_clip(cr);
  cairo_new_path(cr);
  cairo_rectangle(cr, iw->x, iw->y, iw->width, iw->height);
  cairo_clip(cr);

  cairo_set_source_rgba(cr, bg->r, bg->g, bg->b, bg->a);
  cairo_paint(cr);

  cairo_set_source_rgba(cr, fg->r, fg->g, fg->b, fg->a);
  cairo_move_to(cr, iw->x, iw->y + ih_frac);
  cairo_line_to(cr, iw->x + ih_half, iw->y + ih_half);
  cairo_line_to(cr, iw->x, iw->y + (ih - ih_frac));
  cairo_set_line_width(cr, INPUT_PROMPT_THICKNESS);
  cairo_stroke(cr);

  pango_layout_index_to_pos(iw->layout, iw->cursor, &cpos);
  cx      = (cpos.x / PANGO_SCALE) + iw->x + ih;
  cy      = (cpos.y / PANGO_SCALE) + iw->y;
  cheight = cpos.height / PANGO_SCALE;

  if (cx < (iw->x + ih))
    offset = (iw->x + ih) - cx;
  else if (cx > (iw->x + iw->width))
    offset = (iw->x + iw->width) - cx;
  else
    offset = 0;

  cx += offset;

  cursor_visible = (
    ((current_time % TICRATE) >= ((TICRATE / 2))) ||
    (iw->cursor_active <= (current_time - 500))
  );

  if (cursor_visible) {
    cairo_set_source_rgba(cr, cfg->r, cfg->g, cfg->b, cfg->a);
    cairo_move_to(cr, cx, cy);
    cairo_line_to(cr, cx, cy + cheight);
    cairo_set_line_width(cr, INPUT_CURSOR_THICKNESS);
    cairo_stroke(cr);
    cairo_set_source_rgba(cr, fg->r, fg->g, fg->b, fg->a);
  }

  cairo_reset_clip(cr);
  cairo_new_path(cr);
  cairo_rectangle(cr, iw->x + ih, iw->y, iw->width - ih, iw->height);
  cairo_clip(cr);

  cairo_move_to(cr, iw->x + ih + offset, iw->y);
  pango_cairo_show_layout(cr, iw->layout);

  cairo_restore(cr);
}

bool HU_InputWidgetResponder(input_widget_t *iw, event_t *ev) {
  char *us;
  size_t us_len;
  gunichar *ucss;
  glong char_count;
  GError *error = NULL;
  bool unprintable = false;

  if (!((ev->type == ev_keydown) || (ev->type == ev_mouse)))
    return false;

  if (ev->data1 == SDLK_RETURN) {
    iw->handleInput(iw);
    clear(iw);
    activate_cursor(iw);
    return true;
  }

  if (ev->data1 == SDLK_DELETE) {
    delete(iw);
    return true;
  }

  if (ev->data1 == SDLK_BACKSPACE) {
    backspace(iw);
    return true;
  }

  if (ev->data1 == SDLK_LEFT) {
    move_cursor_left(iw);
    return true;
  }

  if (ev->data1 == SDLK_RIGHT) {
    move_cursor_right(iw);
    return true;
  }

  if (ev->data1 == SDLK_HOME) {
    iw->cursor = 0;
    return true;
  }

  if (ev->data1 == SDLK_END) {
    iw->cursor = iw->buf->len;
    return true;
  }

  if (ev->data1 == SDLK_UP) {
    load_previous_history_line(iw);
    return true;
  }

  if (ev->data1 == SDLK_DOWN) {
    load_next_history_line(iw);
    return true;
  }

  us = g_locale_to_utf8(
    (const char *)&ev->wchar, sizeof(uint16_t), NULL, &us_len, &error
  );

  if (us == NULL) {
    I_Error("C_Responder: Error converting local input to UTF-8: %s (%d)",
      error->message, error->code
    );
  }

  ucss = g_utf8_to_ucs4_fast(us, -1, &char_count);

  for (glong i = 0; i < char_count; i++) {
    if (!g_unichar_isprint(ucss[i])) {
      unprintable = true;
      break;
    }
  }

  g_free(ucss);

  if (unprintable)
    lprintf(LO_WARN, "Unprintable text in console input\n");
  else
    insert_text(iw, us);

  g_free(us);

  return true;
}

char* HU_InputWidgetGetText(input_widget_t *iw) {
  return iw->buf->str;
}

size_t HU_InputWidgetGetTextLength(input_widget_t *iw) {
  return iw->buf->len;
}

void HU_InputWidgetClear(input_widget_t *iw) {
  g_string_erase(iw->buf, 0, -1);
  iw->cursor = 0;
  activate_cursor(iw);
}

/* vi: set et ts=2 sw=2: */

