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


#ifndef HU_INPUT_H__
#define HU_INPUT_H__

struct input_widget_s;
typedef struct input_widget_s input_widget_t;
typedef void (*InputHandler)(input_widget_t *iw);

input_widget_t* HU_InputWidgetNew(void *render_context,
                                  int x, int y, int w, int h,
                                  InputHandler handler);
void   HU_InputWidgetReset(input_widget_t *iw, void *render_context);
void   HU_InputWidgetGetSize(input_widget_t *iw, int *width, int *height);
void   HU_InputWidgetSetSize(input_widget_t *iw, int width, int height);
void   HU_InputWidgetGetPosition(input_widget_t *iw, int *x, int *y);
void   HU_InputWidgetSetPosition(input_widget_t *iw, int x, int y);
void   HU_InputWidgetGetLayoutSize(input_widget_t *iw, int *width, int *height);
void   HU_InputWidgetSetFGColor(input_widget_t *iw, hu_color_t fg_color);
void   HU_InputWidgetSetBGColor(input_widget_t *iw, hu_color_t bg_color);
void   HU_InputWidgetSetCursorColor(input_widget_t *iw, hu_color_t curs_color);
void   HU_InputWidgetSetFont(input_widget_t *iw, const char *font);
void   HU_InputWidgetSetHeightByLines(input_widget_t *iw, int lines);
void   HU_InputWidgetRebuild(input_widget_t *iw, void *render_context);
void   HU_InputWidgetDrawer(input_widget_t *iw, void *render_context);
bool   HU_InputWidgetResponder(input_widget_t *iw, event_t *ev);
char*  HU_InputWidgetGetText(input_widget_t *iw);
size_t HU_InputWidgetGetTextLength(input_widget_t *iw);
void   HU_InputWidgetClear(input_widget_t *iw);

#endif

/* vi: set et ts=2 sw=2: */

