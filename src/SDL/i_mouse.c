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
#include "doomstat.h"
#include "info.h"
#include "p_setup.h"
#include "p_mobj.h"
#include "p_user.h"
#include "d_event.h"
#include "g_game.h"
#include "r_defs.h"
#include "r_patch.h"
#include "r_data.h"
#include "v_video.h"
#include "hu_lib.h"
#include "e6y.h"
#include "i_mouse.h"
#include "i_video.h"
#include "m_argv.h"

extern int usemouse;

static SDL_Cursor* cursors[2] = {NULL, NULL};
static bool mouse_enabled;

static bool mouse_should_be_grabbed() {
  if (!window_focused)
    return false;

  if (desired_fullscreen)
    return true;

  if (!mouse_enabled)
    return false;

  if (walkcamera.type)
    return (demoplayback && gamestate == GS_LEVEL && !menuactive);

  /* CG [TODO] Add consoleactive here */
  if (menuactive || paused)
    return false;

  if (demoplayback)
    return false;

  if (gamestate != GS_LEVEL)
    return false;

  return true;
}

void I_MouseInit(void) {
  static Uint8 empty_cursor_data = 0;

  int nomouse_parm = M_CheckParm("-nomouse");

  // check if the user wants to use the mouse
  mouse_enabled = usemouse && !nomouse_parm;

  // Save the default cursor so it can be recalled later
  cursors[0] = SDL_GetCursor();
  // Create an empty cursor
  cursors[1] = SDL_CreateCursor(
    &empty_cursor_data, &empty_cursor_data, 8, 1, 0, 0
  );

  if (mouse_enabled)
    MouseAccelChanging();

  atexit(I_MouseShutdown);
}

void I_MouseShutdown(void) {
  SDL_FreeCursor(cursors[1]);
  I_MouseDeactivate();
}

bool I_MouseEnabled(void) {
  return mouse_enabled;
}

void I_MouseActivate(void) {
  SDL_WM_GrabInput(SDL_GRAB_ON);
  SDL_ShowCursor(SDL_DISABLE);
}

void I_MouseDeactivate(void) {
  SDL_WM_GrabInput(SDL_GRAB_OFF);
  SDL_ShowCursor(SDL_ENABLE);
  SDL_WarpMouse(
    (unsigned short)(REAL_SCREENWIDTH  / 2),
    (unsigned short)(REAL_SCREENHEIGHT / 2)
  );
}

void I_MouseReset(void) {
  static int mouse_currently_grabbed = true;

  if (!usemouse)
    return;

  if (!mouse_should_be_grabbed()) {
    mouse_currently_grabbed = false;
    return;
  }

  if (!mouse_currently_grabbed && !desired_fullscreen)
    mouse_currently_grabbed = true;
}

void I_MouseUpdateGrab(void) {
  static bool currently_grabbed = false;
  bool grab;

  grab = mouse_should_be_grabbed();

  if (grab && !currently_grabbed)
    I_MouseActivate();

  if (!grab && currently_grabbed)
    I_MouseDeactivate();

  currently_grabbed = grab;
}

/* vi: set et ts=2 sw=2: */

