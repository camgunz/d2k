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

void M_ObjBufferInitWithSize(objbuf_t **obuf, int size) {
  (*obuf) = calloc(1, sizeof(objbuf_t));

  if ((*obuf) == NULL)
    I_Error("M_ObjBufferInitWithSize: Allocating buffer failed");

  (*obuf)->size = size;
  (*obuf)->objects = calloc(size, sizeof(void *));
}

void M_ObjBufferAppend(objbuf_t *obuf, void *obj) {
  int index = obuf->size;

  M_ObjBufferEnsureSize(obuf->size + 1);
  M_ObjBufferInsert(obuf, index, obj);
}

void M_ObjBufferInsert(objbuf_t *obuf, int index, void *obj) {
  if (index >= obuf->size) {
    I_Error("M_ObjBufferInsert: Insertion index %d out of bounds (%d)", 
      index,
      obuf->size
    );
  }
  obuf->objects[index] = obj;
}

int M_ObjBufferInsertAtFirstFreeSlot(objbuf_t *obuf, void *obj) {
  for (int i = 0; i < obuf->size; i++) {
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
  return obuf->size - 1;
}

void M_ObjBufferRemove(objbuf_t *obuf, int index) {
  obuf->objects[index] = NULL;
}

void M_ObjBufferEnsureSize(objbuf_t *obuf, int size) {
  if (obuf->size < size) {
    int old_size = obuf->size;

    obuf->size = size;
    obuf->objects = realloc(obuf->objects, obuf->size * sizeof(void *));

    if (obuf->objects == NULL)
      I_Error("M_ObjBufferEnsureSize: Allocating buffer objects failed");

    memset(obuf->objects + old_size, 0, obuf->size - old_size);
  }
}

int M_ObjBufferGetObjectCount(objbuf_t *obuf) {
  int count = 0;

  for (int i = 0; i < obuf->size; i++) {
    if (objf->objects[i] != NULL) {
      count++;
    }
  }

  return count;
}

void M_ObjBufferClear(objbuf_t *obuf) {
  memset(obuf->objects, 0, obuf->size);
  obuf->size = 0;
}

void M_ObjBufferFreeEntriesAndClear(objbuf_t *obuf) {
  for (int i = 0; i < obuf->size; i++) {
    if (objf->objects[i] != NULL) {
      free(obuf->objects[i]);
    }
  }

  M_ObjBufferClear(obuf);
}

void M_ObjBufferFree(objbuf_t *obuf) {
  obuf->size = 0;
  free(obuf->objects);
}

