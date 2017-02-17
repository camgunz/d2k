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

#include "c_main.h"
#include "x_intern.h"
#include "x_main.h"

int XD_Msg(lua_State *L) {
  msg_channel_e chan = luaL_checkinteger(L, 1);
  const char *input_text = luaL_checkstring(L, 2);

  D_Msg(chan, "%s", input_text);

  return 0;
}

int XD_MsgEcho(lua_State *L) {
  const char *input_text = luaL_checkstring(L, 1);

  C_Echo(input_text);

  return 0;
}

int XD_MsgMEcho(lua_State *L) {
  const char *input_text = luaL_checkstring(L, 1);

  C_MEcho(input_text);

  return 0;
}

int XD_MsgWrite(lua_State *L) {
  const char *input_text = luaL_checkstring(L, 1);

  C_Write(input_text);

  return 0;
}

int XD_MsgMWrite(lua_State *L) {
  const char *input_text = luaL_checkstring(L, 1);

  C_MWrite(input_text);

  return 0;
}


static int XD_MsgGetMessages(lua_State *L) {
  GString *messages = C_GetMessages();

  if (!messages->len) {
    lua_pushnil(L);

    return 1;
  }

  lua_pushlstring(L, messages->str, messages->len);

  return 1;
}

static int XD_MsgGetMessagesUpdated(lua_State *L) {
  if (C_MessagesUpdated()) {
    lua_pushboolean(L, true);
  }
  else {
    lua_pushboolean(L, false);
  }

  return 1;
}

static int XD_MsgClearMessagesUpdated(lua_State *L) {
  C_ClearMessagesUpdated();

  return 0;
}

void XD_MsgRegisterInterface(void) {
  X_RegisterObjects("Messaging", 14,
    "DEBUG",                          X_INTEGER,  MSG_DEBUG,
    "INFO",                           X_INTEGER,  MSG_INFO,
    "WARN",                           X_INTEGER,  MSG_WARN,
    "ERROR",                          X_INTEGER,  MSG_ERROR,
    "DEH",                            X_INTEGER,  MSG_DEH,
    "GAME",                           X_INTEGER,  MSG_GAME,
    "print",                          X_FUNCTION, XD_Msg,
    "echo",                           X_FUNCTION, XD_MsgEcho,
    "mecho",                          X_FUNCTION, XD_MsgMEcho,
    "write",                          X_FUNCTION, XD_MsgWrite,
    "mwrite",                         X_FUNCTION, XD_MsgMWrite,
    "get_console_messages",           X_FUNCTION, XD_MsgGetMessages,
    "get_console_messages_updated",   X_FUNCTION, XD_MsgGetMessagesUpdated,
    "clear_console_messages_updated", X_FUNCTION, XD_MsgClearMessagesUpdated
  );
}

/* vi: set et ts=2 sw=2: */

