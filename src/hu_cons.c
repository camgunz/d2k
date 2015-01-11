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
#include "lprintf.h"
#include "v_video.h"

#define CONSOLE_MAXHEIGHT (REAL_SCREENHEIGHT / 2)
#define CONSOLE_MAXWIDTH  (REAL_SCREENWIDTH)
#define CONSOLE_SCROLL_DOWN_TIME 150.0
#define CONSOLE_SCROLL_UP_TIME   150.0
#define CONSOLE_SCROLL_DOWN_RATE  (CONSOLE_MAXHEIGHT / CONSOLE_SCROLL_DOWN_TIME)
#define CONSOLE_SCROLL_UP_RATE   -(CONSOLE_MAXHEIGHT / CONSOLE_SCROLL_UP_TIME)
#define CONSOLE_MARGIN 8
#define CONSOLE_SCROLLBACK_AMOUNT (CONSOLE_MAXHEIGHT / 4)

struct console_widget_s {
  message_widget_t *scrollback;
  input_widget_t *input;
  double scroll_rate;
  double height;
  int max_width;
  int max_height;
};

console_widget_t* HU_ConsoleWidgetNew(void *render_context,
                                      int x, int y, int w, int h,
                                      InputHandler handler) {
  int input_width;
  int input_height;
  hu_color_t white = {1.0f, 1.0f, 1.0f, 1.0f};
  hu_color_t clear = {0.0f, 0.0f, 0.0f, 0.0f};
  console_widget_t *cons = calloc(1, sizeof(console_widget_t));

  if (cons == NULL)
    I_Error("HU_ConsoleWidgetNew: calloc failed");

  cons->scroll_rate = 0.0;
  cons->height = 0.0;
  cons->max_width = w;
  cons->max_height = h;

  cons->scrollback = HU_MessageWidgetNew(
    render_context,
    x + CONSOLE_MARGIN,
    0,
    cons->max_width,
    cons->max_height,
    CONSOLE_SCROLLBACK_AMOUNT
  );

  cons->input = HU_InputWidgetNew(
    render_context,
    x + CONSOLE_MARGIN,
    0,
    cons->max_width,
    cons->max_height,
    handler
  );

  HU_InputWidgetSetFont(cons->input, HU_FONT);
  HU_InputWidgetGetLayoutSize(cons->input, &input_width, &input_height);
  HU_InputWidgetSetSize(
    cons->input,
    cons->max_width - (CONSOLE_MARGIN + CONSOLE_MARGIN),
    input_height
  );

  HU_MessageWidgetSetScrollable(cons->scrollback, true);
  HU_MessageWidgetSetFont(cons->scrollback, HU_FONT);
  HU_MessageWidgetSetSize(
    cons->scrollback,
    cons->max_width,
    cons->max_height - (CONSOLE_MARGIN + input_height + input_height)
  );
  HU_MessageWidgetSetAlignBottom(cons->scrollback, true);

  HU_InputWidgetSetFGColor(cons->input, white);
  HU_InputWidgetSetBGColor(cons->input, clear);
  HU_MessageWidgetSetFGColor(cons->scrollback, white);
  HU_MessageWidgetSetBGColor(cons->scrollback, clear);

  return cons;
}

void HU_ConsoleWidgetReset(console_widget_t *cons, void *render_context) {
  int input_width;
  int input_height;
  hu_color_t white = {1.0f, 1.0f, 1.0f, 1.0f};
  hu_color_t clear = {0.0f, 0.0f, 0.0f, 0.0f};

  cons->scroll_rate = 0.0;
  cons->height = 0.0;
  cons->max_height = CONSOLE_MAXHEIGHT;
  cons->max_width = CONSOLE_MAXWIDTH;

  HU_InputWidgetReset(cons->input, render_context);
  HU_MessageWidgetReset(cons->scrollback, render_context);
  HU_MessageWidgetSetScrollAmount(
    cons->scrollback, CONSOLE_SCROLLBACK_AMOUNT
  );

  HU_InputWidgetGetLayoutSize(cons->input, &input_width, &input_height);
  HU_InputWidgetSetSize(
    cons->input,
    cons->max_width - (CONSOLE_MARGIN + CONSOLE_MARGIN),
    input_height
  );

  HU_MessageWidgetSetSize(
    cons->scrollback,
    cons->max_width,
    cons->max_height - (CONSOLE_MARGIN + input_height + input_height)
  );

  HU_InputWidgetSetFGColor(cons->input, white);
  HU_InputWidgetSetBGColor(cons->input, clear);
  HU_MessageWidgetSetFGColor(cons->scrollback, white);
  HU_MessageWidgetSetBGColor(cons->scrollback, clear);
}

void HU_ConsoleWidgetTicker(console_widget_t *cons) {
  static int last_run_ms = 0;

  int ms_elapsed;
  int current_ms = I_GetTicks();

  if (last_run_ms == 0)
    last_run_ms = current_ms;

  ms_elapsed = current_ms - last_run_ms;
  last_run_ms = current_ms;

  cons->height += (cons->scroll_rate * ms_elapsed);

  if (cons->height <= 0.0) {
    cons->height = 0.0;
    cons->scroll_rate = 0.0;
  }
  else if (cons->height >= cons->max_height) {
    cons->height = cons->max_height;
    cons->scroll_rate = 0.0;
  }
}

void HU_ConsoleWidgetDrawer(console_widget_t *cons, void *render_context) {
  cairo_t *cr = (cairo_t *)render_context;
  int sb_width;
  int sb_height;
  int input_width;
  int input_height;

  cairo_save(cr);

  cairo_reset_clip(cr);
  cairo_new_path(cr);
  cairo_rectangle(cr, 0, 0, cons->max_width, cons->height);
  cairo_clip(cr);
  cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
  cairo_set_source_rgba(cr, 0.0f, 0.0f, 0.0f, 0.8f);
  cairo_paint(cr);

  cairo_restore(cr);

  HU_MessageWidgetGetLayoutSize(cons->scrollback, &sb_width, &sb_height);
  HU_InputWidgetGetLayoutSize(cons->input, &input_width, &input_height);

  HU_InputWidgetSetPosition(
    cons->input,
    CONSOLE_MARGIN,
    cons->height - (input_height + CONSOLE_MARGIN)
  );

  HU_MessageWidgetSetPosition(
    cons->scrollback, CONSOLE_MARGIN, 0 - (cons->max_height - cons->height)
  );

  HU_MessageWidgetDrawer(cons->scrollback, render_context);
  HU_InputWidgetDrawer(cons->input, render_context);
}

bool HU_ConsoleWidgetResponder(console_widget_t *cons, event_t *ev) {
  if (ev->type == ev_key && ev->pressed && ev->key == SDLK_BACKQUOTE) {
    HU_ConsoleWidgetToggleScroll(cons);
    return true;
  }

  if (!HU_ConsoleWidgetActive(cons))
    return false;

  if (HU_MessageWidgetResponder(cons->scrollback, ev))
    return true;

  if (HU_InputWidgetResponder(cons->input, ev))
    return true;

  return true;
}

void HU_ConsoleWidgetScrollDown(console_widget_t *cons) {
  cons->scroll_rate = CONSOLE_SCROLL_DOWN_RATE;
}

void HU_ConsoleWidgetScrollUp(console_widget_t *cons) {
  cons->scroll_rate = CONSOLE_SCROLL_UP_RATE;
}

void HU_ConsoleWidgetToggleScroll(console_widget_t *cons) {
  if (cons->height == cons->max_height)
    HU_ConsoleWidgetScrollUp(cons);
  else if (cons->height == 0)
    HU_ConsoleWidgetScrollDown(cons);
  else if (cons->scroll_rate < 0.0)
    HU_ConsoleWidgetScrollDown(cons);
  else if (cons->scroll_rate > 0.0)
    HU_ConsoleWidgetScrollUp(cons);
}

void HU_ConsoleWidgetSummon(console_widget_t *cons) {
  cons->height = cons->max_height;
  cons->scroll_rate = 0.0;
}

void HU_ConsoleWidgetBanish(console_widget_t *cons) {
  cons->height = 0;
  cons->scroll_rate = 0.0;
}

void HU_ConsoleWidgetSetFullScreen(console_widget_t *cons) {
  cons->height = REAL_SCREENHEIGHT;
  cons->scroll_rate = 0.0;
}

bool HU_ConsoleWidgetActive(console_widget_t *cons) {
  return ((cons->scroll_rate >= 0.0) && (cons->height > 0.0));
}

char* HU_ConsoleWidgetGetInputText(console_widget_t *cons) {
  return HU_InputWidgetGetText(cons->input);
}

void HU_ConsoleWidgetPrintf(console_widget_t *cons, const char *fmt, ...) {
  va_list args;

  va_start(args, fmt);
  if (nodrawers)
    vprintf(fmt, args);
  else
    HU_MessageWidgetVPrintf(cons->scrollback, fmt, args);
  va_end(args);
}

void HU_ConsoleWidgetVPrintf(console_widget_t *cons, const char *fmt,
                                                     va_list args) {
  if (nodrawers)
    vprintf(fmt, args);
  else
    HU_MessageWidgetVPrintf(cons->scrollback, fmt, args);
}

void HU_ConsoleWidgetMPrintf(console_widget_t *cons, const char *fmt, ...) {
  va_list args;

  /* CG: TODO: Strip markup in nodrawers case */

  va_start(args, fmt);
  if (nodrawers)
    vprintf(fmt, args);
  else
    HU_MessageWidgetMVPrintf(cons->scrollback, fmt, args);
  va_end(args);
}

void HU_ConsoleWidgetMVPrintf(console_widget_t *cons, const char *fmt,
                                                      va_list args) {
  /* CG: TODO: Strip markup in nodrawers case */

  if (nodrawers)
    vprintf(fmt, args);
  else
    HU_MessageWidgetMVPrintf(cons->scrollback, fmt, args);
}

void HU_ConsoleWidgetEcho(console_widget_t *cons, const char *message) {
  if (nodrawers)
    puts(message);
  else
    HU_MessageWidgetEcho(cons->scrollback, message);
}

void HU_ConsoleWidgetMEcho(console_widget_t *cons, const char *message) {
  /* CG: TODO: Strip markup in nodrawers case */

  if (nodrawers)
    puts(message);
  else
    HU_MessageWidgetMEcho(cons->scrollback, message);
}

void HU_ConsoleWidgetWrite(console_widget_t *cons, const char *message) {
  if (nodrawers)
    printf("%s", message);
  else
    HU_MessageWidgetWrite(cons->scrollback, message);
}

void HU_ConsoleWidgetMWrite(console_widget_t *cons, const char *message) {
  /* CG: TODO: Strip markup in nodrawers case */

  if (nodrawers)
    printf("%s", message);
  else
    HU_MessageWidgetMWrite(cons->scrollback, message);
}

/* vi: set et ts=2 sw=2: */

