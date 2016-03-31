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
#include "g_game.h"
#include "n_net.h"
#include "x_intern.h"
#include "x_main.h"

static int XN_IsSingleplayer(lua_State *L) {
  if (SINGLEPLAYER)
    lua_pushboolean(L, true);
  else
    lua_pushboolean(L, false);

  return 1;
}

static int XN_IsMultiplayer(lua_State *L) {
  if (MULTINET)
    lua_pushboolean(L, true);
  else
    lua_pushboolean(L, false);

  return 1;
}

static int XN_IsClient(lua_State *L) {
  if (CLIENT)
    lua_pushboolean(L, true);
  else
    lua_pushboolean(L, false);

  return 1;
}

static int XN_IsServer(lua_State *L) {
  if (SERVER)
    lua_pushboolean(L, true);
  else
    lua_pushboolean(L, false);

  return 1;
}

static int XN_Connect(lua_State *L) {
  const char *address = luaL_checkstring(L, 1);
  bool result = N_ConnectToServer(address);

  lua_pushboolean(L, result);

  return 1;
}

static int XN_Disconnect(lua_State *L) {
  N_Disconnect();

  return 0;
}

static int XN_Reconnect(lua_State *L) {
  bool result = N_Reconnect();

  lua_pushboolean(L, result);

  return 1;
}

void XN_RegisterInterface(void) {
  X_RegisterObjects("Net", 7,
    "is_singleplayer", X_FUNCTION, XN_IsSingleplayer,
    "is_multiplayer",  X_FUNCTION, XN_IsMultiplayer,
    "is_client",       X_FUNCTION, XN_IsClient,
    "is_server",       X_FUNCTION, XN_IsServer,
    "connect",         X_FUNCTION, XN_Connect,
    "disconnect",      X_FUNCTION, XN_Disconnect,
    "reconnect",       X_FUNCTION, XN_Reconnect
  );
}

/* vi: set et ts=2 sw=2: */

