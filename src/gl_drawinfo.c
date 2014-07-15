/* Emacs style mode select   -*- C++ -*- 
 *-----------------------------------------------------------------------------
 *
 *
 *  PrBoom a Doom port merged with LxDoom and LSDLDoom
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 1999 by
 *  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
 *  Copyright (C) 1999-2000 by
 *  Jess Haas, Nicolas Kalkhof, Colin Phipps, Florian Schulze
 *  
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 
 *  02111-1307, USA.
 *
 * DESCRIPTION:
 *
 *  memory manager for GL data
 *---------------------------------------------------------------------
 */

#include "z_zone.h"

#include "doomtype.h"
#include "gl_opengl.h"
#include "gl_intern.h"
#include "lprintf.h"

GLDrawInfo gld_drawinfo;

//
// gld_FreeDrawInfo
//
void gld_FreeDrawInfo(void)
{
  int i;

  for (i = 0; i < gld_drawinfo.maxsize; i++)
  {
    if (gld_drawinfo.data[i].data)
    {
      free(gld_drawinfo.data[i].data);
      gld_drawinfo.data[i].data = 0;
    }
  }
  free(gld_drawinfo.data);
  gld_drawinfo.data = 0;

  for (i = 0; i < GLDIT_TYPES; i++)
  {
    if (gld_drawinfo.items[i])
    {
      free(gld_drawinfo.items[i]);
      gld_drawinfo.items[i] = 0;
    }
  }

  memset(&gld_drawinfo, 0, sizeof(GLDrawInfo));
}

//
// gld_ResetDrawInfo
//
// Should be used between frames (in gld_StartDrawScene)
//
void gld_ResetDrawInfo(void) {
  int i;

  for (i = 0; i < gld_drawinfo.maxsize; i++)
    gld_drawinfo.data[i].size = 0;

  gld_drawinfo.size = 0;

  for (i = 0; i < GLDIT_TYPES; i++)
    gld_drawinfo.num_items[i] = 0;
}

//
// gld_AddDrawRange
//
static void gld_AddDrawRange(int size) {
  gld_drawinfo.maxsize++;
  gld_drawinfo.data = realloc(
    gld_drawinfo.data, gld_drawinfo.maxsize * sizeof(gld_drawinfo.data[0])
  );

  gld_drawinfo.data[gld_drawinfo.size].maxsize = size;
  gld_drawinfo.data[gld_drawinfo.size].data = malloc(size);
  gld_drawinfo.data[gld_drawinfo.size].size = 0;
}

//
// gld_AddDrawItem
//
static int itemsizes[GLDIT_TYPES] = {
  0,
  (sizeof(GLWall)      + 7) & ~7,
  (sizeof(GLWall)      + 7) & ~7,
  (sizeof(GLWall)      + 7) & ~7,
  (sizeof(GLWall)      + 7) & ~7,
  (sizeof(GLWall)      + 7) & ~7,
  (sizeof(GLWall)      + 7) & ~7,
  (sizeof(GLWall)      + 7) & ~7,
  (sizeof(GLFlat)      + 7) & ~7,
  (sizeof(GLFlat)      + 7) & ~7,
  (sizeof(GLFlat)      + 7) & ~7,
  (sizeof(GLFlat)      + 7) & ~7,
  (sizeof(GLSprite)    + 7) & ~7,
  (sizeof(GLSprite)    + 7) & ~7,
  (sizeof(GLSprite)    + 7) & ~7,
  (sizeof(GLShadow)    + 7) & ~7,
  (sizeof(GLHealthBar) + 7) & ~7,
};

void gld_AddDrawItem(GLDrawItemType dit, void *itemdata) {
  int itemsize = 0;
  byte *item_p = NULL;

  itemsize = itemsizes[dit];
  if (itemsize == 0)
    I_Error("gld_AddDrawItem: unknown GLDrawItemType %d", dit);

  if (gld_drawinfo.maxsize == 0)
    gld_AddDrawRange(MAX(64 * 1024, itemsize));

  if (gld_drawinfo.data[gld_drawinfo.size].size + itemsize >=
      gld_drawinfo.data[gld_drawinfo.size].maxsize) {
    gld_drawinfo.size++;

    if (gld_drawinfo.size >= gld_drawinfo.maxsize)
      gld_AddDrawRange(MAX(64 * 1024, itemsize));
  }

  item_p = gld_drawinfo.data[gld_drawinfo.size].data +
           gld_drawinfo.data[gld_drawinfo.size].size;

  memcpy(item_p, itemdata, itemsize);

  gld_drawinfo.data[gld_drawinfo.size].size += itemsize;

  if (gld_drawinfo.num_items[dit] >= gld_drawinfo.max_items[dit]) {
    gld_drawinfo.max_items[dit] += 64;
    gld_drawinfo.items[dit] = realloc(
      gld_drawinfo.items[dit],
      gld_drawinfo.max_items[dit] * sizeof(gld_drawinfo.items[0][0]));
  }

  gld_drawinfo.items[dit][gld_drawinfo.num_items[dit]].item.item = item_p;
  gld_drawinfo.num_items[dit]++;
}

/* vi: set et ts=2 sw=2: */

