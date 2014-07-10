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

#define TEST_SCROLLBACK_TEXT \
"<[UD]Ladna> sup\n" \
"<JKist3[NS]> go?\n" \
"<icereg> 開発メモ 技術系の作業メモおよびアイデアの記録\n" \
"[UD]Ladna joined the game\n" \
"JKist3[NS] joined the game\n" \
"[UD]Ladna was splattered by JKist3[NS]'s super shotgun\n" \
"icereg joined the game\n" \
"<[UD]Ladna> icereg spec\n" \
"<icereg> Photoshop等で実装されている\n" \
"*icereg was kicked from the server (Message: spec)*\n"
#define TEST_INPUT_TEXT "技術系の作業メモおよびアイデアの記録"

extern SDL_Surface *screen;

static cairo_t *cairo_context = NULL;
static cairo_surface_t *cairo_surface = NULL;
static PangoContext *pango_context = NULL;
static PangoLayout *pango_scrollback_layout = NULL;
static PangoLayout *pango_input_layout = NULL;

void C_Init(void) {
  cairo_status_t status;

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
  pango_input_layout = pango_layout_new(pango_context);
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

void C_Ticker(void) {
}

void C_Drawer(void) {
  PangoFontDescription *desc = pango_font_description_from_string("Unifont 12");

  return;

  if (SDL_MUSTLOCK(screen))
    SDL_LockSurface(screen);

  cairo_set_source_rgb(cairo_context, 1.0, 1.0, 1.0);
  cairo_paint(cairo_context);
  cairo_set_source_rgb(cairo_context, 0.0, 0.0, 0.0);

  pango_layout_set_text(pango_scrollback_layout, TEST_SCROLLBACK_TEXT, -1);
  pango_layout_set_font_description(pango_scrollback_layout, desc);
  pango_cairo_update_layout(cairo_context, pango_scrollback_layout);
  pango_cairo_show_layout(cairo_context, pango_scrollback_layout);

  pango_layout_set_text(pango_input_layout, TEST_INPUT_TEXT, -1);
  pango_layout_set_font_description(pango_input_layout, desc);
  pango_cairo_update_layout(cairo_context, pango_input_layout);
  pango_cairo_show_layout(cairo_context, pango_input_layout);

  if (SDL_MUSTLOCK(screen))
    SDL_UnlockSurface(screen);

  pango_font_description_free(desc);
}

/* vi: set et ts=2 sw=2: */

