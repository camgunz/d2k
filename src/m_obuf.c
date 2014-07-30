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
/* vi: set et ts=2 sw=2:                                                     */
/*                                                                           */
/*****************************************************************************/

#include "z_zone.h"

#include "doomtype.h"
#include "lprintf.h"
#include "m_obuf.h"

obuf_t* M_OBufNew(void) {
  obuf_t *obuf = malloc(sizeof(obuf_t));

  M_OBufInit(obuf);

  return obuf;
}

obuf_t* M_OBufNewWithCapacity(int capacity) {
  obuf_t *obuf = malloc(sizeof(obuf_t));

  M_OBufInitWithCapacity(obuf, capacity);

  return obuf;
}

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

    for (int i = old_capacity; i < obuf->capacity; i++)
      obuf->objects[i] = NULL;
  }
}

void M_OBufAppend(obuf_t *obuf, const void *obj) {
  int index = obuf->capacity;

  M_OBufEnsureCapacity(obuf, obuf->capacity + 1);
  M_OBufInsert(obuf, index, obj);
}

void M_OBufInsert(obuf_t *obuf, int index, const void *obj) {
  if (index >= obuf->capacity) {
    I_Error("M_OBufInsert: Insertion index %d out of bounds (%d)",
      index, obuf->capacity
    );
  }

  obuf->objects[index] = (void *)obj;
}

int M_OBufInsertAtFirstFreeSlot(obuf_t *obuf, const void *obj) {
  for (int i = 0; i < obuf->capacity; i++) {
    if (obuf->objects[i] == NULL) {
      M_OBufInsert(obuf, i, obj);
      return i;
    }
  }

  return -1;
}

int M_OBufInsertAtFirstFreeSlotOrAppend(obuf_t *obuf, const void *obj) {
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

