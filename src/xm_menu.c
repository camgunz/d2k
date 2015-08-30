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

#include "doomstat.h"
#include "d_event.h"
#include "m_menu.h"
#include "x_main.h"

static int XM_MenuActivate(lua_State *L) {
  menuactive = true;

  return 0;
}

static int XM_MenuDeactivate(lua_State *L) {
  menuactive = false;

  return 0;
}

static int XM_MenuIsActive(lua_State *L) {
  lua_pushboolean(L, menuactive);

  return 1;
}

static int XM_MenuTick(lua_State *L) {
  M_Ticker();

  return 0;
}

static int XM_MenuDraw(lua_State *L) {
  M_Drawer();

  return 0;
}

static int XM_MenuHandleEvent(lua_State *L) {
  event_t *ev = luaL_checkudata(L, -1, "InputEvent");
  bool event_handled = M_Responder(ev);

  lua_pushboolean(L, event_handled);

  return 1;
}

void XM_MenuRegisterInterface(void) {
  X_RegisterObjects("Menu", 6,
    "activate",     X_FUNCTION, XM_MenuActivate,
    "deactivate",   X_FUNCTION, XM_MenuDeactivate,
    "is_active",    X_FUNCTION, XM_MenuIsActive,
    "tick",         X_FUNCTION, XM_MenuTick,
    "draw",         X_FUNCTION, XM_MenuDraw,
    "handle_event", X_FUNCTION, XM_MenuHandleEvent
  );
}

/* vi: set et ts=2 sw=2: */

