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
 *  A buffer of objects
 *
 *-----------------------------------------------------------------------------
 */

#include "z_zone.h"
#include "l_printf.h"
#include "m_obuf.h"

void M_ObjBufferInit(objbuf_t **obuf) {
  (*obuf) = calloc(1, sizeof(objbuf_t));

  if ((*obuf) == NULL)
    I_Error("M_ObjBufferInit: Allocating buffer failed");
}

void M_ObjBufferInitWithCapacity(objbuf_t **obuf, int capacity) {
  (*obuf) = calloc(1, sizeof(objbuf_t));

  if ((*obuf) == NULL)
    I_Error("M_ObjBufferInitWithCapacity: Allocating buffer failed");

  (*obuf)->capacity = capacity;
  (*obuf)->objects = calloc(capacity, sizeof(void *));
}

void M_ObjBufferAppend(objbuf_t *obuf, void *obj) {
  int index = obuf->capacity;

  M_ObjBufferEnsureCapacity(obuf->capacity + 1);
  M_ObjBufferInsert(obuf, index, obj);
}

void M_ObjBufferInsert(objbuf_t *obuf, int index, void *obj) {
  if (index >= obuf->capacity) {
    I_Error("M_ObjBufferInsert: Insertion index %d out of bounds (%d)",
      index, obuf->capacity
    );
  }

  obuf->objects[index] = obj;
}

int M_ObjBufferInsertAtFirstFreeSlot(objbuf_t *obuf, void *obj) {
  for (int i = 0; i < obuf->capacity; i++) {
    if (obuf[i] == NULL) {
      obuf->objects[i] = obj;
      return i;
    }
  }

  return -1;
}

int M_ObjBufferInsertAtFirstFreeSlotOrAppend(objbuf_t *obuf, void *obj) {
  int slot = M_ObjBufferInsertAtFirstFreeSlot(obuf, obj);

  if (slot != -1)
    return slot;

  M_ObjBufferAppend(obuf, obj);
  return obuf->capacity - 1;
}

dboolean M_ObjBufferIter(objbuf_t *obuf, int *index, void **obj) {
  int i;

  if ((*index) == -1)
    i = 0;
  else
    i = (*index);

  for (*index = -1, *obj = NULL;i < obuf->capacity; i++) {
    if (obuf->objects[i] != NULL) {
      *index = i;
      *obj = obuf->objects[i];
      return true;
    }
  }

  return false;
}

void* M_ObjBufferIter(objbuf_t *obuf, void *obj) {
  int i = 0;
  void *out = obuf->objects[0];

  if (obj != NULL)
    for (; i < obuf->capacity && obuf->objects[i] != obj; i++);

  if (i == obuf->capacity)
    return NULL;

  for (; i < obuf->capacity; i++) {
    if (obuf->objects[i] != NULL) {
      return obuf->objects[i];
    }
  }

  return NULL;
}

int M_ObjBufferIterIndex(objbuf_t *obuf, int index) {
  int i;

  if (index == -1)
    i = 0;
  else
    i = index;

  for (; i < obuf->capacity; i++) {
    if (obuf->objects[i] != NULL) {
      return i;
    }
  }
}

void M_ObjBufferConsolidate(objbuf_t *obuf) {
  void **dst = obuf->objects;
  void **src = obuf->objects;

  while (true) {
    while (*dst != NULL && dst - obuf->objects < obuf->capacity)
      dst++;

    if (dst - obuf->objects >= obuf->capacity)
      return;

    if (src < dst)
      src = dst + 1;

    while (src == NULL && src - obuf->objects < obuf->capacity)
      src++;

    if (src - obuf->objects >= obuf->capacity)
      return;

    *dst = *src;
    *src = NULL;
  }
}

void M_ObjBufferRemove(objbuf_t *obuf, int index) {
  obuf->objects[index] = NULL;
}

void M_ObjBufferEnsureCapacity(objbuf_t *obuf, int capacity) {
  if (obuf->capacity < capacity) {
    int old_capacity = obuf->capacity;

    obuf->capacity = capacity;
    obuf->objects = realloc(obuf->objects, obuf->capacity * sizeof(void *));

    if (obuf->objects == NULL)
      I_Error("M_ObjBufferEnsureCapacity: Allocating buffer objects failed");

    memset(obuf->objects + old_capacity, 0, obuf->capacity - old_capacity);
  }
}

int M_ObjBufferGetObjectCount(objbuf_t *obuf) {
  int count = 0;

  for (int i = 0; i < obuf->capacity; i++) {
    if (objf->objects[i] != NULL) {
      count++;
    }
  }

  return count;
}

void M_ObjBufferClear(objbuf_t *obuf) {
  memset(obuf->objects, 0, obuf->capacity);
}

void M_ObjBufferFreeEntriesAndClear(objbuf_t *obuf) {
  for (int i = 0; i < obuf->capacity; i++) {
    if (objf->objects[i] != NULL) {
      free(obuf->objects[i]);
    }
  }

  M_ObjBufferClear(obuf);
}

void M_ObjBufferFree(objbuf_t *obuf) {
  obuf->capacity = 0;
  free(obuf->objects);
}

/* vi: set et ts=2 sw=2: */

