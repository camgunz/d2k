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
#include "g_keys.h"
#include "i_input.h"
#include "i_joy.h"
#include "i_main.h"
#include "i_mouse.h"
#include "v_video.h"
#include "x_main.h"

extern bool doSkip;

key_states_t key_states;

void I_InputInit(void) {
  SDL_PumpEvents();

  I_MouseInit();
  I_InitJoystick();
}

void I_InputUpdateKeyStates(void) {
  SDLMod mod_keys = SDL_GetModState();

  key_states.shiftdown  = false;
  key_states.ctrldown   = false;
  key_states.altdown    = false;
  key_states.metadown   = false;

  if (mod_keys & KMOD_SHIFT)
    key_states.shiftdown = true;
  if (mod_keys & KMOD_CTRL)
    key_states.ctrldown = true;
  if (mod_keys & KMOD_ALT)
    key_states.altdown = true;
  if (mod_keys & KMOD_META)
    key_states.metadown = true;

  /*
   * CG: [TODO]
   *
   * `shiftdown`, etc. ought to mean "any shift is down", whereas
   * `leftshiftdown` ought to mean the left shift key is down.  I'm not
   * totally sure how this translates to an interface (how does a user
   * specify "I mean any ctrl key, not just the left/right one"?), but right
   * now there isn't even an option to differentiate.
   *
   */
}

void I_InputHandle(void) {
  x_engine_t *xe = X_GetState();

  if (!X_Call(xe, "input_event_dispatcher", "dispatch_events", 0, 0)) {
    I_Error("I_InputHandle: Error handling input (%s)", X_GetError(xe));
  }

  I_MouseReset();
}

bool I_Responder(event_t *ev) {
  if (ev->type == ev_key && ev->pressed && key_states.altdown) {
#ifdef MACOSX
    // Switch windowed<->fullscreen if pressed <Command-F>
    if (ev->key == SDLK_f) {
      V_ToggleFullscreen();
      return true;
    }
#else
    // Prevent executing action on Alt-Tab
    if (ev->key == SDLK_TAB)
      return false;

    // Switch windowed<->fullscreen if pressed Alt-Enter
    if (ev->key == SDLK_RETURN) {
      V_ToggleFullscreen();
      return true;
    }

    // Immediately exit on Alt+F4 ("Boss Key")
    if (ev->key == SDLK_F4) {
      I_SafeExit(0);
      return true;
    }
#endif
  }

  // Allow only sensible keys during skipping
  if (doSkip) {
    if (ev->type == ev_key) {
      // Immediate exit if key_quit is pressed in skip mode
      if (ev->key == key_quit) {
        I_SafeExit(0);
        return true;
      }

      // key_use is used for seeing the current frame
      if (ev->key != key_use && ev->key != key_demo_skip)
        return false;
    }
  }

  return false;
}

const char* I_GetKeyString(int keycode) {
  return SDL_GetKeyName(keycode);
}

/* vi: set et ts=2 sw=2: */

