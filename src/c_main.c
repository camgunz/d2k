/* Emacs style mode select   -*- C++ -*-
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
 *  Console
 *
 *-----------------------------------------------------------------------------
 */

#include "z_zone.h"

#include <SDL.h>
#include <pango/pangocairo.h>

#include "doomdef.h"
#include "doomstat.h"
#include "c_main.h"
#include "lprintf.h"

#define CAIRO_ASHIFT 24
#define CAIRO_RSHIFT 16
#define CAIRO_GSHIFT  8
#define CAIRO_BSHIFT  0
#define CAIRO_AMASK (255U << CAIRO_ASHIFT)
#define CAIRO_RMASK (255U << CAIRO_RSHIFT)
#define CAIRO_GMASK (255U << CAIRO_GSHIFT)
#define CAIRO_BMASK (255U << CAIRO_BSHIFT)

#define CONSOLE_HEIGHT (REAL_SCREENHEIGHT / 2)
#define CONSOLE_WIDTH  (REAL_SCREENWIDTH)
#define CONSOLE_SCROLL_DOWN_RATE (2 / 25)
#define CONSOLE_SCROLL_UP_RATE (-(2 / 15))
#define CONSOLE_PROMPT ">"
#define CONSOLE_CURSOR "_"

#define TEST_SCROLLBACK_TEXT \
"<[UD]Ladna> line 1\n" \
"<[UD]Ladna> line 2\n" \
"<[UD]Ladna> line 3\n" \
"<[UD]Ladna> line 4\n" \
"<[UD]Ladna> line 5\n" \
"<[UD]Ladna> line 6\n" \
"<[UD]Ladna> line 7\n" \
"<[UD]Ladna> line 8\n" \
"<[UD]Ladna> line 9\n" \
"<[UD]Ladna> line 10\n" \
"<[UD]Ladna> line 11\n" \
"<[UD]Ladna> line 12\n" \
"<[UD]Ladna> line 13\n" \
"<[UD]Ladna> line 14\n" \
"<[UD]Ladna> line 14\n" \
"<JKist3[NS]> go?\n" \
"<icereg> 開発メモ 技術系の作業メモおよびアイデアの記録\n" \
"[UD]Ladna joined the game\n" \
"JKist3[NS] joined the game\n" \
"[UD]Ladna was splattered by JKist3[NS]'s super shotgun\n" \
"icereg joined the game\n" \
"<[UD]Ladna> icereg spec\n" \
"<icereg> Photoshop等で実装されている\n" \
"*icereg was kicked from the server (Message: spec)*\n" \
"<[UD]Ladna> This is a super long message, well well well, who knew I could " \
    "type such a long line eh????  Apparently it wasn't long enough to " \
    "start with, so I've now made it even longer.  Let's hope this is long " \
    "enough to wrap now\n" \

#define TEST_INPUT_TEXT \
  CONSOLE_PROMPT \
  " 技術系の作業メモおよびアイデアの記録" \
  CONSOLE_CURSOR

extern SDL_Surface *screen;

static cairo_t *cairo_context = NULL;
static cairo_surface_t *cairo_surface = NULL;
static PangoContext *pango_context = NULL;
static PangoLayout *pango_scrollback_layout = NULL;
static PangoLayout *pango_input_layout = NULL;
static double console_scroll_rate = 0;
static int console_current_height = 0;

void C_Init(void) {
  cairo_status_t status;
  cairo_font_options_t *font_options = cairo_font_options_create();

  C_Reset();

  cairo_surface = cairo_image_surface_create_for_data(
    screen->pixels, CAIRO_FORMAT_RGB24, screen->w, screen->h, screen->pitch
  );

  status = cairo_surface_status(cairo_surface);

  if (status != CAIRO_STATUS_SUCCESS)
    I_Error("C_Init: Error creating cairo surface (error %d)\n", status);

  cairo_context = cairo_create(cairo_surface);

  status = cairo_status(cairo_context);

  if (status != CAIRO_STATUS_SUCCESS)
    I_Error("C_Init: Error creating cairo context (error %d)\n", status);

  pango_context = pango_cairo_create_context(cairo_context);

  pango_scrollback_layout = pango_layout_new(pango_context);
  pango_layout_set_width(pango_scrollback_layout, CONSOLE_WIDTH * PANGO_SCALE);
  pango_layout_set_wrap(pango_scrollback_layout, PANGO_WRAP_WORD_CHAR);
  // pango_layout_set_ellipsize(pango_scrollback_layout, PANGO_ELLIPSIZE_START);

  pango_input_layout = pango_layout_new(pango_context);
  pango_layout_set_width(pango_input_layout, CONSOLE_WIDTH * PANGO_SCALE);
  pango_layout_set_text(pango_input_layout,
    CONSOLE_PROMPT " " CONSOLE_CURSOR, -1
  );

  cairo_font_options_destroy(font_options);

  console_current_height = CONSOLE_HEIGHT;
}

void C_Reset(void) {
  if (cairo_context)
    cairo_destroy(cairo_context);

  if (cairo_surface)
    cairo_surface_destroy(cairo_surface);

  if (pango_context)
    g_object_unref(pango_context);

  if (pango_scrollback_layout)
    g_object_unref(pango_scrollback_layout);

  if (pango_input_layout)
    g_object_unref(pango_input_layout);
}

void C_ScrollDown(void) {
  console_scroll_rate = CONSOLE_HEIGHT * CONSOLE_SCROLL_DOWN_RATE;
}

void C_ScrollUp(void) {
  console_scroll_rate = CONSOLE_HEIGHT * CONSOLE_SCROLL_DOWN_RATE;
}

void C_Ticker(void) {
  static int last_run_tic = 0;

  int tics_run;

  if (last_run_tic == 0)
    last_run_tic = gametic - 1;

  tics_run = gametic - last_run_tic;

  last_run_tic = gametic;

  console_current_height += console_scroll_rate;

  if (console_current_height <= 0.0 ||
      console_current_height >= CONSOLE_HEIGHT) {
    console_scroll_rate = 0.0;
  }

}

void C_Drawer(void) {
  PangoFontDescription *desc = pango_font_description_from_string("FreeSans 8");
  int input_width;
  int input_height;
  int sb_width;
  int sb_height;

  if (SDL_MUSTLOCK(screen))
    SDL_LockSurface(screen);

  cairo_new_path(cairo_context);
  cairo_rectangle(cairo_context, 0, 0, CONSOLE_WIDTH, console_current_height);
  cairo_clip(cairo_context);

  cairo_set_source_rgb(cairo_context, 1.0, 1.0, 1.0);
  cairo_paint(cairo_context);

  cairo_set_source_rgb(cairo_context, 0.0, 0.0, 0.0);

  pango_layout_set_text(pango_input_layout, TEST_INPUT_TEXT, -1);
  pango_layout_set_font_description(pango_input_layout, desc);
  pango_layout_set_width(pango_input_layout, -1);
  pango_cairo_update_layout(cairo_context, pango_input_layout);

  pango_layout_get_size(pango_input_layout, &input_width, &input_height);
  input_width /= PANGO_SCALE;
  input_height /= PANGO_SCALE;
  cairo_move_to(cairo_context, 4, console_current_height - (input_height + 4));
  pango_cairo_show_layout(cairo_context, pango_input_layout);

  if (console_current_height > ((input_height * 2) + 4 + 4)) {
    pango_layout_set_text(pango_scrollback_layout, TEST_SCROLLBACK_TEXT, -1);
    pango_layout_set_font_description(pango_scrollback_layout, desc);
    /*
    pango_layout_set_height(pango_scrollback_layout,
      (console_current_height - ((input_height * 2) + 4 + 4)) * PANGO_SCALE
    );
    */
    pango_layout_set_wrap(pango_scrollback_layout, PANGO_WRAP_WORD_CHAR);
    pango_cairo_update_layout(cairo_context, pango_scrollback_layout);

    pango_layout_get_size(pango_scrollback_layout, &sb_width, &sb_height);
    sb_width /= PANGO_SCALE;
    sb_height /= PANGO_SCALE;
    printf("%d, %d, %d\n", input_height, console_current_height, sb_height);
    cairo_move_to(cairo_context, 4, console_current_height -
      ((input_height * 2) + sb_height)
    );

    /*
    cairo_move_to(cairo_context, 4, 4);
    */
    pango_cairo_show_layout(cairo_context, pango_scrollback_layout);
  }

  if (SDL_MUSTLOCK(screen))
    SDL_UnlockSurface(screen);

  pango_font_description_free(desc);
}

/* vi: set et ts=2 sw=2: */

