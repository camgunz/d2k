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
 *  A buffer of objects that uses copies instead of pointers, more efficient
 *  than m_obuf when allocations can be avoided or have already been performed.
 *
 *-----------------------------------------------------------------------------
 */

#include "z_zone.h"

#include "doomtype.h"
#include "lprintf.h"
#include "m_cbuf.h"

void M_CBufInit(cbuf_t *cbuf, size_t obj_size) {
  cbuf->capacity = 0;
  cbuf->obj_size = obj_size;
  cbuf->nodes    = NULL;
}

void M_CBufInitWithCapacity(cbuf_t *cbuf, size_t obj_size, int capacity) {
  M_CBufInit(cbuf, obj_size);
  M_CBufEnsureCapacity(cbuf, capacity);
}

dboolean M_CBufIsValidIndex(cbuf_t *cbuf, int index) {
  if (index >= 0 && index < cbuf->capacity && cbuf->nodes[index].used)
    return true;

  return false;
}

int M_CBufGetObjectCount(cbuf_t *cbuf) {
  int count = 0;

  for (int i = 0; i < cbuf->capacity; i++) {
    if (cbuf->nodes[i].used) {
      count++;
    }
  }

  return count;
}

void M_CBufEnsureCapacity(cbuf_t *cbuf, int capacity) {
  if (cbuf->capacity < capacity) {
    int old_capacity = cbuf->capacity;

    cbuf->capacity = capacity;
    cbuf->nodes = realloc(cbuf->nodes, cbuf->capacity * sizeof(cbufnode_t));

    if (cbuf->nodes == NULL)
      I_Error("M_CBufEnsureCapacity: Allocating buffer nodes failed");

    for (int i = old_capacity; i < cbuf->capacity; i++) {
      cbuf->nodes[i].used = false;
      cbuf->nodes[i].obj = calloc(1, cbuf->obj_size);

      if (cbuf->nodes[i].obj == NULL)
        I_Error("M_CBufEnsureCapacity: Allocating buffer node's obj failed");
    }
  }
}

void M_CBufAppend(cbuf_t *cbuf, void *obj) {
  int index = cbuf->capacity;

  M_CBufEnsureCapacity(cbuf, cbuf->capacity + 1);
  M_CBufInsert(cbuf, index, obj);
}

void M_CBufInsert(cbuf_t *cbuf, int index, void *obj) {
  if (index >= cbuf->capacity) {
    I_Error("M_CBufInsert: Insertion index %d out of bounds (>= %d)",
      index, cbuf->capacity
    );
  }

  memcpy(cbuf->nodes[index].obj, obj, cbuf->obj_size);
  cbuf->nodes[index].used = true;
}

int M_CBufInsertAtFirstFreeSlot(cbuf_t *cbuf, void *obj) {
  for (int i = 0; i < cbuf->capacity; i++) {
    if (!cbuf->nodes[i].used) {
      M_CBufInsert(cbuf, i, obj);
      return i;
    }
  }

  return -1;
}

int M_CBufInsertAtFirstFreeSlotOrAppend(cbuf_t *cbuf, void *obj) {
  int slot = M_CBufInsertAtFirstFreeSlot(cbuf, obj);

  if (slot != -1)
    return slot;

  M_CBufAppend(cbuf, obj);
  return cbuf->capacity - 1;
}

dboolean M_CBufIter(cbuf_t *cbuf, int *index, void **obj) {
  for (int i = (*index) + 1; i < cbuf->capacity; i++) {
    if (cbuf->nodes[i].used) {
      *index = i;
      *obj = cbuf->nodes[i].obj;
      return true;
    }
  }

  *index = -1;
  return false;
}

void* M_CBufGet(cbuf_t *cbuf, int index) {
  if (!M_CBufIsValidIndex(cbuf, index))
    return NULL;

  return cbuf->nodes[index].obj;
}

void* M_CBufGetFirstFreeSlot(cbuf_t *cbuf) {
  for (int i = 0; i < cbuf->capacity; i++) {
    if (!cbuf->nodes[i].used) {
      cbuf->nodes[i].used = true;
      return cbuf->nodes[i].obj;
    }
  }

  return NULL;
}

void* M_CBufGetNewSlot(cbuf_t *cbuf) {
  int index = cbuf->capacity;

  M_CBufEnsureCapacity(cbuf, cbuf->capacity + 1);
  cbuf->nodes[index].used = true;
  return cbuf->nodes[index].obj;
}

void* M_CBufGetFirstFreeOrNewSlot(cbuf_t *cbuf) {
  void *slot = M_CBufGetFirstFreeSlot(cbuf);

  if (slot == NULL)
    slot = M_CBufGetNewSlot(cbuf);

  return slot;
}

void M_CBufRemove(cbuf_t *cbuf, int index) {
  cbuf->nodes[index].used = false;
  memset(cbuf->nodes[index].obj, 0, cbuf->obj_size);
}

void M_CBufConsolidate(cbuf_t *cbuf) {
  int d = 0;
  int s = 0;
  void *tmp = NULL;

  while (true) {
    while (d < cbuf->capacity && cbuf->nodes[d].used)
      d++;

    if (d >= cbuf->capacity)
      return;

    if (s < d)
      s = d + 1;

    while (s < cbuf->capacity && (!cbuf->nodes[s].used))
      s++;

    if (s >= cbuf->capacity)
      return;

    tmp = cbuf->nodes[d].obj;
    cbuf->nodes[d].obj = cbuf->nodes[s].obj;
    cbuf->nodes[s].obj = tmp;
    cbuf->nodes[d].used = true;
    cbuf->nodes[s].used = false;
  }
}

void M_CBufClear(cbuf_t *cbuf) {
  for (int i = 0; i < cbuf->capacity; i++)
    M_CBufRemove(cbuf, i);
}

void M_CBufFree(cbuf_t *cbuf) {
  for (int i = 0; i < cbuf->capacity; i++)
    free(cbuf->nodes[i].obj);

  free(cbuf->nodes);
  M_CBufInit(cbuf, 0);
}

/* vi: set et ts=2 sw=2: */

