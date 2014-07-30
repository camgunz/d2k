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


#ifndef HU_CONS_H__
#define HU_CONS_H__

struct console_widget_s;
typedef struct console_widget_s console_widget_t;
typedef void (*ConsoleInputHandler)(console_widget_t *cons);

console_widget_t* HU_ConsoleWidgetNew(void *render_context,
                                      int x, int y, int w, int h,
                                      InputHandler handler);
void  HU_ConsoleWidgetReset(console_widget_t *cons, void *render_context);
void  HU_ConsoleWidgetTicker(console_widget_t *cons);
void  HU_ConsoleWidgetDrawer(console_widget_t *cons, void *render_context);
bool  HU_ConsoleWidgetResponder(console_widget_t *cons, event_t *ev);
void  HU_ConsoleWidgetScrollDown(console_widget_t *cons);
void  HU_ConsoleWidgetScrollUp(console_widget_t *cons);
void  HU_ConsoleWidgetToggleScroll(console_widget_t *cons);
void  HU_ConsoleWidgetSummon(console_widget_t *cons);
void  HU_ConsoleWidgetBanish(console_widget_t *cons);
void  HU_ConsoleWidgetSetFullScreen(console_widget_t *cons);
bool  HU_ConsoleWidgetActive(console_widget_t *cons);
char* HU_ConsoleWidgetGetInputText(console_widget_t *cons);
void  HU_ConsoleWidgetPrintf(console_widget_t *cons, const char *fmt, ...)
  PRINTF_DECL(2, 3);
void  HU_ConsoleWidgetVPrintf(console_widget_t *cons, const char *fmt,
                                                      va_list args);
void  HU_ConsoleWidgetMPrintf(console_widget_t *cons, const char *fmt, ...)
  PRINTF_DECL(2, 3);
void  HU_ConsoleWidgetMVPrintf(console_widget_t *cons, const char *fmt,
                                                       va_list args);
void  HU_ConsoleWidgetEcho(console_widget_t *cons, const char *message);
void  HU_ConsoleWidgetMEcho(console_widget_t *cons, const char *message);
void  HU_ConsoleWidgetWrite(console_widget_t *cons, const char *message);
void  HU_ConsoleWidgetMWrite(console_widget_t *cons, const char *message);

#endif

/* vi: set et ts=2 sw=2: */

