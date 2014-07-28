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
 *   Input Widget
 *
 *-----------------------------------------------------------------------------
 */

#ifndef HU_INPUT_H__
#define HU_INPUT_H__

struct input_widget_s;
typedef struct input_widget_s input_widget_t;
typedef void (*InputHandler)(input_widget_t *iw);

input_widget_t* HU_InputWidgetNew(void *render_context,
                                  int x, int y, int w, int h,
                                  InputHandler handler);
void  HU_InputWidgetReset(input_widget_t *iw, void *render_context);
void  HU_InputWidgetGetSize(input_widget_t *iw, int *width, int *height);
void  HU_InputWidgetSetSize(input_widget_t *iw, int width, int height);
void  HU_InputWidgetGetPosition(input_widget_t *iw, int *x, int *y);
void  HU_InputWidgetSetPosition(input_widget_t *iw, int x, int y);
void  HU_InputWidgetGetLayoutSize(input_widget_t *iw, int *width, int *height);
void  HU_InputWidgetSetFGColor(input_widget_t *iw, hu_color_t fg_color);
void  HU_InputWidgetSetBGColor(input_widget_t *iw, hu_color_t bg_color);
void  HU_InputWidgetSetCursorColor(input_widget_t *iw, hu_color_t curs_color);
void  HU_InputWidgetSetFont(input_widget_t *iw, const char *font);
void  HU_InputWidgetRebuild(input_widget_t *iw, void *render_context);
void  HU_InputWidgetDrawer(input_widget_t *iw, void *render_context);
bool  HU_InputWidgetResponder(input_widget_t *iw, event_t *ev);
char* HU_InputWidgetGetText(input_widget_t *iw);

#endif

/* vi: set et ts=2 sw=2: */

