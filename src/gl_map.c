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
#ifdef HAVE_LIBSDL_IMAGE
#include <SDL_image.h>
#endif /* ifdef HAVE_LIBSDL_IMAGE */

#include "v_video.h"
#include "w_wad.h"
#include "m_misc.h"
#include "am_map.h"

#include "gl_opengl.h"
#include "gl_intern.h"
#include "gl_struct.h"

am_icon_t am_icons[am_icon_count + 1] = {
  { -1, "M_SHADOW" },
  { -1, "M_ARROW" },
  { -1, "M_NORMAL" },
  { -1, "M_HEALTH" },
  { -1, "M_ARMOUR" },
  { -1, "M_AMMO" },
  { -1, "M_KEY" },
  { -1, "M_POWER" },
  { -1, "M_WEAP" },
  { -1, "M_ARROW" },
  { -1, "M_ARROW" },
  { -1, "M_ARROW" },
  { -1, "M_MARK" },
  { -1, "M_NORMAL" },
  { -1, NULL },
};

typedef struct map_nice_thing_s {
  vbo_xy_uv_rgba_t v[4];
} PACKEDATTR map_nice_thing_t;

static GPtrArray *map_things = NULL;

void gld_InitMapPics(void) {
  int i = 0;

  if (!map_things) {
    map_things = g_ptr_array_sized_new(am_icon_count);
  }

  for (i = 0; i < am_icon_count; i++) {
    GArray *things = g_array_new(false, false, sizeof(map_nice_thing_t));

    g_ptr_array_add(map_things, things);
  }

  while (am_icons[i].name) {
    int lump = (W_CheckNumForName)(am_icons[i].name, ns_prboom);

    am_icons[i].lumpnum = lump;

    if (lump != -1) {
      SDL_Surface *surf = NULL;
#ifdef HAVE_LIBSDL_IMAGE
      SDL_Surface *surf_raw;

      surf_raw = IMG_Load_RW(
        SDL_RWFromConstMem(W_CacheLumpNum(lump), W_LumpLength(lump)), true
        );

      surf = SDL_ConvertSurface(surf_raw, &RGBAFormat, SDL_SRCALPHA);
      SDL_FreeSurface(surf_raw);
#endif /* ifdef HAVE_LIBSDL_IMAGE */

      W_UnlockLumpNum(lump);

      if (surf) {
        glGenTextures(1, &am_icons[i].tex_id);
        glBindTexture(GL_TEXTURE_2D, am_icons[i].tex_id);

        if (gl_arb_texture_non_power_of_two) {
          glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
          glTexImage2D(
            GL_TEXTURE_2D,
            0,
            gl_tex_format,
            surf->w,
            surf->h,
            0,
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            surf->pixels
          );
        }
        else if (gl_arb_framebuffer_object) {
          gld_BuildMipmaps(surf->w, surf->h, surf->pixels, GL_CLAMP);
        }
        SDL_FreeSurface(surf);
      }
    }

    i++;
  }
}

void gld_AddNiceThing(int type, float         x,
                                float         y,
                                float         radius,
                                float         angle,
                                unsigned char r,
                                unsigned char g,
                                unsigned char b,
                                unsigned char a) {
  GArray *things = g_ptr_array_index(map_things, type);
  map_nice_thing_t thing;
  float sina_r = (float)sin(angle) * radius;
  float cosa_r = (float)cos(angle) * radius;

  thing.v[0].x = x + sina_r + cosa_r;
  thing.v[0].y = y - cosa_r + sina_r;
  thing.v[0].u = 1.0f;
  thing.v[0].v = 0.0f;
  thing.v[0].r = r;
  thing.v[0].g = g;
  thing.v[0].b = b;
  thing.v[0].a = a;
  thing.v[1].x = x + sina_r - cosa_r;
  thing.v[1].y = y - cosa_r - sina_r;
  thing.v[1].u = 0.0f;
  thing.v[1].v = 0.0f;
  thing.v[1].r = r;
  thing.v[1].g = g;
  thing.v[1].b = b;
  thing.v[1].a = a;
  thing.v[2].x = x - sina_r - cosa_r;
  thing.v[2].y = y + cosa_r - sina_r;
  thing.v[2].u = 0.0f;
  thing.v[2].v = 1.0f;
  thing.v[2].r = r;
  thing.v[2].g = g;
  thing.v[2].b = b;
  thing.v[2].a = a;
  thing.v[3].x = x - sina_r + cosa_r;
  thing.v[3].y = y + cosa_r + sina_r;
  thing.v[3].u = 1.0f;
  thing.v[3].v = 1.0f;
  thing.v[3].r = r;
  thing.v[3].g = g;
  thing.v[3].b = b;
  thing.v[3].a = a;

  g_array_append_val(things, thing);
}

void gld_DrawNiceThings(int fx, int fy, int fw, int fh) {
  int i;

  glScissor(fx, SCREENHEIGHT - (fy + fh), fw, fh);
  glEnable(GL_SCISSOR_TEST);

  glDisable(GL_ALPHA_TEST);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  gld_EnableTexture2D(GL_TEXTURE0_ARB, true);

#if defined(USE_VERTEX_ARRAYS) || defined(USE_VBO)

  // activate vertex array, texture coord array and color arrays
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  glEnableClientState(GL_COLOR_ARRAY);
#endif /* if defined(USE_VERTEX_ARRAYS) || defined(USE_VBO) */

  for (i = 0; i < am_icon_count; i++) {
    GArray *things = g_ptr_array_index(map_things, i);

    if (things->len == 0) {
      continue;
    }

    glBindTexture(GL_TEXTURE_2D, am_icons[i].tex_id);

#if defined(USE_VERTEX_ARRAYS) || defined(USE_VBO)
    map_nice_thing_t *thing = (map_nice_thing_t *)things->data;

    // activate and specify pointers to arrays
    glVertexPointer(2, GL_FLOAT, sizeof(thing->v[0]), &thing->v[0].x);
    glTexCoordPointer(2, GL_FLOAT, sizeof(thing->v[0]), &thing->v[0].u);
    glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(thing->v[0]), &thing->v[0].r);

    glDrawArrays(GL_QUADS, 0, things->len * 4);
#else  /* if defined(USE_VERTEX_ARRAYS) || defined(USE_VBO) */
    for (j = 0; j < things->len; j++) {
      map_nice_thing_t *thing = g_array_index(things, j);

      glColor4ubv(&thing->v[0].r);

      glBegin(GL_TRIANGLE_FAN);
      glTexCoord2f(thing->v[0].u, thing->v[0].v);
      glVertex2f(thing->v[0].x, thing->v[0].y);
      glTexCoord2f(thing->v[1].u, thing->v[1].v);
      glVertex2f(thing->v[1].x, thing->v[1].y);
      glTexCoord2f(thing->v[2].u, thing->v[2].v);
      glVertex2f(thing->v[2].x, thing->v[2].y);
      glTexCoord2f(thing->v[3].u, thing->v[3].v);
      glVertex2f(thing->v[3].x, thing->v[3].y);
      glEnd();
    }
#endif /* if defined(USE_VERTEX_ARRAYS) || defined(USE_VBO) */
  }

#if defined(USE_VERTEX_ARRAYS) || defined(USE_VBO)

  // deactivate vertex array, texture coord array and color arrays
  glDisableClientState(GL_VERTEX_ARRAY);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  glDisableClientState(GL_COLOR_ARRAY);
#endif /* if defined(USE_VERTEX_ARRAYS) || defined(USE_VBO) */

  gld_ResetLastTexture();
  glDisable(GL_SCISSOR_TEST);
}

void gld_ClearNiceThings(void) {
  for (int type = 0; type < am_icon_count; type++) {
    GArray *things = g_ptr_array_index(map_things, type);

    g_array_remove_range(things, 0, things->len);
  }
}

void gld_DrawMapLines(void) {
#if defined(USE_VERTEX_ARRAYS) || defined(USE_VBO)
  if (map_lines->len > 0) {
    map_point_t *point = (map_point_t *)map_lines->data;

    gld_EnableTexture2D(GL_TEXTURE0_ARB, false);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    glVertexPointer(2, GL_FLOAT, sizeof(point[0]), &point->x);
    glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(point[0]), &point->r);

    glDrawArrays(GL_LINES, 0, map_lines->len * 2);

    gld_EnableTexture2D(GL_TEXTURE0_ARB, true);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
  }
#endif /* if defined(USE_VERTEX_ARRAYS) || defined(USE_VBO) */
}

/* vi: set et ts=2 sw=2: */
