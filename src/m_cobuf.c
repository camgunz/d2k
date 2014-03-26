/* Emacs style mode select   -*- C++ -*-
 *-----------------------------------------------------------------------------
 *
 *
 *  PrBoom: a Doom port merged with LxDoom and LSDLDoom
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 1999 by
 *  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
 *  Copyright (C) 1999-2000 by
 *  Jess Haas, Nicolas Kalkhof, Colin Phipps, Florian Schulze
 *  Copyright 2005, 2006 by
 *  Florian Schulze, Colin Phipps, Neil Stevens, Andrey Budko
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
 *  A buffer of objects that uses copies instead of pointers
 *
 *-----------------------------------------------------------------------------
 */

#include "z_zone.h"
#include "doomtype.h"
#include "l_printf.h"
#include "m_cobuf.h"

void M_CObjBufferInit(cobjbuf_t **cobuf, size_t obj_size) {
  (*cobuf) = calloc(1, sizeof(cobjbuf_t));

  if ((*cobuf) == NULL)
    I_Error("M_CObjBufferInit: Allocating buffer failed");

  (*cobuf)->obj_size = obj_size;
}

void M_CObjBufferInitWithCapacity(cobjbuf_t **cobuf, size_t obj_size,
                                                     int capacity) {
  M_CObjBufferInit(cobuf, obj_size);
  M_CObjBufferEnsureCapacity(capacity);
}

void M_CObjBufferAppend(cobjbuf_t *cobuf, void *obj) {
  int index = cobuf->capacity;

  M_CObjBufferEnsureCapacity(cobuf->capacity + 1);
  M_CObjBufferInsert(cobuf, index, obj);
}

void M_CObjBufferInsert(cobjbuf_t *cobuf, int index, void *obj) {
  if (index >= cobuf->capacity) {
    I_Error("M_CObjBufferInsert: Insertion index %d out of bounds (%d)",
      index, cobuf->capacity
    );
  }

  memcpy(cobuf->nodes[index].obj, obj, cobuf->obj_size);
  cobuf->nodes[index].used = true;
}

int M_CObjBufferInsertAtFirstFreeSlot(cobjbuf_t *cobuf, void *obj) {
  for (int i = 0; i < cobuf->capacity; i++) {
    cobjnode_t *node = cobuf->nodes[i];

    if (!node->used) {
      memcpy(cobuf->nodes[i].obj, obj, cobuf->obj_size);
      return i;
    }
  }

  return -1;
}

int M_CObjBufferInsertAtFirstFreeSlotOrAppend(cobjbuf_t *cobuf, void *obj) {
  int slot = M_CObjBufferInsertAtFirstFreeSlot(cobuf, obj);

  if (slot == -1) {
    M_CObjBufferAppend(cobuf, obj);
    slot = cobuf->capacity - 1;
  }

  return slot;
}

void M_CObjBufferConsolidate(cobjbuf_t *cobuf) {
  cobjnode_t *dst = cobuf->nodes;
  cobjnode_t *src = cobuf->nodes;

  while (true) {
    while (dst->used && dst - cobuf->nodes < cobuf->capacity)
      dst++;

    if (dst - cobuf->nodes >= cobuf->capacity)
      return;

    if (src < dst)
      src = dst + 1;

    while ((!src->used) && src - cobuf->nodes < cobuf->capacity)
      src++;

    if (src - cobuf->nodes >= cobuf->capacity)
      return;

    *dst = *src;
    *src = NULL;
  }
}

void M_CObjBufferRemove(cobjbuf_t *cobuf, int index) {
  cobuf->nodes[index]->used = false;
  memset(cobuf->nodes[index]->obj, 0, cobuf->obj_size);
}

void M_CObjBufferEnsureCapacity(cobjbuf_t *cobuf, int capacity) {
  if (cobuf->capacity < capacity) {
    int old_capacity = cobuf->capacity;

    cobuf->capacity = capacity;
    cobuf->nodes = realloc(
      cobuf->nodes, cobuf->capacity * sizeof(cobjnode_t *)
    );

    if (cobuf->nodes == NULL)
      I_Error("M_CObjBufferEnsureCapacity: Allocating buffer nodes failed");

    for (int i = old_capacity; i < cobuf->capacity; i++) {
      cobuf->nodes[i] = calloc(1, sizeof(cobjnode_t));
      cobuf->nodes[i]->used = false;
      cobuf->nodes[i]->obj = calloc(1, cobuf->obj_size);
    }
  }
}

int M_CObjBufferGetObjectCount(cobjbuf_t *cobuf) {
  int count = 0;

  for (int i = 0; i < cobuf->capacity; i++) {
    if (cobuf->nodes[i]->used) {
      count++;
    }
  }

  return count;
}

void M_CObjBufferClear(cobjbuf_t *cobuf) {
  for (int i = 0; i < cobuf->capacity; i++)
    M_CObjBufferRemove(cobuf, i);
}

void M_CObjBufferFreeEntriesAndClear(cobjbuf_t *cobuf) {
  for (int i = 0; i < cobuf->capacity; i++) {
    free(cobuf->nodes[i]->obj);
    free(cobuf->nodes[i]);
  }

  cobuf->capacity = 0;
}

void M_CObjBufferFree(cobjbuf_t *cobuf) {
  M_CobjBufferFreeEntriesAndClear(cobuf);
  free(cobuf->nodes);
}

/* vi: set et ts=2 sw=2: */

