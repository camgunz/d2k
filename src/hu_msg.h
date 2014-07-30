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


#ifndef HU_MSG_H__
#define HU_MSG_H__

struct message_widget_s;
typedef struct message_widget_s message_widget_t;

message_widget_t* HU_MessageWidgetNew(void *render_context,
                                      int x, int y, int w, int h,
                                      int scroll_amount);
void HU_MessageWidgetReset(message_widget_t *mw, void *render_context);
void HU_MessageWidgetSetAlignBottom(message_widget_t *mw, bool align_bottom);
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
void HU_MessageWidgetSetHeightByLines(message_widget_t *mw, int lines);
bool HU_MessageWidgetHasContent(message_widget_t *mw);
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
void HU_MessageWidgetClear(message_widget_t *mw);

#endif

/* vi: set et ts=2 sw=2: */

