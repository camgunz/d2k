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

#include "doomstat.h"
#include "d_event.h"
#include "d_main.h"
#include "hu_lib.h"
#include "hu_msg.h"
#include "hu_stuff.h"
#include "i_system.h"
#include "i_video.h"
#include "lprintf.h"

#define RETRACTION_TIME     666
#define RETRACTION_TIMEOUT 2000

struct message_widget_s {
  GString *buf;
  PangoContext *layout_context;
  PangoLayout *layout;
  int base_line_height;
  int base_layout_width;
  int base_layout_height;
  const char *font_description;
  int x;
  int y;
  int width;
  int height;
  hu_color_t fg_color;
  hu_color_t bg_color;
  int layout_width;
  int layout_height;
  bool needs_rebuilding;
  int align_bottom;
  double offset;
  int line_count;
  bool scrollable;
  int scroll_amount;
  bool retractable;
  double retraction_rate;
  uint64_t last_retraction;
  int retraction_timeout;
  double retraction_target;
};

static void scroll_up(message_widget_t *mw) {
  mw->offset += mw->scroll_amount;
}

static void scroll_down(message_widget_t *mw) {
  mw->offset -= mw->scroll_amount;

  if (mw->offset <= 0)
    mw->offset = 0;
}

static unsigned int get_line_count(message_widget_t *mw) {
  HU_MessageWidgetRebuild(mw, I_GetRenderContext());

  return pango_layout_get_line_count(mw->layout);
}

static float get_average_line_height(message_widget_t *mw) {
  int layout_width;
  int layout_height;
  bool was_blank = false;
  float line_height;

  if (mw->line_count == 0) {
    g_string_append(mw->buf, "Doom");
    mw->line_count++;
    was_blank = true;
  }

  HU_MessageWidgetGetLayoutSize(mw, &layout_width, &layout_height);

  line_height = ((float)layout_height / (float)mw->line_count);

  if (was_blank) {
    g_string_erase(mw->buf, 0, -1);
    mw->line_count--;
  }

  return line_height;
}

static void reset_retraction_target(message_widget_t *mw, bool reset_timeout) {
  mw->last_retraction = 0;
  mw->retraction_target = 0.0;

  if (reset_timeout)
    mw->retraction_timeout = RETRACTION_TIMEOUT;
  else
    mw->retraction_timeout = 0;
}

static void calculate_line_height(gpointer data, gpointer user_data) {
  PangoLayoutLine *line = (PangoLayoutLine *)data;
  int *line_height = (int *)user_data;
  PangoRectangle rect;

  pango_layout_line_get_pixel_extents(line, NULL, &rect);
  *line_height = rect.height;
}

static void calculate_base_layout_dimensions(message_widget_t *mw) {
  GString *buf = mw->buf;
  GSList *lines = pango_layout_get_lines_readonly(mw->layout);

  mw->buf = g_string_new("");

  HU_MessageWidgetGetLayoutSize(
    mw, &mw->base_layout_width, &mw->base_layout_height
  );

  g_slist_foreach(lines, calculate_line_height, &mw->base_line_height);

  g_string_free(mw->buf, true);

  mw->buf = buf;

  if (mw->offset <= 0.0)
    mw->offset = mw->base_line_height;
}

message_widget_t* HU_MessageWidgetNew(void *render_context, int x, int y,
                                                            int w, int h,
                                                            int scroll_amount) {
  return HU_MessageWidgetNewBuf(
    render_context, g_string_new(""), x, y, w, h, scroll_amount
  );
}

message_widget_t* HU_MessageWidgetNewBuf(void *render_context, void *buf,
                                         int x, int y,
                                         int w, int h,
                                         int scroll_amount) {
  hu_color_t clear = {0.0f, 0.0f, 0.0f, 0.0f};
  hu_color_t white = {1.0f, 1.0f, 1.0f, 1.0f};
  message_widget_t *mw = calloc(1, sizeof(message_widget_t));

  if (mw == NULL)
    I_Error("HU_MessageWidgetNew: calloc failed");

  mw->buf = (GString *)buf;
  mw->layout_context = NULL;
  mw->layout = NULL;
  mw->font_description = HU_FONT;
  mw->x = x;
  mw->y = y;
  mw->width = w;
  mw->height = h;
  mw->fg_color = white;
  mw->bg_color = clear;
  mw->layout_width = 0;
  mw->layout_height = 0;
  mw->scroll_amount = scroll_amount;
  mw->retractable = false;
  mw->scrollable = false;
  mw->retraction_rate = 0.0;
  mw->last_retraction = 0;
  mw->retraction_timeout = 0;
  mw->retraction_target = 0;
  mw->line_count = 0;

  HU_MessageWidgetReset(mw, render_context);

  return mw;
}

void HU_MessageWidgetSetBuf(message_widget_t *mw, void *buf) {
  mw->buf = (GString *)buf;
}

void HU_MessageWidgetReset(message_widget_t *mw, void *render_context) {
  cairo_font_options_t *font_options = cairo_font_options_create();
  PangoFontMap *dfm = pango_cairo_font_map_get_default();
  cairo_font_type_t ft = pango_cairo_font_map_get_font_type((PangoCairoFontMap *)dfm);
  PangoFontMap *fm = pango_cairo_font_map_new_for_font_type(ft);

  if (mw->layout)
    g_object_unref(mw->layout);

  if (mw->layout_context)
    g_object_unref(mw->layout_context);

  mw->layout_context = pango_font_map_create_context(fm);
  pango_cairo_context_set_resolution(mw->layout_context, 96.0);

  cairo_font_options_set_hint_style(font_options, CAIRO_HINT_STYLE_FULL);
  cairo_font_options_set_hint_metrics(font_options, CAIRO_HINT_METRICS_ON);
  cairo_font_options_set_antialias(font_options, CAIRO_ANTIALIAS_SUBPIXEL);
  pango_cairo_context_set_font_options(mw->layout_context, font_options);
  cairo_font_options_destroy(font_options);

  pango_cairo_update_context(render_context, mw->layout_context);

  mw->layout = pango_layout_new(mw->layout_context);

  mw->base_line_height = -1;
  mw->base_layout_width = -1;
  mw->base_layout_height = -1;

  mw->offset = -1;

  mw->needs_rebuilding = true;
}

void HU_MessageWidgetSetAlignBottom(message_widget_t *mw, bool align_bottom) {
  mw->align_bottom = align_bottom;
}

void HU_MessageWidgetGetSize(message_widget_t *mw, int *width, int *height) {
  *width = mw->width;
  *height = mw->height;
}

void HU_MessageWidgetSetSize(message_widget_t *mw, int width, int height) {
  mw->width = width;
  mw->height = height;
  mw->needs_rebuilding = true;
}

void HU_MessageWidgetGetPosition(message_widget_t *mw, int *x, int *y) {
  *x = mw->x;
  *y = mw->y;
}

void HU_MessageWidgetSetPosition(message_widget_t *mw, int x, int y) {
  mw->x = x;
  mw->y = y;
}

void HU_MessageWidgetGetLayoutSize(message_widget_t *mw, int *width, int *height) {
  int layout_width;
  int layout_height;

  pango_layout_get_size(mw->layout, &layout_width, &layout_height);

  *width  = layout_width  / PANGO_SCALE;
  *height = layout_height / PANGO_SCALE;
}

void HU_MessageWidgetSetFGColor(message_widget_t *mw, hu_color_t fg_color) {
  mw->fg_color = fg_color;
}

void HU_MessageWidgetSetBGColor(message_widget_t *mw, hu_color_t bg_color) {
  mw->bg_color = bg_color;
}

void HU_MessageWidgetSetFont(message_widget_t *mw, const char *font) { 
  mw->font_description = font;
  mw->needs_rebuilding = true;
}

void HU_MessageWidgetSetScrollAmount(message_widget_t *mw, int scroll_amount) {
  mw->scroll_amount = scroll_amount;
}

void HU_MessageWidgetSetHeightByLines(message_widget_t *mw, int lines) {
  int widget_width;
  int widget_height;
  float line_height = get_average_line_height(mw);

  HU_MessageWidgetGetSize(mw, &widget_width, &widget_height);
  HU_MessageWidgetSetSize(mw, widget_width, line_height * lines);
}

bool HU_MessageWidgetHasContent(message_widget_t *mw) {
  return mw->buf->len > 0;
}

void HU_MessageWidgetRebuild(message_widget_t *mw, void *render_context) {
  PangoFontDescription *desc;

  desc = pango_font_description_from_string(mw->font_description);

  pango_layout_set_attributes(mw->layout, NULL);
  pango_layout_set_markup(mw->layout, mw->buf->str, -1);
  pango_layout_set_font_description(mw->layout, desc);
  pango_layout_set_width(mw->layout, mw->width * PANGO_SCALE);
  pango_layout_set_wrap(mw->layout, PANGO_WRAP_WORD_CHAR);
  pango_cairo_update_layout(render_context, mw->layout);

  pango_font_description_free(desc);
}

void HU_MessageWidgetDrawer(message_widget_t *mw, void *render_context) {
  cairo_t *cr = (cairo_t *)render_context;
  int width;
  int height;
  hu_color_t *bg = &mw->bg_color;
  hu_color_t *fg = &mw->fg_color;

  if (mw->needs_rebuilding)
    HU_MessageWidgetRebuild(mw, cr);

  if (mw->base_line_height == -1)
    calculate_base_layout_dimensions(mw);

  cairo_save(cr);

  cairo_set_operator(cr, CAIRO_OPERATOR_OVER);

  cairo_reset_clip(cr);
  cairo_new_path(cr);
  cairo_rectangle(cr, mw->x, mw->y, mw->width, mw->height);
  cairo_clip(cr);

  cairo_set_source_rgba(cr, bg->r, bg->g, bg->b, bg->a);
  cairo_paint(cr);

  cairo_set_source_rgba(cr, fg->r, fg->g, fg->b, fg->a);
  HU_MessageWidgetGetLayoutSize(mw, &width, &height);

  if (mw->scrollable)
    mw->offset = MIN(mw->offset, height);

  if (mw->align_bottom)
    cairo_move_to(cr, mw->x, mw->y + mw->height - height + mw->offset);
  else if (mw->retractable)
    cairo_move_to(cr, mw->x, mw->y + mw->offset - height);
  else
    cairo_move_to(cr, mw->x, mw->y - mw->offset);

  pango_cairo_show_layout(cr, mw->layout);

  cairo_restore(cr);
}

void HU_MessageWidgetSetScrollable(message_widget_t *mw, bool scrollable) {
  if (mw->scrollable == scrollable)
    return;

  mw->scrollable = scrollable;

  if (scrollable && mw->retractable)
    HU_MessageWidgetSetRetractable(mw, false);
}

void HU_MessageWidgetSetRetractable(message_widget_t *mw, bool retractable) {
  if (mw->retractable == retractable)
    return;

  mw->retractable = retractable;

  if (retractable) {
    if (mw->scrollable)
      HU_MessageWidgetSetScrollable(mw, false);

    mw->retraction_rate = get_average_line_height(mw) / RETRACTION_TIME;
    mw->offset = mw->base_line_height;
  }
  else {
    mw->retraction_rate = 0.0;
  }
}

void HU_MessageWidgetRetract(message_widget_t *mw) {
  mw->offset = mw->base_line_height;
}

void HU_MessageWidgetTicker(message_widget_t *mw) {
  uint64_t current_time;
  uint64_t time_elapsed;

  return;

  mw->line_count = get_line_count(mw);

  if (!mw->retractable)
    return;

  if (mw->offset == mw->base_line_height)
    return;

  current_time = I_GetTicks();

  if (mw->last_retraction == 0)
    mw->last_retraction = current_time;

  time_elapsed = current_time - mw->last_retraction;

  mw->last_retraction = current_time;

  if (mw->retraction_timeout)
    mw->retraction_timeout -= time_elapsed;

  if (mw->retraction_timeout < 0)
    mw->retraction_timeout = 0;

  if (mw->retraction_timeout != 0)
    return;

  mw->offset -= (mw->retraction_rate * time_elapsed);

  if (mw->offset > mw->retraction_target)
    return;

  mw->offset = mw->retraction_target;

  if (mw->offset <= mw->base_line_height) {
    mw->offset = mw->base_line_height;
    return;
  }

  reset_retraction_target(mw, false);
}

bool HU_MessageWidgetResponder(message_widget_t *mw, event_t *ev) {
  if (!mw->scrollable)
    return false;

  if (!((ev->type == ev_keydown) || (ev->type == ev_mouse)))
    return false;

  if (ev->data1 == SDLK_PAGEUP && keybindings.shiftdown) {
    scroll_up(mw);
    return true;
  }

  if (ev->data1 == SDLK_PAGEDOWN && keybindings.shiftdown) {
    scroll_down(mw);
    return true;
  }

  return false;
}

void HU_MessageWidgetTextAppended(message_widget_t *mw) {
  int line_count;
  int total_line_height = 0;
  int widget_width;
  int widget_height;
  PangoLayoutIter *it;
  PangoRectangle rect;

  if (!mw->retractable)
    return;

  HU_MessageWidgetGetSize(mw, &widget_width, &widget_height);

  it = pango_layout_get_iter(mw->layout);

  do {
    PangoLayoutLine *line = pango_layout_iter_get_line_readonly(it);
    PangoRectangle rect;

    pango_layout_line_get_pixel_extents(line, NULL, &rect);
    total_line_height += rect.height;
  } while (pango_layout_iter_next_line(it));

  mw->offset += total_line_height;
  mw->offset = MIN(mw->offset, widget_height);

  if (mw->offset == widget_height)
    mw->offset += mw->y;

  reset_retraction_target(mw, true);
}

void HU_MessageWidgetPrintf(message_widget_t *mw, const char *fmt, ...) {
  gchar *markup_string;
  va_list args;
  
  if (fmt == NULL)
    return;

  va_start(args, fmt);

  markup_string = g_markup_vprintf_escaped(fmt, args);

  g_string_append(mw->buf, markup_string);
  g_free(markup_string);

  va_end(args);

  HU_MessageWidgetTextAppended(mw);

  mw->needs_rebuilding = true;
}

void HU_MessageWidgetVPrintf(message_widget_t *mw, const char *fmt,
                                                   va_list args) {
  gchar *markup_string;

  if (fmt == NULL)
    return;

  markup_string = g_markup_vprintf_escaped(fmt, args);

  g_string_append(mw->buf, markup_string);
  g_free(markup_string);

  HU_MessageWidgetTextAppended(mw);

  mw->needs_rebuilding = true;
}

void HU_MessageWidgetMPrintf(message_widget_t *mw, const char *fmt, ...) {
  va_list args;

  if (fmt == NULL)
    return;

  va_start(args, fmt);

  g_string_append_vprintf(mw->buf, fmt, args);

  va_end(args);

  HU_MessageWidgetTextAppended(mw);

  mw->needs_rebuilding = true;
}

void HU_MessageWidgetMVPrintf(message_widget_t *mw, const char *fmt,
                                                    va_list args) {
  if (fmt == NULL)
    return;

  g_string_append_vprintf(mw->buf, fmt, args);

  HU_MessageWidgetTextAppended(mw);

  mw->needs_rebuilding = true;
}

void HU_MessageWidgetEcho(message_widget_t *mw, const char *message) {
  gchar *markup_message;

  if (message == NULL)
    return;
  
  markup_message = g_markup_escape_text(message, -1);

  g_string_append(mw->buf, markup_message);
  g_string_append(mw->buf, "\n");

  g_free(markup_message);

  HU_MessageWidgetTextAppended(mw);

  mw->needs_rebuilding = true;
}

void HU_MessageWidgetMEcho(message_widget_t *mw, const char *message) {
  if (message == NULL)
    return;

  g_string_append(mw->buf, message);
  g_string_append(mw->buf, "\n");

  HU_MessageWidgetTextAppended(mw);

  mw->needs_rebuilding = true;
}

void HU_MessageWidgetWrite(message_widget_t *mw, const char *message) {
  gchar *markup_message;

  if (message == NULL)
    return;
  
  markup_message = g_markup_escape_text(message, -1);

  g_string_append(mw->buf, markup_message);

  g_free(markup_message);

  HU_MessageWidgetTextAppended(mw);

  mw->needs_rebuilding = true;
}

void HU_MessageWidgetMWrite(message_widget_t *mw, const char *message) {
  if (message == NULL)
    return;

  g_string_append(mw->buf, message);

  HU_MessageWidgetTextAppended(mw);

  mw->needs_rebuilding = true;
}

void HU_MessageWidgetClear(message_widget_t *mw) {
  g_string_erase(mw->buf, 0, -1);
}

/* vi: set et ts=2 sw=2: */

