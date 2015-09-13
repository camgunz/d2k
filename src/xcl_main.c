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

#include <enet/enet.h>

#include "n_net.h"
#include "n_peer.h"
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

static int XCL_GetNetStats(lua_State *L) {
  netpeer_t *server = NULL;

  if (!CLIENT)
    return 0;
  
  CL_GetServerPeer();

  lua_createtable(L, 0, 18);

  lua_pushinteger(L, N_GetUploadBandwidth());
  lua_setfield(L, 2, "upload");

  lua_pushinteger(L, N_GetDownloadBandwidth());
  lua_setfield(L, 2, "download");

  lua_pushinteger(L, server->lowestRoundTripTime);
  lua_setfield(L, 2, "ping_low");

  lua_pushinteger(L, server->lastRoundTripTime);
  lua_setfield(L, 2, "ping_last");

  lua_pushinteger(L, server->roundTripTime);
  lua_setfield(L, 2, "ping_average");

  lua_pushinteger(L, server->highestRoundTripTimeVariance);
  lua_setfield(L, 2, "jitter_high");

  lua_pushinteger(L, server->lastRoundTripTimeVariance);
  lua_setfield(L, 2, "jitter_last");

  lua_pushinteger(L, server->roundTripTimeVariance);
  lua_setfield(L, 2, "jitter_average");

  if (CLIENT)
    lua_pushinteger(L, CL_GetUnsynchronizedCommandCount(consoleplayer));
  else
    lua_pushinteger(L, 0);
  lua_setfield(L, 2, "unsynchronized_commands");

  lua_pushinteger(L, P_GetCommandCount(iter.np->playernum));
  lua_setfield(L, 2, "total_commands");

  lua_pushnumber(L,
    (server->packetLoss / (float)ENET_PEER_PACKET_LOSS_SCALE) * 100.f
  );
  lua_setfield(L, 2, "packet_loss");

  lua_pushnumber(L,
    (server->packetLossVariance / (float)ENET_PEER_PACKET_LOSS_SCALE) * 100.f
  );
  lua_setfield(L, 2, "packet_loss_jitter");

  lua_pushinteger(L, server->packetThrottle);
  lua_setfield(L, 2, "throttle");

  lua_pushinteger(L, server->packetThrottleAcceleration);
  lua_setfield(L, 2, "throttle_acceleration");

  lua_pushinteger(L, server->packetThrottleCounter);
  lua_setfield(L, 2, "throttle_counter");

  lua_pushinteger(L, server->packetThrottleDeceleration);
  lua_setfield(L, 2, "throttle_deceleration");

  lua_pushinteger(L, server->packetThrottleInterval);
  lua_setfield(L, 2, "throttle_interval");

  lua_pushinteger(L, server->packetThrottleLimit);
  lua_setfield(L, 2, "throttle_limit");

  return 1;
}

void XCL_RegisterInterface(void) {
  X_RegisterObjects("Client", 9,
    "say",                 X_FUNCTION, XCL_Say,
    "say_to",              X_FUNCTION, XCL_SayToPlayer,
    "say_to_server",       X_FUNCTION, XCL_SayToServer,
    "say_to_team",         X_FUNCTION, XCL_SayToTeam,
    "say_to_current_team", X_FUNCTION, XCL_SayToCurrentTeam,
    "set_name",            X_FUNCTION, XCL_SetName,
    "set_team",            X_FUNCTION, XCL_SetTeam,
    "set_bobbing",         X_FUNCTION, XCL_SetBobbing,
    "get_netstats",        X_FUNCTION, XCL_GetNetStats
  );
}

/* vi: set et ts=2 sw=2: */

