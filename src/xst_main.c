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

#include "doomdef.h"
#include "r_defs.h"
#include "d_event.h"
#include "st_stuff.h"
#include "x_intern.h"
#include "x_main.h"

static int XST_HandleEvent(lua_State *L) {
  event_t *ev = luaL_checkudata(L, -1, "InputEvent");
  bool event_handled = ST_Responder(ev);

  lua_pushboolean(L, event_handled);

  return 1;
}

void XST_RegisterInterface(void) {
  X_RegisterObjects("StatusBar", 4,
    "handle_event",                    X_FUNCTION, XST_HandleEvent,
    "ammo_colour_behaviour_no",        X_INTEGER,  ammo_colour_behaviour_no,
    "ammo_colour_behaviour_full_only", X_INTEGER, 
                                               ammo_colour_behaviour_full_only,
    "ammo_colour_behaviour_yes",       X_INTEGER,  ammo_colour_behaviour_yes
  );
}

/* vi: set et ts=2 sw=2: */

