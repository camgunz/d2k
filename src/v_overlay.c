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

#include "doomdef.h"
#include "i_video.h"
#include "r_defs.h"
#include "r_patch.h"
#include "v_video.h"
#include "x_main.h"

#include "gl_opengl.h"
#include "gl_struct.h"

#include "i_vid8ingl.h"

typedef struct overlay_s {
  unsigned char *pixels;
  bool owns_pixels;
  GLuint tex_id;
  bool needs_resetting;
} overlay_t;

static overlay_t overlay;

void V_OverlayInit(void) {
  overlay.pixels          = NULL;
  overlay.owns_pixels     = false;
  overlay.tex_id          = 0;
  overlay.needs_resetting = false;
}

void V_OverlayBuildPixels(void) {
  if (overlay.pixels)
    I_Error("build_overlay_pixels: pixels already built");

  if (V_GetMode() == VID_MODEGL) {
    overlay.pixels = calloc(SCREENWIDTH * SCREENHEIGHT, sizeof(uint32_t));

    if (!overlay.pixels)
      I_Error("build_overlay_pixels: Allocating overlay pixels failed");

    overlay.owns_pixels = true;
  }
  else if (use_gl_surface) {
    overlay.pixels = vid_8ingl.screen->pixels;
    overlay.owns_pixels = false;
  }
  else {
    overlay.pixels = screens[0].data;
    overlay.owns_pixels = false;
  }
}

void V_OverlayDestroyPixels(void) {
  if (overlay.pixels && overlay.owns_pixels) {
    free(overlay.pixels);
    overlay.owns_pixels = false;
  }

  overlay.pixels = NULL;
}

void V_OverlayBuildTexture(void) {
  if (V_GetMode() == VID_MODEGL) {
    if (overlay.tex_id) {
      glDeleteTextures(1, &overlay.tex_id);
      overlay.tex_id = 0;
    }

    glGenTextures(1, &overlay.tex_id);
    glBindTexture(GL_TEXTURE_2D, overlay.tex_id);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GLEXT_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GLEXT_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    last_glTexID = &overlay.tex_id;
  }
}

void V_OverlayDestroyTexture(void) {
  if (overlay.tex_id != 0) {
    glDeleteTextures(1, &overlay.tex_id);
    overlay.tex_id = 0;
  }
}

bool V_OverlayNeedsResetting(void) {
  return overlay.needs_resetting;
}

void V_OverlaySetNeedsResetting(void) {
  overlay.needs_resetting = true;
}

void V_OverlayClearNeedsResetting(void) {
  overlay.needs_resetting = false;
}

GLuint V_OverlayGetTexID(void) {
  return overlay.tex_id;
}

GLuint* V_OverlayGetTexIDPointer(void) {
  return &overlay.tex_id;
}

unsigned char* V_OverlayGetPixels(void) {
  return overlay.pixels;
}

/* vi: set et ts=2 sw=2: */
