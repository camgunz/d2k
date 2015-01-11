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
    "is_key",               X_FUNCTION, XF_InputEventIsKey,
    "is_mouse",             X_FUNCTION, XF_InputEventIsMouse,
    "is_joystick",          X_FUNCTION, XF_InputEventIsJoystick,
    "is_mouse_movement",    X_FUNCTION, XF_InputEventIsMouseMovement,
    "is_joystick_movement", X_FUNCTION, XF_InputEventIsJoystickMovement,
    "is_joystick_axis",     X_FUNCTION, XF_InputEventIsJoystickAxis,
    "is_joystick_ball",     X_FUNCTION, XF_InputEventIsJoystickBall,
    "is_joystick_hat",      X_FUNCTION, XF_InputEventIsJoystickHat,
    "is_movement",          X_FUNCTION, XF_InputEventIsMovement,
    "is_press",             X_FUNCTION, XF_InputEventIsPress,
    "is_release",           X_FUNCTION, XF_InputEventIsRelease,
    "get_key",              X_FUNCTION, XF_InputEventGetKey,
    "get_key_name",         X_FUNCTION, XF_InputEventGetKeyName,
    "get_value",            X_FUNCTION, XF_InputEventGetValue,
    "get_xmove",            X_FUNCTION, XF_InputEventGetXMove,
    "get_ymove",            X_FUNCTION, XF_InputEventGetYMove,
    "get_char",             X_FUNCTION, XF_InputEventGetChar
  );

  X_RegisterObjects("Key", 133,
    "BACKSPACE",    X_INTEGER, SDLK_BACKSPACE,
    "TAB",          X_INTEGER, SDLK_TAB,
    "CLEAR",        X_INTEGER, SDLK_CLEAR,
    "RETURN",       X_INTEGER, SDLK_RETURN,
    "PAUSE",        X_INTEGER, SDLK_PAUSE,
    "ESCAPE",       X_INTEGER, SDLK_ESCAPE,
    "SPACE",        X_INTEGER, SDLK_SPACE,
    "EXCLAIM",      X_INTEGER, SDLK_EXCLAIM,
    "QUOTEDBL",     X_INTEGER, SDLK_QUOTEDBL,
    "HASH",         X_INTEGER, SDLK_HASH,
    "DOLLAR",       X_INTEGER, SDLK_DOLLAR,
    "AMPERSAND",    X_INTEGER, SDLK_AMPERSAND,
    "QUOTE",        X_INTEGER, SDLK_QUOTE,
    "LEFTPAREN",    X_INTEGER, SDLK_LEFTPAREN,
    "RIGHTPAREN",   X_INTEGER, SDLK_RIGHTPAREN,
    "ASTERISK",     X_INTEGER, SDLK_ASTERISK,
    "PLUS",         X_INTEGER, SDLK_PLUS,
    "COMMA",        X_INTEGER, SDLK_COMMA,
    "MINUS",        X_INTEGER, SDLK_MINUS,
    "PERIOD",       X_INTEGER, SDLK_PERIOD,
    "SLASH",        X_INTEGER, SDLK_SLASH,
    "ZERO",         X_INTEGER, SDLK_0,
    "ONE",          X_INTEGER, SDLK_1,
    "TWO",          X_INTEGER, SDLK_2,
    "THREE",        X_INTEGER, SDLK_3,
    "FOUR",         X_INTEGER, SDLK_4,
    "FIVE",         X_INTEGER, SDLK_5,
    "SIX",          X_INTEGER, SDLK_6,
    "SEVEN",        X_INTEGER, SDLK_7,
    "EIGHT",        X_INTEGER, SDLK_8,
    "NINE",         X_INTEGER, SDLK_9,
    "COLON",        X_INTEGER, SDLK_COLON,
    "SEMICOLON",    X_INTEGER, SDLK_SEMICOLON,
    "LESS",         X_INTEGER, SDLK_LESS,
    "EQUALS",       X_INTEGER, SDLK_EQUALS,
    "GREATER",      X_INTEGER, SDLK_GREATER,
    "QUESTION",     X_INTEGER, SDLK_QUESTION,
    "AT",           X_INTEGER, SDLK_AT,
    "LEFTBRACKET",  X_INTEGER, SDLK_LEFTBRACKET,
    "BACKSLASH",    X_INTEGER, SDLK_BACKSLASH,
    "RIGHTBRACKET", X_INTEGER, SDLK_RIGHTBRACKET,
    "CARET",        X_INTEGER, SDLK_CARET,
    "UNDERSCORE",   X_INTEGER, SDLK_UNDERSCORE,
    "BACKQUOTE",    X_INTEGER, SDLK_BACKQUOTE,
    "a",            X_INTEGER, SDLK_a,
    "b",            X_INTEGER, SDLK_b,
    "c",            X_INTEGER, SDLK_c,
    "d",            X_INTEGER, SDLK_d,
    "e",            X_INTEGER, SDLK_e,
    "f",            X_INTEGER, SDLK_f,
    "g",            X_INTEGER, SDLK_g,
    "h",            X_INTEGER, SDLK_h,
    "i",            X_INTEGER, SDLK_i,
    "j",            X_INTEGER, SDLK_j,
    "k",            X_INTEGER, SDLK_k,
    "l",            X_INTEGER, SDLK_l,
    "m",            X_INTEGER, SDLK_m,
    "n",            X_INTEGER, SDLK_n,
    "o",            X_INTEGER, SDLK_o,
    "p",            X_INTEGER, SDLK_p,
    "q",            X_INTEGER, SDLK_q,
    "r",            X_INTEGER, SDLK_r,
    "s",            X_INTEGER, SDLK_s,
    "t",            X_INTEGER, SDLK_t,
    "u",            X_INTEGER, SDLK_u,
    "v",            X_INTEGER, SDLK_v,
    "w",            X_INTEGER, SDLK_w,
    "x",            X_INTEGER, SDLK_x,
    "y",            X_INTEGER, SDLK_y,
    "z",            X_INTEGER, SDLK_z,
    "DELETE",       X_INTEGER, SDLK_DELETE,
    "KP0",          X_INTEGER, SDLK_KP0,
    "KP1",          X_INTEGER, SDLK_KP1,
    "KP2",          X_INTEGER, SDLK_KP2,
    "KP3",          X_INTEGER, SDLK_KP3,
    "KP4",          X_INTEGER, SDLK_KP4,
    "KP5",          X_INTEGER, SDLK_KP5,
    "KP6",          X_INTEGER, SDLK_KP6,
    "KP7",          X_INTEGER, SDLK_KP7,
    "KP8",          X_INTEGER, SDLK_KP8,
    "KP9",          X_INTEGER, SDLK_KP9,
    "KP_PERIOD",    X_INTEGER, SDLK_KP_PERIOD,
    "KP_DIVIDE",    X_INTEGER, SDLK_KP_DIVIDE,
    "KP_MULTIPLY",  X_INTEGER, SDLK_KP_MULTIPLY,
    "KP_MINUS",     X_INTEGER, SDLK_KP_MINUS,
    "KP_PLUS",      X_INTEGER, SDLK_KP_PLUS,
    "KP_ENTER",     X_INTEGER, SDLK_KP_ENTER,
    "KP_EQUALS",    X_INTEGER, SDLK_KP_EQUALS,
    "UP",           X_INTEGER, SDLK_UP,
    "DOWN",         X_INTEGER, SDLK_DOWN,
    "RIGHT",        X_INTEGER, SDLK_RIGHT,
    "LEFT",         X_INTEGER, SDLK_LEFT,
    "INSERT",       X_INTEGER, SDLK_INSERT,
    "HOME",         X_INTEGER, SDLK_HOME,
    "END",          X_INTEGER, SDLK_END,
    "PAGEUP",       X_INTEGER, SDLK_PAGEUP,
    "PAGEDOWN",     X_INTEGER, SDLK_PAGEDOWN,
    "F1",           X_INTEGER, SDLK_F1,
    "F2",           X_INTEGER, SDLK_F2,
    "F3",           X_INTEGER, SDLK_F3,
    "F4",           X_INTEGER, SDLK_F4,
    "F5",           X_INTEGER, SDLK_F5,
    "F6",           X_INTEGER, SDLK_F6,
    "F7",           X_INTEGER, SDLK_F7,
    "F8",           X_INTEGER, SDLK_F8,
    "F9",           X_INTEGER, SDLK_F9,
    "F10",          X_INTEGER, SDLK_F10,
    "F11",          X_INTEGER, SDLK_F11,
    "F12",          X_INTEGER, SDLK_F12,
    "F13",          X_INTEGER, SDLK_F13,
    "F14",          X_INTEGER, SDLK_F14,
    "F15",          X_INTEGER, SDLK_F15,
    "NUMLOCK",      X_INTEGER, SDLK_NUMLOCK,
    "CAPSLOCK",     X_INTEGER, SDLK_CAPSLOCK,
    "SCROLLOCK",    X_INTEGER, SDLK_SCROLLOCK,
    "RSHIFT",       X_INTEGER, SDLK_RSHIFT,
    "LSHIFT",       X_INTEGER, SDLK_LSHIFT,
    "RCTRL",        X_INTEGER, SDLK_RCTRL,
    "LCTRL",        X_INTEGER, SDLK_LCTRL,
    "RALT",         X_INTEGER, SDLK_RALT,
    "LALT",         X_INTEGER, SDLK_LALT,
    "RMETA",        X_INTEGER, SDLK_RMETA,
    "LMETA",        X_INTEGER, SDLK_LMETA,
    "LSUPER",       X_INTEGER, SDLK_LSUPER,
    "RSUPER",       X_INTEGER, SDLK_RSUPER,
    "MODE",         X_INTEGER, SDLK_MODE,
    "HELP",         X_INTEGER, SDLK_HELP,
    "PRINT",        X_INTEGER, SDLK_PRINT,
    "SYSREQ",       X_INTEGER, SDLK_SYSREQ,
    "BREAK",        X_INTEGER, SDLK_BREAK,
    "MENU",         X_INTEGER, SDLK_MENU,
    "POWER",        X_INTEGER, SDLK_POWER,
    "EURO",         X_INTEGER, SDLK_EURO
  );

  X_RegisterObjects("Mouse", 7,
    "LEFT",      X_INTEGER, SDL_BUTTON_LEFT,
    "MIDDLE",    X_INTEGER, SDL_BUTTON_MIDDLE,
    "RIGHT",     X_INTEGER, SDL_BUTTON_RIGHT,
    "WHEELUP",   X_INTEGER, SDL_BUTTON_WHEELUP,
    "WHEELDOWN", X_INTEGER, SDL_BUTTON_WHEELDOWN,
    "X1",        X_INTEGER, SDL_BUTTON_X1,
    "X2",        X_INTEGER, SDL_BUTTON_X2
  );

  X_RegisterObjects("Joystick", 9,
    "HAT_CENTERED",  X_INTEGER, SDL_HAT_CENTERED,
    "HAT_UP",        X_INTEGER, SDL_HAT_UP,
    "HAT_RIGHT",     X_INTEGER, SDL_HAT_RIGHT,
    "HAT_DOWN",      X_INTEGER, SDL_HAT_DOWN,
    "HAT_LEFT",      X_INTEGER, SDL_HAT_LEFT,
    "HAT_RIGHTUP",   X_INTEGER, SDL_HAT_RIGHTUP,
    "HAT_RIGHTDOWN", X_INTEGER, SDL_HAT_RIGHTDOWN,
    "HAT_LEFTUP",    X_INTEGER, SDL_HAT_LEFTUP,
    "HAT_LEFTDOWN",  X_INTEGER, SDL_HAT_LEFTDOWN
  );
}

/* vi: set et ts=2 sw=2: */

