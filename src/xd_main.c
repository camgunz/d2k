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

#include "d_event.h"
#include "x_main.h"

static int XF_InputEventIsKey(lua_State *L) {
  event_t *ev = (event_t *)luaL_checkudata(L, -1, "InputEvent");

  if (ev->type == ev_key)
    lua_pushboolean(L, true);
  else
    lua_pushboolean(L, false);

  return 1;
}

static int XF_InputEventIsMouse(lua_State *L) {
  event_t *ev = (event_t *)luaL_checkudata(L, -1, "InputEvent");

  if (ev->type == ev_mouse)
    lua_pushboolean(L, true);
  else
    lua_pushboolean(L, false);

  return 1;
}

static int XF_InputEventIsJoystick(lua_State *L) {
  event_t *ev = (event_t *)luaL_checkudata(L, -1, "InputEvent");

  if (ev->type == ev_joystick)
    lua_pushboolean(L, true);
  else
    lua_pushboolean(L, false);

  return 1;
}

static int XF_InputEventIsMouseMovement(lua_State *L) {
  event_t *ev = (event_t *)luaL_checkudata(L, -1, "InputEvent");

  if (ev->type == ev_mouse_movement)
    lua_pushboolean(L, true);
  else
    lua_pushboolean(L, false);

  return 1;
}

static int XF_InputEventIsJoystickMovement(lua_State *L) {
  event_t *ev = (event_t *)luaL_checkudata(L, -1, "InputEvent");

  if (ev->type == ev_joystick_movement)
    lua_pushboolean(L, true);
  else
    lua_pushboolean(L, false);

  return 1;
}

static int XF_InputEventIsJoystickAxis(lua_State *L) {
  event_t *ev = (event_t *)luaL_checkudata(L, -1, "InputEvent");

  if (ev->type == ev_joystick_movement && ev->jstype == ev_joystick_axis)
    lua_pushboolean(L, true);
  else
    lua_pushboolean(L, false);

  return 1;
}

static int XF_InputEventIsJoystickBall(lua_State *L) {
  event_t *ev = (event_t *)luaL_checkudata(L, -1, "InputEvent");

  if (ev->type == ev_joystick_movement && ev->jstype == ev_joystick_ball)
    lua_pushboolean(L, true);
  else
    lua_pushboolean(L, false);

  return 1;
}

static int XF_InputEventIsJoystickHat(lua_State *L) {
  event_t *ev = (event_t *)luaL_checkudata(L, -1, "InputEvent");

  if (ev->type == ev_joystick_movement && ev->jstype == ev_joystick_hat)
    lua_pushboolean(L, true);
  else
    lua_pushboolean(L, false);

  return 1;
}

static int XF_InputEventIsMovement(lua_State *L) {
  event_t *ev = (event_t *)luaL_checkudata(L, -1, "InputEvent");

  if (ev->type == ev_mouse_movement || ev->type == ev_joystick_movement)
    lua_pushboolean(L, true);
  else
    lua_pushboolean(L, false);

  return 1;
}

static int XF_InputEventIsPress(lua_State *L) {
  event_t *ev = (event_t *)luaL_checkudata(L, -1, "InputEvent");

  if (((ev->type == ev_key)   ||
       (ev->type == ev_mouse) ||
       (ev->type == ev_joystick)) && ev->pressed)
    lua_pushboolean(L, true);
  else
    lua_pushboolean(L, false);

  return 1;
}

static int XF_InputEventIsRelease(lua_State *L) {
  event_t *ev = (event_t *)luaL_checkudata(L, -1, "InputEvent");

  if (((ev->type == ev_key)   ||
       (ev->type == ev_mouse) ||
       (ev->type == ev_joystick)) && !ev->pressed)
    lua_pushboolean(L, true);
  else
    lua_pushboolean(L, false);

  return 1;
}

static int XF_InputEventGetKey(lua_State *L) {
  event_t *ev = (event_t *)luaL_checkudata(L, -1, "InputEvent");

  if (ev->type == ev_key)
    lua_pushnumber(L, ev->key);
  else
    lua_pushnumber(L, 0);

  return 1;
}

static int XF_InputEventGetKeyName(lua_State *L) {
  event_t *ev = (event_t *)luaL_checkudata(L, -1, "InputEvent");

  if (ev->type == ev_key)
    lua_pushstring(L, SDL_GetKeyName(ev->key));
  else
    lua_pushstring(L, "[unknown]");

  return 1;
}

static int XF_InputEventGetValue(lua_State *L) {
  event_t *ev = (event_t *)luaL_checkudata(L, -1, "InputEvent");

  if (ev->type == ev_joystick_movement && (ev->jstype == ev_joystick_axis ||
                                           ev->jstype == ev_joystick_hat)) {
    lua_pushnumber(L, ev->value);
  }
  else {
    lua_pushnumber(L, 0);
  }

  return 1;
}

static int XF_InputEventGetXMove(lua_State *L) {
  event_t *ev = (event_t *)luaL_checkudata(L, -1, "InputEvent");

  if (ev->type == ev_mouse_movement ||
      (ev->type == ev_joystick_movement && ev->jstype == ev_joystick_ball)) {
    lua_pushnumber(L, ev->xmove);
  }
  else {
    lua_pushnumber(L, 0);
  }

  return 1;
}

static int XF_InputEventGetYMove(lua_State *L) {
  event_t *ev = (event_t *)luaL_checkudata(L, -1, "InputEvent");

  if (ev->type == ev_mouse_movement ||
      (ev->type == ev_joystick_movement && ev->jstype == ev_joystick_ball)) {
    lua_pushnumber(L, ev->ymove);
  }
  else {
    lua_pushnumber(L, 0);
  }

  return 1;
}

static int XF_InputEventGetChar(lua_State *L) {
  /* CG: [TODO] Convert this to UTF-8 before sending to scripting */
  event_t *ev = (event_t *)luaL_checkudata(L, -1, "InputEvent");

  if (ev->type == ev_key && ev->pressed)
    lua_pushnumber(L, ev->wchar);
  else
    lua_pushnumber(L, 0);

  return 1;
}

void XD_RegisterInterface(void) {
  /* CG: [TODO] Add functions to return names for mouse & joystick buttons */
  X_RegisterObjects("InputEvent", 8,
    "is_key",               X_FUNC, XF_InputEventIsKey,
    "is_mouse",             X_FUNC, XF_InputEventIsMouse,
    "is_joystick",          X_FUNC, XF_InputEventIsJoystick,
    "is_mouse_movement",    X_FUNC, XF_InputEventIsMouseMovement,
    "is_joystick_movement", X_FUNC, XF_InputEventIsJoystickMovement,
    "is_joystick_axis",     X_FUNC, XF_InputEventIsJoystickAxis,
    "is_joystick_ball",     X_FUNC, XF_InputEventIsJoystickBall,
    "is_joystick_hat",      X_FUNC, XF_InputEventIsJoystickHat,
    "is_movement",          X_FUNC, XF_InputEventIsMovement,
    "is_press",             X_FUNC, XF_InputEventIsPress,
    "is_release",           X_FUNC, XF_InputEventIsRelease,
    "get_key",              X_FUNC, XF_InputEventGetKey,
    "get_key_name",         X_FUNC, XF_InputEventGetKeyName,
    "get_value",            X_FUNC, XF_InputEventGetValue,
    "get_xmove",            X_FUNC, XF_InputEventGetXMove,
    "get_ymove",            X_FUNC, XF_InputEventGetYMove,
    "get_char",             X_FUNC, XF_InputEventGetChar
  );

  X_RegisterObjects("Key", 133,
    "BACKSPACE",    X_NUM, SDLK_BACKSPACE,
    "TAB",          X_NUM, SDLK_TAB,
    "CLEAR",        X_NUM, SDLK_CLEAR,
    "RETURN",       X_NUM, SDLK_RETURN,
    "PAUSE",        X_NUM, SDLK_PAUSE,
    "ESCAPE",       X_NUM, SDLK_ESCAPE,
    "SPACE",        X_NUM, SDLK_SPACE,
    "EXCLAIM",      X_NUM, SDLK_EXCLAIM,
    "QUOTEDBL",     X_NUM, SDLK_QUOTEDBL,
    "HASH",         X_NUM, SDLK_HASH,
    "DOLLAR",       X_NUM, SDLK_DOLLAR,
    "AMPERSAND",    X_NUM, SDLK_AMPERSAND,
    "QUOTE",        X_NUM, SDLK_QUOTE,
    "LEFTPAREN",    X_NUM, SDLK_LEFTPAREN,
    "RIGHTPAREN",   X_NUM, SDLK_RIGHTPAREN,
    "ASTERISK",     X_NUM, SDLK_ASTERISK,
    "PLUS",         X_NUM, SDLK_PLUS,
    "COMMA",        X_NUM, SDLK_COMMA,
    "MINUS",        X_NUM, SDLK_MINUS,
    "PERIOD",       X_NUM, SDLK_PERIOD,
    "SLASH",        X_NUM, SDLK_SLASH,
    "ZERO",         X_NUM, SDLK_0,
    "ONE",          X_NUM, SDLK_1,
    "TWO",          X_NUM, SDLK_2,
    "THREE",        X_NUM, SDLK_3,
    "FOUR",         X_NUM, SDLK_4,
    "FIVE",         X_NUM, SDLK_5,
    "SIX",          X_NUM, SDLK_6,
    "SEVEN",        X_NUM, SDLK_7,
    "EIGHT",        X_NUM, SDLK_8,
    "NINE",         X_NUM, SDLK_9,
    "COLON",        X_NUM, SDLK_COLON,
    "SEMICOLON",    X_NUM, SDLK_SEMICOLON,
    "LESS",         X_NUM, SDLK_LESS,
    "EQUALS",       X_NUM, SDLK_EQUALS,
    "GREATER",      X_NUM, SDLK_GREATER,
    "QUESTION",     X_NUM, SDLK_QUESTION,
    "AT",           X_NUM, SDLK_AT,
    "LEFTBRACKET",  X_NUM, SDLK_LEFTBRACKET,
    "BACKSLASH",    X_NUM, SDLK_BACKSLASH,
    "RIGHTBRACKET", X_NUM, SDLK_RIGHTBRACKET,
    "CARET",        X_NUM, SDLK_CARET,
    "UNDERSCORE",   X_NUM, SDLK_UNDERSCORE,
    "BACKQUOTE",    X_NUM, SDLK_BACKQUOTE,
    "a",            X_NUM, SDLK_a,
    "b",            X_NUM, SDLK_b,
    "c",            X_NUM, SDLK_c,
    "d",            X_NUM, SDLK_d,
    "e",            X_NUM, SDLK_e,
    "f",            X_NUM, SDLK_f,
    "g",            X_NUM, SDLK_g,
    "h",            X_NUM, SDLK_h,
    "i",            X_NUM, SDLK_i,
    "j",            X_NUM, SDLK_j,
    "k",            X_NUM, SDLK_k,
    "l",            X_NUM, SDLK_l,
    "m",            X_NUM, SDLK_m,
    "n",            X_NUM, SDLK_n,
    "o",            X_NUM, SDLK_o,
    "p",            X_NUM, SDLK_p,
    "q",            X_NUM, SDLK_q,
    "r",            X_NUM, SDLK_r,
    "s",            X_NUM, SDLK_s,
    "t",            X_NUM, SDLK_t,
    "u",            X_NUM, SDLK_u,
    "v",            X_NUM, SDLK_v,
    "w",            X_NUM, SDLK_w,
    "x",            X_NUM, SDLK_x,
    "y",            X_NUM, SDLK_y,
    "z",            X_NUM, SDLK_z,
    "DELETE",       X_NUM, SDLK_DELETE,
    "KP0",          X_NUM, SDLK_KP0,
    "KP1",          X_NUM, SDLK_KP1,
    "KP2",          X_NUM, SDLK_KP2,
    "KP3",          X_NUM, SDLK_KP3,
    "KP4",          X_NUM, SDLK_KP4,
    "KP5",          X_NUM, SDLK_KP5,
    "KP6",          X_NUM, SDLK_KP6,
    "KP7",          X_NUM, SDLK_KP7,
    "KP8",          X_NUM, SDLK_KP8,
    "KP9",          X_NUM, SDLK_KP9,
    "KP_PERIOD",    X_NUM, SDLK_KP_PERIOD,
    "KP_DIVIDE",    X_NUM, SDLK_KP_DIVIDE,
    "KP_MULTIPLY",  X_NUM, SDLK_KP_MULTIPLY,
    "KP_MINUS",     X_NUM, SDLK_KP_MINUS,
    "KP_PLUS",      X_NUM, SDLK_KP_PLUS,
    "KP_ENTER",     X_NUM, SDLK_KP_ENTER,
    "KP_EQUALS",    X_NUM, SDLK_KP_EQUALS,
    "UP",           X_NUM, SDLK_UP,
    "DOWN",         X_NUM, SDLK_DOWN,
    "RIGHT",        X_NUM, SDLK_RIGHT,
    "LEFT",         X_NUM, SDLK_LEFT,
    "INSERT",       X_NUM, SDLK_INSERT,
    "HOME",         X_NUM, SDLK_HOME,
    "END",          X_NUM, SDLK_END,
    "PAGEUP",       X_NUM, SDLK_PAGEUP,
    "PAGEDOWN",     X_NUM, SDLK_PAGEDOWN,
    "F1",           X_NUM, SDLK_F1,
    "F2",           X_NUM, SDLK_F2,
    "F3",           X_NUM, SDLK_F3,
    "F4",           X_NUM, SDLK_F4,
    "F5",           X_NUM, SDLK_F5,
    "F6",           X_NUM, SDLK_F6,
    "F7",           X_NUM, SDLK_F7,
    "F8",           X_NUM, SDLK_F8,
    "F9",           X_NUM, SDLK_F9,
    "F10",          X_NUM, SDLK_F10,
    "F11",          X_NUM, SDLK_F11,
    "F12",          X_NUM, SDLK_F12,
    "F13",          X_NUM, SDLK_F13,
    "F14",          X_NUM, SDLK_F14,
    "F15",          X_NUM, SDLK_F15,
    "NUMLOCK",      X_NUM, SDLK_NUMLOCK,
    "CAPSLOCK",     X_NUM, SDLK_CAPSLOCK,
    "SCROLLOCK",    X_NUM, SDLK_SCROLLOCK,
    "RSHIFT",       X_NUM, SDLK_RSHIFT,
    "LSHIFT",       X_NUM, SDLK_LSHIFT,
    "RCTRL",        X_NUM, SDLK_RCTRL,
    "LCTRL",        X_NUM, SDLK_LCTRL,
    "RALT",         X_NUM, SDLK_RALT,
    "LALT",         X_NUM, SDLK_LALT,
    "RMETA",        X_NUM, SDLK_RMETA,
    "LMETA",        X_NUM, SDLK_LMETA,
    "LSUPER",       X_NUM, SDLK_LSUPER,
    "RSUPER",       X_NUM, SDLK_RSUPER,
    "MODE",         X_NUM, SDLK_MODE,
    "HELP",         X_NUM, SDLK_HELP,
    "PRINT",        X_NUM, SDLK_PRINT,
    "SYSREQ",       X_NUM, SDLK_SYSREQ,
    "BREAK",        X_NUM, SDLK_BREAK,
    "MENU",         X_NUM, SDLK_MENU,
    "POWER",        X_NUM, SDLK_POWER,
    "EURO",         X_NUM, SDLK_EURO
  );

  X_RegisterObjects("Mouse", 7,
    "LEFT",      X_NUM, SDL_BUTTON_LEFT,
    "MIDDLE",    X_NUM, SDL_BUTTON_MIDDLE,
    "RIGHT",     X_NUM, SDL_BUTTON_RIGHT,
    "WHEELUP",   X_NUM, SDL_BUTTON_WHEELUP,
    "WHEELDOWN", X_NUM, SDL_BUTTON_WHEELDOWN,
    "X1",        X_NUM, SDL_BUTTON_X1,
    "X2",        X_NUM, SDL_BUTTON_X2
  );

  X_RegisterObjects("Joystick", 9,
    "HAT_CENTERED",  X_NUM, SDL_HAT_CENTERED,
    "HAT_UP",        X_NUM, SDL_HAT_UP,
    "HAT_RIGHT",     X_NUM, SDL_HAT_RIGHT,
    "HAT_DOWN",      X_NUM, SDL_HAT_DOWN,
    "HAT_LEFT",      X_NUM, SDL_HAT_LEFT,
    "HAT_RIGHTUP",   X_NUM, SDL_HAT_RIGHTUP,
    "HAT_RIGHTDOWN", X_NUM, SDL_HAT_RIGHTDOWN,
    "HAT_LEFTUP",    X_NUM, SDL_HAT_LEFTUP,
    "HAT_LEFTDOWN",  X_NUM, SDL_HAT_LEFTDOWN
  );
}

/* vi: set et ts=2 sw=2: */

