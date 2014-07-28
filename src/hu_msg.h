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
 *   Message Widget
 *
 *-----------------------------------------------------------------------------
 */

#ifndef HU_MSG_H__
#define HU_MSG_H__

struct message_widget_s;
typedef struct message_widget_s message_widget_t;

message_widget_t* HU_MessageWidgetNew(void *render_context,
                                      int x, int y, int w, int h,
                                      int scroll_amount);
void HU_MessageWidgetReset(message_widget_t *mw, void *render_context);
void HU_MessageWidgetGetSize(message_widget_t *mw, int *width, int *height);
void HU_MessageWidgetSetSize(message_widget_t *mw, int width, int height);
void HU_MessageWidgetGetPosition(message_widget_t *mw, int *x, int *y);
void HU_MessageWidgetSetPosition(message_widget_t *mw, int x, int y);
void HU_MessageWidgetGetLayoutSize(message_widget_t *mw, int *width,
                                                         int *height);
void HU_MessageWidgetSetFGColor(message_widget_t *mw, hu_color_t fg_color);
void HU_MessageWidgetSetBGColor(message_widget_t *mw, hu_color_t bg_color);
void HU_MessageWidgetSetFont(message_widget_t *mw, const char *font);
void HU_MessageWidgetSetScrollAmount(message_widget_t *mw, int scroll_amount);
void HU_MessageWidgetRebuild(message_widget_t *mw, void *render_context);
void HU_MessageWidgetDrawer(message_widget_t *mw, void *render_context);
bool HU_MessageWidgetResponder(message_widget_t *mw, event_t *ev);
void HU_MessageWidgetPrintf(message_widget_t *mw, const char *fmt, ...)
  PRINTF_DECL(2, 3);
void HU_MessageWidgetVPrintf(message_widget_t *mw, const char *fmt,
                                                   va_list args);
void HU_MessageWidgetMPrintf(message_widget_t *mw, const char *fmt, ...)
  PRINTF_DECL(2, 3);
void HU_MessageWidgetMVPrintf(message_widget_t *mw, const char *fmt,
                                                    va_list args);
void HU_MessageWidgetEcho(message_widget_t *mw, const char *message);
void HU_MessageWidgetMEcho(message_widget_t *mw, const char *message);
void HU_MessageWidgetWrite(message_widget_t *mw, const char *message);
void HU_MessageWidgetMWrite(message_widget_t *mw, const char *message);

#endif

/* vi: set et ts=2 sw=2: */

