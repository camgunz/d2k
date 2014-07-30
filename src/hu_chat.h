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


#ifndef HU_CHAT_H__
#define HU_CHAT_H__

struct chat_widget_s;
typedef struct chat_widget_s chat_widget_t;
typedef void (*ChatInputHandler)(chat_widget_t *cw);

chat_widget_t* HU_ChatWidgetNew(void *render_context,
                                int x, int y, int w, int h,
                                ChatInputHandler handler);
void   HU_ChatWidgetSetPosition(chat_widget_t *cw, int x, int y);
void   HU_ChatWidgetReset(chat_widget_t *cw, void *render_context);
void   HU_ChatWidgetMoveToBottom(chat_widget_t *cw, int baseline);
void   HU_ChatWidgetSetHeightByLines(chat_widget_t *cw, int lines);
void   HU_ChatWidgetSetFGColor(chat_widget_t *cw, hu_color_t fg_color);
void   HU_ChatWidgetSetBGColor(chat_widget_t *cw, hu_color_t bg_color);
void   HU_ChatWidgetActivate(chat_widget_t *cw);
void   HU_ChatWidgetDeactivate(chat_widget_t *cw);
bool   HU_ChatWidgetActive(chat_widget_t *cw);
void   HU_ChatWidgetDrawer(chat_widget_t *cw, void *render_context);
bool   HU_ChatWidgetResponder(chat_widget_t *cw, event_t *ev);
char*  HU_ChatWidgetGetInputText(chat_widget_t *cw);
size_t HU_ChatWidgetGetTextLength(chat_widget_t *cw);
void   HU_ChatWidgetClear(chat_widget_t *cw);

#endif

/* vi: set et ts=2 sw=2: */

