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
#include "d_player.h"
#include "g_game.h"
#include "p_user.h"
#include "x_main.h"

static int XG_GameGetGametic(lua_State *L) {
  lua_pushinteger(L, gametic);

  return 1;
}

static int XG_GameHandleEvent(lua_State *L) {
  event_t *ev = luaL_checkudata(L, -1, "InputEvent");
  bool event_handled = G_Responder(ev);

  lua_pushboolean(L, event_handled);

  return 1;
}

static int XG_GameGetConsoleplayerMessages(lua_State *L) {
  GPtrArray *cpms = players[consoleplayer].messages.messages;
  GString *messages = g_string_new("");

  for (guint i = 0; i < cpms->len; i++) {
    player_message_t *pm = (player_message_t *)g_ptr_array_index(cpms, i);

    if (!pm->centered)
      g_string_append(messages, pm->content);
  }

  lua_pushstring(L, messages->str);

  g_string_free(messages, true);

  return 1;
}

static int XG_GameGetConsoleplayerMessagesUpdated(lua_State *L) {
  if (players[consoleplayer].messages.updated)
    lua_pushboolean(L, true);
  else
    lua_pushboolean(L, false);

  return 1;
}

static int XG_GameClearConsoleplayerMessagesUpdated(lua_State *L) {
  P_ClearMessagesUpdated(consoleplayer);

  return 0;
}

void XG_GameRegisterInterface(void) {
  X_RegisterObjects("Game", 5,
    "get_gametic",  X_FUNCTION, XG_GameGetGametic,
    "handle_event", X_FUNCTION, XG_GameHandleEvent,
    "get_consoleplayer_messages", X_FUNCTION, XG_GameGetConsoleplayerMessages,
    "get_consoleplayer_messages_updated", X_FUNCTION,
      XG_GameGetConsoleplayerMessagesUpdated,
    "clear_consoleplayer_messages_updated", X_FUNCTION,
      XG_GameClearConsoleplayerMessagesUpdated
  );
}

/* vi: set et ts=2 sw=2: */

