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
#include "doomstat.h"
#include "d_event.h"
#include "g_game.h"
#include "m_argv.h"
#include "n_main.h"
#include "p_spec.h"
#include "p_user.h"
#include "x_intern.h"
#include "x_main.h"

static int XG_GameGetGametic(lua_State *L) {
  lua_pushinteger(L, gametic);

  return 1;
}

static int XG_GameGetGameMode(lua_State *L) {
  if (deathmatch) {
    lua_pushstring(L, "deathmatch");
  }
  else if (MULTINET) {
    lua_pushstring(L, "cooperative");
  }
  else if (solonet) {
    lua_pushstring(L, "solonet");
  }
  else {
    lua_pushstring(L, "singleplayer");
  }

  return 1;
}

static int XG_GameIsMultiplayer(lua_State *L) {
  if (deathmatch || MULTINET) {
    lua_pushboolean(L, true);
  }
  else {
    lua_pushboolean(L, false);
  }

  return 1;
}

static int XG_GameGetFragLimit(lua_State *L) {
  if (deathmatch) {
    lua_pushinteger(L, levelFragLimitCount);
  }
  else {
    lua_pushnil(L);
  }

  return 1;
}

static int XG_GameGetTimeLimit(lua_State *L) {
  int i = M_CheckParm("-timer"); /* user defined timer on game play */

  if (i && deathmatch) {
    int time = atoi(myargv[i + 1]) * 60;

    lua_pushinteger(L, time);
  }
  else {
    lua_pushinteger(L, 0);
  }

  return 1;
}

static int XG_GameGetPlayers(lua_State *L) {
  int player_count = 0;
  int player_start = 0;

  if (MULTINET) {
    player_start = 1;
  }

  for (int i = player_start; i < MAXPLAYERS; i++) {
    if (playeringame[i]) {
      player_count++;
    }
  }

  lua_createtable(L, player_count, 0); // 1

  for (int i = player_start; i < MAXPLAYERS; i++) {
    if (!playeringame[i])
      continue;

    lua_createtable(L, 0, 5); // 2

    lua_pushstring(L, players[i].name); // 3
    lua_setfield(L, 2, "name");

    lua_pushinteger(L, P_PlayerGetFragCount(i)); // 3
    lua_setfield(L, 2, "frags");

    lua_pushinteger(L, P_PlayerGetDeathCount(i)); // 4
    lua_setfield(L, 2, "deaths");

    lua_pushinteger(L, P_PlayerGetPing(i)); // 5
    lua_setfield(L, 2, "ping");

    lua_pushinteger(L, P_PlayerGetTime(i)); // 6
    lua_setfield(L, 2, "time");

    lua_rawseti(L, 1, (i + 1) - player_start); // players[i + 1] = player
  }

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
  GString *messages;

  if (!cpms->len) {
    lua_pushnil(L);
    return 1;
  }

  messages = g_string_new("");

  for (guint i = 0; i < cpms->len; i++) {
    player_message_t *pm = (player_message_t *)g_ptr_array_index(cpms, i);

    if (pm->centered)
      continue;

    g_string_append(messages, pm->content);
  }

  lua_pushlstring(L, messages->str, messages->len);

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

static int XG_GameTick(lua_State *L) {
  G_Ticker();

  return 0;
}

static int XG_GameRender(lua_State *L) {
  G_Drawer();

  return 0;
}

static int XG_GameInLevel(lua_State *L) {
  if (G_GetGameState() == GS_LEVEL)
    lua_pushboolean(L, true);
  else
    lua_pushboolean(L, false);

  return 1;
}

static int XG_GameInIntermission(lua_State *L) {
  if (G_GetGameState() == GS_INTERMISSION)
    lua_pushboolean(L, true);
  else
    lua_pushboolean(L, false);

  return 1;
}

static int XG_GameInFinale(lua_State *L) {
  if (G_GetGameState() == GS_FINALE)
    lua_pushboolean(L, true);
  else
    lua_pushboolean(L, false);

  return 1;
}

static int XG_GameInDemoScreen(lua_State *L) {
  if (G_GetGameState() == GS_DEMOSCREEN)
    lua_pushboolean(L, true);
  else
    lua_pushboolean(L, false);

  return 1;
}

void XG_GameRegisterInterface(void) {
  X_RegisterObjects("Game", 16,
    "tick",            X_FUNCTION, XG_GameTick,
    "render",          X_FUNCTION, XG_GameRender,
    "in_level",        X_FUNCTION, XG_GameInLevel,
    "in_intermission", X_FUNCTION, XG_GameInIntermission,
    "in_finale",       X_FUNCTION, XG_GameInFinale,
    "in_demoscreen",   X_FUNCTION, XG_GameInDemoScreen,
    "handle_event",    X_FUNCTION, XG_GameHandleEvent,
    "get_gametic",     X_FUNCTION, XG_GameGetGametic,
    "get_game_mode",   X_FUNCTION, XG_GameGetGameMode,
    "is_multiplayer",  X_FUNCTION, XG_GameIsMultiplayer,
    "get_frag_limit",  X_FUNCTION, XG_GameGetFragLimit,
    "get_time_limit",  X_FUNCTION, XG_GameGetTimeLimit,
    "get_players",     X_FUNCTION, XG_GameGetPlayers,
    "get_consoleplayer_messages", X_FUNCTION,
      XG_GameGetConsoleplayerMessages,
    "get_consoleplayer_messages_updated", X_FUNCTION,
      XG_GameGetConsoleplayerMessagesUpdated,
    "clear_consoleplayer_messages_updated", X_FUNCTION,
      XG_GameClearConsoleplayerMessagesUpdated
  );
}

/* vi: set et ts=2 sw=2: */

