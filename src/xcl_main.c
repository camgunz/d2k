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

#include "n_net.h"
#include "n_proto.h"
#include "x_main.h"

static int XCL_Say(lua_State *L) {
  const char *message = luaL_checkstring(L, 1);

  CL_SendMessage(message);

  return 0;
}

static int XCL_SayToPlayer(lua_State *L) {
  int recipient = luaL_checkinteger(L, 1);
  const char *message = luaL_checkstring(L, 2);

  CL_SendMessageToPlayer(recipient, message);

  return 0;
}

static int XCL_SayToServer(lua_State *L) {
  const char *message = luaL_checkstring(L, 1);

  CL_SendMessageToServer(message);

  return 0;
}

static int XCL_SayToTeam(lua_State *L) {
  int team = luaL_checkinteger(L, 1);
  const char *message = luaL_checkstring(L, 2);

  CL_SendMessageToTeam(team, message);

  return 0;
}

static int XCL_SayToCurrentTeam(lua_State *L) {
  const char *message = luaL_checkstring(L, 1);

  CL_SendMessageToCurrentTeam(message);

  return 0;
}

static int XCL_SetName(lua_State *L) {
  const char *new_name = luaL_checkstring(L, 1);

  CL_SendNameChange(new_name);

  return 0;
}

static int XCL_SetTeam(lua_State *L) {
  int new_team = luaL_checkinteger(L, 1);

  CL_SendTeamChange(new_team);

  return 0;
}

static int XCL_SetBobbing(lua_State *L) {
  double new_bobbing_amount = luaL_checknumber(L, 1);

  CL_SendBobbingChange(new_bobbing_amount);

  return 0;
}

void XCL_RegisterInterface(void) {
  X_RegisterObjects("Client", 8,
    "say",                 X_FUNCTION, XCL_Say,
    "say_to",              X_FUNCTION, XCL_SayToPlayer,
    "say_to_server",       X_FUNCTION, XCL_SayToServer,
    "say_to_team",         X_FUNCTION, XCL_SayToTeam,
    "say_to_current_team", X_FUNCTION, XCL_SayToCurrentTeam,
    "set_name",            X_FUNCTION, XCL_SetName,
    "set_team",            X_FUNCTION, XCL_SetTeam,
    "set_bobbing",         X_FUNCTION, XCL_SetBobbing
  );
}

/* vi: set et ts=2 sw=2: */

