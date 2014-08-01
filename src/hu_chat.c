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

#include "doomdef.h"
#include "doomstat.h"
#include "d_event.h"
#include "c_main.h"
#include "d_main.h"
#include "gl_opengl.h"
#include "gl_intern.h"
#include "hu_lib.h"
#include "hu_input.h"
#include "hu_chat.h"
#include "hu_stuff.h"
#include "i_main.h"
#include "i_system.h"
#include "lprintf.h"
#include "n_net.h"
#include "n_proto.h"
#include "v_video.h"

struct chat_widget_s {
  input_widget_t *input;
  bool active;
  ChatInputHandler handleInput;
};

chat_widget_t* HU_ChatWidgetNew(void *render_context,
                                int x, int y, int w, int h,
                                ChatInputHandler handler) {
  int input_width;
  int input_height;
  hu_color_t white = {1.0f, 1.0f, 1.0f, 1.0f};
  hu_color_t clear = {0.0f, 0.0f, 0.0f, 0.0f};
  chat_widget_t *cw = calloc(1, sizeof(chat_widget_t));

  if (cw == NULL)
    I_Error("HU_ChatWidgetNew: calloc failed");

  cw->input = HU_InputWidgetNew(render_context, x, y, w, h, NULL);
  cw->handleInput = handler;

  HU_InputWidgetSetFont(cw->input, HU_FONT);
  HU_InputWidgetGetLayoutSize(cw->input, &input_width, &input_height);
  HU_InputWidgetSetSize(cw->input, REAL_SCREENWIDTH, input_height);
  HU_InputWidgetSetFGColor(cw->input, white);
  HU_InputWidgetSetBGColor(cw->input, clear);

  return cw;
}

void HU_ChatWidgetSetPosition(chat_widget_t *cw, int x, int y) {
  HU_InputWidgetSetPosition(cw->input, x, y);
}

void HU_ChatWidgetReset(chat_widget_t *cw, void *render_context) {
  int input_width;
  int input_height;

  HU_InputWidgetReset(cw->input, render_context);
  HU_InputWidgetGetLayoutSize(cw->input, &input_width, &input_height);
  HU_InputWidgetSetSize(cw->input, REAL_SCREENWIDTH, input_height);
}

void HU_ChatWidgetMoveToBottom(chat_widget_t *cw, int baseline) {
  int iw_x;
  int iw_y;
  int iw_w;
  int iw_h;

  HU_InputWidgetGetPosition(cw->input, &iw_x, &iw_y);
  HU_InputWidgetGetLayoutSize(cw->input, &iw_w, &iw_h);

  HU_ChatWidgetSetPosition(cw, iw_x, baseline - iw_h);
}

void HU_ChatWidgetSetHeightByLines(chat_widget_t *cw, int lines) {
  HU_InputWidgetSetHeightByLines(cw->input, lines);
}

void HU_ChatWidgetSetFGColor(chat_widget_t *cw, hu_color_t fg_color) {
  HU_InputWidgetSetFGColor(cw->input, fg_color);
}

void HU_ChatWidgetSetBGColor(chat_widget_t *cw, hu_color_t bg_color) {
  HU_InputWidgetSetBGColor(cw->input, bg_color);
}

void HU_ChatWidgetActivate(chat_widget_t *cw) {
  cw->active = true;
}

void HU_ChatWidgetDeactivate(chat_widget_t *cw) {
  cw->active = false;
  HU_ChatWidgetClear(cw);
}

bool HU_ChatWidgetActive(chat_widget_t *cw) {
  return cw->active;
}

void HU_ChatWidgetDrawer(chat_widget_t *cw, void *render_context) {
  HU_InputWidgetDrawer(cw->input, render_context);
}

bool HU_ChatWidgetResponder(chat_widget_t *cw, event_t *ev) {
  if (!HU_ChatWidgetActive(cw))
    return false;

  if (ev->type != ev_keydown)
      return false;

  if (ev->data1 == SDLK_ESCAPE) {
    HU_ChatWidgetDeactivate(cw);
    return true;
  }

  if (ev->data1 == SDLK_RETURN) {
    cw->handleInput(cw);
    HU_ChatWidgetDeactivate(cw);
    return true;
  }

  return HU_InputWidgetResponder(cw->input, ev);
}

char* HU_ChatWidgetGetInputText(chat_widget_t *cw) {
  return HU_InputWidgetGetText(cw->input);
}

size_t HU_ChatWidgetGetTextLength(chat_widget_t *cw) {
  return HU_InputWidgetGetTextLength(cw->input);
}

void HU_ChatWidgetClear(chat_widget_t *cw) {
  HU_InputWidgetClear(cw->input);
}

/* vi: set et ts=2 sw=2: */

