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
#include <cairo/cairo.h>

#ifdef GL_DOOM
#include "gl_opengl.h"
#include "gl_intern.h"
#endif

#include "doomdef.h"
#include "doomstat.h"
#include "i_video.h"
#include "v_overlay.h"
#include "v_video.h"
#include "x_main.h"

static int XV_IsEnabled(lua_State *L) {
  if (nodrawers || noblit)
    lua_pushboolean(L, false);
  else
    lua_pushboolean(L, true);

  return 1;
}

static int XV_GetScreenWidth(lua_State *L) {
  lua_pushnumber(L, SCREENWIDTH);

  return 1;
}

static int XV_GetScreenHeight(lua_State *L) {
  lua_pushnumber(L, SCREENHEIGHT);

  return 1;
}

static int XV_GetScreenStride(lua_State *L) {
  lua_pushnumber(L, I_GetScreenStride());

  return 1;
}

static int XV_UsingOpenGL(lua_State *L) {
  bool using_opengl;

#ifdef GL_DOOM
  using_opengl = V_GetMode() == VID_MODEGL;
#else
  using_opengl = false;
#endif

  lua_pushboolean(L, using_opengl);

  return 1;
}

static int XV_BuildOverlayPixels(lua_State *L) {
  V_OverlayBuildPixels();

  return 0;
}

static int XV_GetOverlayPixels(lua_State *L) {
  lua_pushlightuserdata(L, V_OverlayGetPixels());

  return 1;
}

static int XV_DestroyOverlayPixels(lua_State *L) {
  V_OverlayDestroyPixels();

  return 0;
}

static int XV_BuildOverlayTexture(lua_State *L) {
#ifdef GL_DOOM
  V_OverlayBuildTexture();
#endif

  return 0;
}

static int XV_DestroyOverlayTexture(lua_State *L) {
#ifdef GL_DOOM
  V_OverlayDestroyTexture();
#endif

  return 0;
}

static int XV_OverlayNeedsResetting(lua_State *L) {
  lua_pushboolean(L, V_OverlayNeedsResetting());

  return 1;
}

static int XV_ClearOverlayNeedsResetting(lua_State *L) {
  V_OverlayClearNeedsResetting();

  return 0;
}

static int XV_LockScreen(lua_State *L) {
  if (SDL_MUSTLOCK(screen)) {
    if (SDL_LockSurface(screen) < 0)
      I_Error("XV_LockScreen: %s\n", SDL_GetError());
  }

  return 0;
}

static int XV_UnlockScreen(lua_State *L) {
  if (SDL_MUSTLOCK(screen))
    SDL_UnlockSurface(screen);

  return 0;
}

void XV_RegisterInterface(void) {
  X_RegisterObjects("Video", 14,
    "is_enabled",                    X_FUNCTION, XV_IsEnabled,
    "get_screen_width",              X_FUNCTION, XV_GetScreenWidth,
    "get_screen_height",             X_FUNCTION, XV_GetScreenHeight,
    "get_screen_stride",             X_FUNCTION, XV_GetScreenStride,
    "using_opengl",                  X_FUNCTION, XV_UsingOpenGL,
    "build_overlay_pixels",          X_FUNCTION, XV_BuildOverlayPixels,
    "get_overlay_pixels",            X_FUNCTION, XV_GetOverlayPixels,
    "destroy_overlay_pixels",        X_FUNCTION, XV_DestroyOverlayPixels,
    "build_overlay_texture",         X_FUNCTION, XV_BuildOverlayTexture,
    "destroy_overlay_texture",       X_FUNCTION, XV_DestroyOverlayTexture,
    "overlay_needs_resetting",       X_FUNCTION, XV_OverlayNeedsResetting,
    "clear_overlay_needs_resetting", X_FUNCTION, XV_ClearOverlayNeedsResetting,
    "lock_screen",                   X_FUNCTION, XV_LockScreen,
    "unlock_screen",                 X_FUNCTION, XV_UnlockScreen
  );
}

/* vi: set et ts=2 sw=2: */

