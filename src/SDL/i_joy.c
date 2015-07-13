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

#include "doomdef.h"
#include "m_argv.h"
#include "d_event.h"
#include "d_main.h"
#include "i_joy.h"

int joyleft;
int joyright;
int joyup;
int joydown;

int usejoystick;

#ifdef HAVE_SDL_JOYSTICKGETAXIS
static SDL_Joystick *joystick;
#endif

#ifdef HAVE_SDL_JOYSTICKGETAXIS
static void I_EndJoystick(void) {
  D_Msg(MSG_DEBUG, "I_EndJoystick : closing joystick\n");
}
#endif

bool I_JoystickEnabled(void) {
#ifdef HAVE_SDL_JOYSTICKGETAXIS
  return usejoystick && joystick;
#else
  return false;
#endif
}

void I_PollJoystick(void) {
#if 0
#ifdef HAVE_SDL_JOYSTICKGETAXIS
  event_t ev;
  Sint16 axis_value;

  if (!usejoystick || (!joystick))
      return;

  ev.type = ev_joystick;
  ev.key =   (SDL_JoystickGetButton(joystick, 0) << 0) |
             (SDL_JoystickGetButton(joystick, 1) << 1) |
             (SDL_JoystickGetButton(joystick, 2) << 2) |
             (SDL_JoystickGetButton(joystick, 3) << 3) |
             (SDL_JoystickGetButton(joystick, 4) << 4) |
             (SDL_JoystickGetButton(joystick, 5) << 5) |
             (SDL_JoystickGetButton(joystick, 6) << 6) |
             (SDL_JoystickGetButton(joystick, 7) << 7);
  axis_value = SDL_JoystickGetAxis(joystick, 0) / 3000;
  if (abs(axis_value) < 10)
    axis_value = 0;
  ev.data2 = axis_value;
  axis_value = SDL_JoystickGetAxis(joystick, 1) / 3000;
  if (abs(axis_value) < 10)
    axis_value = 0;
  ev.data3 = axis_value;

  D_PostEvent(&ev);
#endif
#endif
}

void I_InitJoystick(void) {
#ifdef HAVE_SDL_JOYSTICKGETAXIS
  const char* fname = "I_InitJoystick : ";
  int num_joysticks;

  if (!usejoystick)
    return;

  SDL_InitSubSystem(SDL_INIT_JOYSTICK);

  num_joysticks = SDL_NumJoysticks();
  if (M_CheckParm("-nojoy") ||
      (usejoystick > num_joysticks) | (usejoystick < 0)) {
    if ((usejoystick > num_joysticks) || (usejoystick < 0))
      D_Msg(MSG_WARN, "%sinvalid joystick %d\n", fname, usejoystick);
    else
      D_Msg(MSG_INFO, "%suser disabled\n", fname);

    return;
  }

  joystick = SDL_JoystickOpen(usejoystick - 1);

  if (!joystick) {
    D_Msg(MSG_ERROR, "%serror opening joystick %s\n",
      fname, SDL_JoystickName(usejoystick - 1)
    );
  }
  else {
    atexit(I_EndJoystick);
    D_Msg(MSG_INFO, "%sopened %s\n",
      fname, SDL_JoystickName(usejoystick - 1)
    );
    joyup    =  32767;
    joydown  = -32768;
    joyright =  32767;
    joyleft  = -32768;
  }
#endif
}

/* vi: set et ts=2 sw=2: */

