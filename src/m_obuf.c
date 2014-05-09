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
 *  A buffer of objects, more efficient than m_cbuf when an object must be
 *  allocated (avoids copying and then deallocating the original object) or
 *  when the object is large enough that a copy takes more time than an
 *  allocation.
 *
 *-----------------------------------------------------------------------------
 */

#include "z_zone.h"

#include "doomtype.h"
#include "lprintf.h"
#include "m_obuf.h"

void M_OBufInit(obuf_t *obuf) {
  obuf->capacity = 0;
  obuf->objects  = NULL;
}

void M_OBufInitWithCapacity(obuf_t *obuf, int capacity) {
  M_OBufInit(obuf);
  M_OBufEnsureCapacity(obuf, capacity);
}

dboolean M_OBufIsValidIndex(obuf_t *obuf, int index) {
  if (index >= 0 && index < obuf->capacity && obuf->objects[index] != NULL)
    return true;

  return false;
}

int M_OBufGetObjectCount(obuf_t *obuf) {
  int count = 0;

  for (int i = 0; i < obuf->capacity; i++) {
    if (obuf->objects[i] != NULL) {
      count++;
    }
  }

  return count;
}

void M_OBufEnsureCapacity(obuf_t *obuf, int capacity) {
  if (obuf->capacity < capacity) {
    int old_capacity = obuf->capacity;

    obuf->capacity = capacity;
    obuf->objects = realloc(obuf->objects, obuf->capacity * sizeof(void *));

    if (obuf->objects == NULL)
      I_Error("M_OBufEnsureCapacity: Allocating buffer objects failed");

    memset(obuf->objects + old_capacity, 0, obuf->capacity - old_capacity);
  }
}

void M_OBufAppend(obuf_t *obuf, void *obj) {
  int index = obuf->capacity;

  M_OBufEnsureCapacity(obuf, obuf->capacity + 1);
  M_OBufInsert(obuf, index, obj);
}

void M_OBufInsert(obuf_t *obuf, int index, void *obj) {
  if (index >= obuf->capacity) {
    I_Error("M_OBufInsert: Insertion index %d out of bounds (%d)",
      index, obuf->capacity
    );
  }

  obuf->objects[index] = obj;
}

int M_OBufInsertAtFirstFreeSlot(obuf_t *obuf, void *obj) {
  for (int i = 0; i < obuf->capacity; i++) {
    if (obuf->objects[i] == NULL) {
      M_OBufInsert(obuf, i, obj);
      return i;
    }
  }

  return -1;
}

int M_OBufInsertAtFirstFreeSlotOrAppend(obuf_t *obuf, void *obj) {
  int slot = M_OBufInsertAtFirstFreeSlot(obuf, obj);

  if (slot != -1)
    return slot;

  M_OBufAppend(obuf, obj);
  return obuf->capacity - 1;
}

dboolean M_OBufIter(obuf_t *obuf, int *index, void **obj) {
  for (int i = (*index) + 1; i < obuf->capacity; i++) {
    if (obuf->objects[i] != NULL) {
      *index = i;
      *obj = obuf->objects[i];
      return true;
    }
  }

  *index = -1;
  return false;
}

void M_OBufRemove(obuf_t *obuf, int index) {
  obuf->objects[index] = NULL;
}

void* M_OBufGet(obuf_t *obuf, int index) {
  if (!M_OBufIsValidIndex(obuf, index))
    return NULL;

  return obuf->objects[index];
}

void M_OBufConsolidate(obuf_t *obuf) {
  int d = 0;
  int s = 0;

  while (true) {
    while (d < obuf->capacity && obuf->objects[d] != NULL)
      d++;

    if (d >= obuf->capacity)
      return;

    if (s < d)
      s = d + 1;

    while (s < obuf->capacity && obuf->objects[s] == NULL)
      s++;

    if (s >= obuf->capacity)
      return;

    obuf->objects[d] = obuf->objects[s];
    obuf->objects[s] = NULL;
  }
}

void M_OBufClear(obuf_t *obuf) {
  for (int i = 0; i < obuf->capacity; i++)
    M_OBufRemove(obuf, i);
}

void M_OBufFreeEntriesAndClear(obuf_t *obuf) {
  for (int i = 0; i < obuf->capacity; i++) {
    if (obuf->objects[i] != NULL) {
      free(obuf->objects[i]);
    }
  }

  M_OBufClear(obuf);
}

void M_OBufFree(obuf_t *obuf) {
  free(obuf->objects);
  M_OBufInit(obuf);
}

/* vi: set et ts=2 sw=2: */

