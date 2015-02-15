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
#include "i_video.h"
#include "lprintf.h"
#include "n_net.h"
#include "n_main.h"
#include "cl_main.h"
#include "v_video.h"
#include "x_main.h"

void C_Reset(void) {
  if (!nodrawers)
    X_Call(X_GetState(), "console", "reset", 0, 0);
}

void C_ScrollDown(void) {
  if (!nodrawers)
    X_Call(X_GetState(), "console", "scroll_down", 0, 0);
}

void C_ScrollUp(void) {
  if (!nodrawers)
    X_Call(X_GetState(), "console", "scroll_up", 0, 0);
}

void C_ToggleScroll(void) {
  if (!nodrawers)
    X_Call(X_GetState(), "console", "toggle_scroll", 0, 0);
}

void C_Summon(void) {
  if (!nodrawers)
    X_Call(X_GetState(), "console", "summon", 0, 0);
}

void C_Banish(void) {
  if (!nodrawers)
    X_Call(X_GetState(), "console", "banish", 0, 0);
}

void C_SetFullscreen(void) {
  if (!nodrawers)
    X_Call(X_GetState(), "console", "set_fullscreen", 0, 0);
}

bool C_Active(void) {
  lua_State *L;
  bool is_active = false;

  if (!nodrawers) {
    L = X_GetState();

    X_Call(L, "console", "is_active", 0, 1);
    is_active = lua_toboolean(L, -1);
    lua_pop(L, 1);
  }

  return is_active;
}

/*
 * CG: TODO: Move printing to stdout from hu_msg to here
 *     TODO: Add D_Log logging
 */

void C_Printf(const char *fmt, ...) {
  va_list args;

  va_start(args, fmt);
  C_VPrintf(fmt, args);
  va_end(args);
}

void C_VPrintf(const char *fmt, va_list args) {
  gchar *message;

  if (!fmt)
    return;

  message = g_strdup_vprintf(fmt, args);
  C_Write(message);
  g_free(message);
}

void C_MPrintf(const char *fmt, ...) {
  va_list args;

  if (!fmt)
    return;

  va_start(args, fmt);
  C_MVPrintf(fmt, args);
  va_end(args);
}

void C_MVPrintf(const char *fmt, va_list args) {
  gchar *message;
  
  if (!fmt)
    return;

  message = g_strdup_vprintf(fmt, args);
  C_MWrite(message);
  g_free(message);
}

void C_Echo(const char *message) {
  gchar *escaped_message;

  if (!message)
    return;
  
  escaped_message = g_markup_escape_text(message, -1);

  X_Call(X_GetState(), "console", "echo", 1, 0, X_STRING, escaped_message);
  g_free(escaped_message);
}

void C_MEcho(const char *message) {
  if (message)
    X_Call(X_GetState(), "console", "mecho", 1, 0, X_STRING, message);
}

void C_Write(const char *message) {
  gchar *escaped_message;

  if (!message)
    return;
  
  escaped_message = g_markup_escape_text(message, -1);

  X_Call(X_GetState(), "console", "write", 1, 0, X_STRING, escaped_message);
  g_free(escaped_message);
}

void C_MWrite(const char *message) {
  if (!message)
    return;

  X_Call(X_GetState(), "console", "mwrite", 1, 0, X_STRING, message);
}

/* vi: set et ts=2 sw=2: */

