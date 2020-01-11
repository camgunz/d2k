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
#include "d_event.h"
#include "m_fixed.h"
#include "am_map.h"
#include "x_intern.h"
#include "x_main.h"

bool AM_Responder(event_t *ev);

static int XAM_HandleEvent(lua_State *L) {
  event_t *ev = luaL_checkudata(L, -1, "InputEvent");
  bool event_handled = AM_Responder(ev);

  lua_pushboolean(L, event_handled);

  return 1;
}

void XAM_RegisterInterface(void) {
  X_RegisterObjects("AutoMap", 4,
    "handle_event",                  X_FUNCTION, XAM_HandleEvent,
    "map_things_appearance_classic", X_INTEGER,  map_things_appearance_classic,
    "map_things_appearance_scaled",  X_INTEGER,  map_things_appearance_scaled,
    "map_things_appearance_icon",    X_INTEGER,  map_things_appearance_icon
  );
}

/* vi: set et ts=2 sw=2: */

