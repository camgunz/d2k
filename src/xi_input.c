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
#include "i_input.h"
#include "i_joy.h"
#include "i_mouse.h"
#include "i_video.h"
#include "m_misc.h"
#include "p_mobj.h"
#include "sounds.h"
#include "s_sound.h"
#include "x_intern.h"
#include "x_main.h"
#include "xi_input.h"

extern void M_QuitDOOM(int choice);

static void reset_input_event(event_t *ev) {
  ev->device_id = -1;
  ev->type      = ev_none;
  ev->jstype    = ev_joystick_none;
  ev->pressed   = false;
  ev->key       = 0;
  ev->value     = 0;
  ev->xmove     = 0;
  ev->ymove     = 0;
  ev->wchar     = 0;
}

static int XI_InputEventToString(lua_State *L) {
  event_t *ev = (event_t *)luaL_checkudata(L, -1, "InputEvent");
  GString *s = g_string_new("<InputEvent({");

  if (ev->type == ev_none)
    g_string_append(s, "type = none");
  else if (ev->type == ev_key)
    g_string_append(s, "type = key");
  else if (ev->type == ev_mouse)
    g_string_append(s, "type = mouse");
  else if (ev->type == ev_joystick)
    g_string_append(s, "type = joystick");
  else if (ev->type == ev_mouse_movement)
    g_string_append(s, "type = mouse_movement");
  else if (ev->type == ev_joystick_movement)
    g_string_append(s, "type = joystick_movement");

  g_string_append(s, ", ");

  if (ev->jstype == ev_joystick_none)
    g_string_append(s, "jstype = none");
  if (ev->jstype == ev_joystick_axis)
    g_string_append(s, "jstype = axis");
  if (ev->jstype == ev_joystick_ball)
    g_string_append(s, "jstype = ball");
  if (ev->jstype == ev_joystick_hat)
    g_string_append(s, "jstype = hat");

  g_string_append(s, ", ");

  if (ev->pressed)
    g_string_append(s, "pressed = true");
  else
    g_string_append(s, "pressed = false");

  g_string_append(s, ", ");

  g_string_append_printf(s, "key = %s, ", I_GetKeyString(ev->key));
  g_string_append_printf(s, "value = %d, ", ev->value);
  g_string_append_printf(s, "xmove = %d, ", ev->xmove);
  g_string_append_printf(s, "ymove = %d, ", ev->ymove);
  g_string_append_printf(s, "wchar = %u", ev->wchar);

  g_string_append(s, "})>");

  lua_pushstring(L, s->str);

  g_string_free(s, true);

  return 1;
}

static int XI_InputEventReset(lua_State *L) {
  event_t *ev = (event_t *)luaL_checkudata(L, -1, "InputEvent");

  reset_input_event(ev);

  return 0;
}

static int XI_InputEventNew(lua_State *L) {
  event_t *ev = lua_newuserdata(L, sizeof(event_t));

  reset_input_event(ev);

  luaL_getmetatable(L, "InputEvent");
  lua_setmetatable(L, -2);

  return 1;
}

static int XI_InputEventIsKey(lua_State *L) {
  event_t *ev = (event_t *)luaL_checkudata(L, -1, "InputEvent");

  if (ev->type == ev_key)
    lua_pushboolean(L, true);
  else
    lua_pushboolean(L, false);

  return 1;
}

static int XI_InputEventIsMouse(lua_State *L) {
  event_t *ev = (event_t *)luaL_checkudata(L, -1, "InputEvent");

  if (ev->type == ev_mouse)
    lua_pushboolean(L, true);
  else
    lua_pushboolean(L, false);

  return 1;
}

static int XI_InputEventIsJoystick(lua_State *L) {
  event_t *ev = (event_t *)luaL_checkudata(L, -1, "InputEvent");

  if (ev->type == ev_joystick)
    lua_pushboolean(L, true);
  else
    lua_pushboolean(L, false);

  return 1;
}

static int XI_InputEventIsMouseMovement(lua_State *L) {
  event_t *ev = (event_t *)luaL_checkudata(L, -1, "InputEvent");

  if (ev->type == ev_mouse_movement)
    lua_pushboolean(L, true);
  else
    lua_pushboolean(L, false);

  return 1;
}

static int XI_InputEventIsJoystickMovement(lua_State *L) {
  event_t *ev = (event_t *)luaL_checkudata(L, -1, "InputEvent");

  if (ev->type == ev_joystick_movement)
    lua_pushboolean(L, true);
  else
    lua_pushboolean(L, false);

  return 1;
}

static int XI_InputEventIsJoystickAxis(lua_State *L) {
  event_t *ev = (event_t *)luaL_checkudata(L, -1, "InputEvent");

  if (ev->type == ev_joystick_movement && ev->jstype == ev_joystick_axis)
    lua_pushboolean(L, true);
  else
    lua_pushboolean(L, false);

  return 1;
}

static int XI_InputEventIsJoystickBall(lua_State *L) {
  event_t *ev = (event_t *)luaL_checkudata(L, -1, "InputEvent");

  if (ev->type == ev_joystick_movement && ev->jstype == ev_joystick_ball)
    lua_pushboolean(L, true);
  else
    lua_pushboolean(L, false);

  return 1;
}

static int XI_InputEventIsJoystickHat(lua_State *L) {
  event_t *ev = (event_t *)luaL_checkudata(L, -1, "InputEvent");

  if (ev->type == ev_joystick_movement && ev->jstype == ev_joystick_hat)
    lua_pushboolean(L, true);
  else
    lua_pushboolean(L, false);

  return 1;
}

static int XI_InputEventIsMovement(lua_State *L) {
  event_t *ev = (event_t *)luaL_checkudata(L, -1, "InputEvent");

  if (ev->type == ev_mouse_movement || ev->type == ev_joystick_movement)
    lua_pushboolean(L, true);
  else
    lua_pushboolean(L, false);

  return 1;
}

static int XI_InputEventIsPress(lua_State *L) {
  event_t *ev = (event_t *)luaL_checkudata(L, -1, "InputEvent");

  if (((ev->type == ev_key)   ||
       (ev->type == ev_mouse) ||
       (ev->type == ev_joystick)) && ev->pressed)
    lua_pushboolean(L, true);
  else
    lua_pushboolean(L, false);

  return 1;
}

static int XI_InputEventIsRelease(lua_State *L) {
  event_t *ev = (event_t *)luaL_checkudata(L, -1, "InputEvent");

  if (((ev->type == ev_key)   ||
       (ev->type == ev_mouse) ||
       (ev->type == ev_joystick)) && !ev->pressed)
    lua_pushboolean(L, true);
  else
    lua_pushboolean(L, false);

  return 1;
}

static int XI_InputEventGetKey(lua_State *L) {
  event_t *ev = (event_t *)luaL_checkudata(L, -1, "InputEvent");

  if (ev->type == ev_key)
    lua_pushnumber(L, ev->key);
  else
    lua_pushnumber(L, 0);

  return 1;
}

static int XI_InputEventGetKeyName(lua_State *L) {
  event_t *ev = (event_t *)luaL_checkudata(L, -1, "InputEvent");

  if (ev->type == ev_key)
    lua_pushstring(L, SDL_GetKeyName(ev->key));
  else
    lua_pushstring(L, "[unknown]");

  return 1;
}

static int XI_InputEventGetValue(lua_State *L) {
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

static int XI_InputEventGetXMove(lua_State *L) {
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

static int XI_InputEventGetYMove(lua_State *L) {
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

static gchar* convert_sdl_wchar(wchar_t wchar) {
  gchar *s = NULL;
  glong items_read;
  glong items_written;
  GError *error = NULL;

  s = g_utf16_to_utf8(
    (const gunichar2 *)&wchar, 1, &items_read, &items_written, &error
  );

  if (!s) {
    I_Error(
      "convert_sdl_wchar: Error during conversion: %s", error->message
    );
  }

  return s;
}

static bool is_printable(gchar *s) {
  gunichar c = g_utf8_get_char_validated(s, -1);

  if (c == ((gunichar)-1))
    return false;

  return g_unichar_isprint(c);
}

static int XI_InputEventGetChar(lua_State *L) {
  event_t *ev = (event_t *)luaL_checkudata(L, -1, "InputEvent");
  gchar *s = NULL;

  if (ev->type == ev_key && ev->pressed)
    s = convert_sdl_wchar(ev->wchar);

  if (s && is_printable(s))
    lua_pushstring(L, s);
  else
    lua_pushstring(L, NULL);

  if (s)
    g_free(s);

  return 1;
}

static int XI_InputEventIsKeyPress(lua_State *L) {
  event_t *ev = (event_t *)luaL_checkudata(L, 1, "InputEvent");
  int key = luaL_checkinteger(L, 2);

  if (ev->type == ev_key && ev->pressed && ev->key == key)
    lua_pushboolean(L, true);
  else
    lua_pushboolean(L, false);

  return 1;
}

static int XI_InputEventIsKeyRelease(lua_State *L) {
  event_t *ev = (event_t *)luaL_checkudata(L, 1, "InputEvent");
  int key = luaL_checkinteger(L, 2);

  if (ev->type == ev_key && (!ev->pressed) && ev->key == key)
    lua_pushboolean(L, true);
  else
    lua_pushboolean(L, false);

  return 1;
}

static int XI_PopulateEvent(lua_State *L) {
  event_t *event = (event_t *)luaL_checkudata(L, -1, "InputEvent");
  SDL_Event sdl_event;
  bool event_received = false;
  bool event_populated = false;

  event_received = SDL_PollEvent(&sdl_event);

  if (!event_received) {
    lua_pushboolean(L, event_received);
    lua_pushboolean(L, event_populated);

    return 2;
  }

  switch (sdl_event.type) {
    case SDL_KEYDOWN:
    case SDL_KEYUP:
      event->type = ev_key;
      event->key = sdl_event.key.keysym.sym;

      if (sdl_event.type == SDL_KEYDOWN) {
        event->pressed = true;
        event->wchar = sdl_event.key.keysym.unicode;
      }

      event_populated = true;
    break;
    case SDL_MOUSEBUTTONDOWN:
    case SDL_MOUSEBUTTONUP:
      if (I_MouseEnabled() && window_focused) {
        event->type = ev_mouse;
        event->pressed = sdl_event.type == SDL_MOUSEBUTTONDOWN;
        event->key = sdl_event.button.button;

        event_populated = true;
      }
    break;
    case SDL_MOUSEMOTION:
      if (I_MouseEnabled() && window_focused) {
        event->type = ev_mouse_movement;
        event->xmove = sdl_event.motion.xrel << 5;
        event->ymove = -(sdl_event.motion.yrel) << 5;

        event_populated = true;
      }
    break;
    case SDL_JOYAXISMOTION:
      if (I_JoystickEnabled()) {
        event->device_id = sdl_event.jaxis.which;
        event->type = ev_joystick_movement;
        event->jstype = ev_joystick_axis;
        event->key = sdl_event.jaxis.axis;
        event->value = sdl_event.jaxis.value;

        /*
         * CG: [XXX] Below modifications to event->value were in original
         *           joystick code, not totally sure why though.
         */

        event->value /= 3000;
        if (abs(event->value) < 10)
          event->value = 0;

        event_populated = true;
      }
    break;
    case SDL_JOYBALLMOTION:
      if (I_JoystickEnabled()) {
        event->device_id = sdl_event.jball.which;
        event->type = ev_joystick_movement;
        event->key = sdl_event.jball.ball;
        event->jstype = ev_joystick_ball;
        event->xmove = sdl_event.jball.xrel;
        event->ymove = sdl_event.jball.yrel;

        event_populated = true;
      }
    break;
    case SDL_JOYHATMOTION:
      if (I_JoystickEnabled()) {
        event->device_id = sdl_event.jball.which;
        event->type = ev_joystick_movement;
        event->key = sdl_event.jhat.hat;
        event->jstype = ev_joystick_hat;
        event->value = sdl_event.jhat.value;

        event_populated = true;
      }
    break;
    case SDL_JOYBUTTONDOWN:
    case SDL_JOYBUTTONUP:
      if (I_JoystickEnabled()) {
        event->device_id = sdl_event.jbutton.which;
        event->type = ev_joystick;
        event->key = sdl_event.jbutton.button;
        event->pressed = sdl_event.type == SDL_JOYBUTTONDOWN;

        event_populated = true;
      }
    break;
    case SDL_ACTIVEEVENT:
      I_VideoUpdateFocus();
    break;
    case SDL_VIDEORESIZE:
      /* ApplyWindowResize(Event); */
    break;
    case SDL_QUIT:
      S_StartSound(NULL, sfx_swtchn);
      M_QuitDOOM(0);
    break;
    default:
    break;
  }

  if (event_populated)
    I_InputUpdateKeyStates();

  lua_pushboolean(L, event_received);
  lua_pushboolean(L, event_populated);

  return 2;
}

static int XI_KeyStatesShiftIsDown(lua_State *L) {
  lua_pushboolean(L, key_states.shiftdown);

  return 1;
}

static int XI_KeyStatesCtrlIsDown(lua_State *L) {
  lua_pushboolean(L, key_states.ctrldown);

  return 1;
}

static int XI_KeyStatesAltIsDown(lua_State *L) {
  lua_pushboolean(L, key_states.altdown);

  return 1;
}

static int XI_KeyStatesMetaIsDown(lua_State *L) {
  lua_pushboolean(L, key_states.metadown);

  return 1;
}

static int XI_HandleEvent(lua_State *L) {
  event_t *ev = luaL_checkudata(L, -1, "InputEvent");
  bool event_handled = I_Responder(ev);

  lua_pushboolean(L, event_handled);

  return 1;
}

void XI_InputRegisterInterface(void) {
  /* CG: [TODO] Add functions to return names for mouse & joystick buttons */

  X_RegisterType("InputEvent", 22,
    "new",                  XI_InputEventNew,
    "reset",                XI_InputEventReset,
    "__tostring",           XI_InputEventToString,
    "is_key",               XI_InputEventIsKey,
    "is_mouse",             XI_InputEventIsMouse,
    "is_joystick",          XI_InputEventIsJoystick,
    "is_mouse_movement",    XI_InputEventIsMouseMovement,
    "is_joystick_movement", XI_InputEventIsJoystickMovement,
    "is_joystick_axis",     XI_InputEventIsJoystickAxis,
    "is_joystick_ball",     XI_InputEventIsJoystickBall,
    "is_joystick_hat",      XI_InputEventIsJoystickHat,
    "is_movement",          XI_InputEventIsMovement,
    "is_press",             XI_InputEventIsPress,
    "is_release",           XI_InputEventIsRelease,
    "get_key",              XI_InputEventGetKey,
    "get_key_name",         XI_InputEventGetKeyName,
    "get_value",            XI_InputEventGetValue,
    "get_xmove",            XI_InputEventGetXMove,
    "get_ymove",            XI_InputEventGetYMove,
    "get_char",             XI_InputEventGetChar,
    "is_key_press",         XI_InputEventIsKeyPress,
    "is_key_release",       XI_InputEventIsKeyRelease
  );

  X_RegisterObjects("KeyStates", 4,
    "shift_is_down", X_FUNCTION, XI_KeyStatesShiftIsDown,
    "ctrl_is_down",  X_FUNCTION, XI_KeyStatesCtrlIsDown,
    "alt_is_down",   X_FUNCTION, XI_KeyStatesAltIsDown,
    "meta_is_down",  X_FUNCTION, XI_KeyStatesMetaIsDown
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

  X_RegisterObjects("Mouse", 8,
    "LEFT",        X_INTEGER, SDL_BUTTON_LEFT,
    "MIDDLE",      X_INTEGER, SDL_BUTTON_MIDDLE,
    "RIGHT",       X_INTEGER, SDL_BUTTON_RIGHT,
    "WHEELUP",     X_INTEGER, SDL_BUTTON_WHEELUP,
    "WHEELDOWN",   X_INTEGER, SDL_BUTTON_WHEELDOWN,
    "X1",          X_INTEGER, SDL_BUTTON_X1,
    "X2",          X_INTEGER, SDL_BUTTON_X2,
    "max_buttons", X_INTEGER, MAX_MOUSEB
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

  X_RegisterObjects(NULL, 1,
    "populate_event", X_FUNCTION, XI_PopulateEvent
  );

  X_RegisterObjects("Input", 1,
    "handle_event", X_FUNCTION, XI_HandleEvent
  );
}

/* vi: set et ts=2 sw=2: */

