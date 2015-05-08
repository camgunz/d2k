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

#include <SDL.h>

#include "doomstat.h"
#include "d_event.h"
#include "am_map.h"
#include "d_main.h"
#include "g_game.h"
#include "g_keys.h"
#include "hu_stuff.h"
#include "i_input.h"
#include "m_menu.h"
#include "m_misc.h"
#include "st_stuff.h"
#include "x_main.h"
#include "xd_main.h"

static int XD_HandleEvent(lua_State *L) {
  event_t *ev = luaL_checkudata(L, -1, "InputEvent");
  bool event_handled = D_Responder(ev);

  lua_pushboolean(L, event_handled);

  return 1;
}

#if 0
static int XI_ResponderDispatch(lua_State *L) {
  event_t *ev = (event_t *)luaL_checkudata(L, -1, "InputEvent");

  if (I_Responder(ev))
    return 0;

  if (nodrawers) {
    G_Responder(ev);
    return 0;
  }

  /*
   * killough 2/22/98: add support for screenshot key:
   * cph 2001/02/04: no need for this to be a gameaction, just do it
   */
  if ((ev->type == ev_key && ev->pressed) && ev->key == key_screenshot)
    M_ScreenShot(); /* Don't eat the keypress. See sf bug #1843280. */

  /* CG 7/30/2014: ESC ought to quit chat if it's active */
#ifdef ENABLE_OVERLAY
  if ((ev->type == ev_key && ev->pressed) &&
      ev->key == SDLK_ESCAPE &&
      HU_ChatActive()) {
    HU_DeactivateChat();
    return 0;
  }
#endif

  if (menuactive) {
    M_Responder(ev);
    return 0;
  }

#ifdef ENABLE_OVERLAY
  if (HU_ChatActive()) {
    HU_Responder(ev);
    return 0;
  }

  if (C_Active()) {
    C_Responder(ev);
    return 0;
  }
#endif

  if (HU_Responder(ev))
    return 0;

  if (M_Responder(ev))
    return 0;

#ifdef ENABLE_OVERLAY
  if (C_Responder(ev))
    return 0;
#endif

  if (D_Responder(ev))
    return 0;

  if (ST_Responder(ev))
    return 0;

  if (AM_Responder(ev))
    return 0;

  G_Responder(ev);

  return 0;
}
#endif

void XD_RegisterInterface(void) {
  X_RegisterObjects("Main", 1,
    "handle_event", X_FUNCTION, XD_HandleEvent
  );
}

/* vi: set et ts=2 sw=2: */

