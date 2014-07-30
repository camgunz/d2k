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

#include "doomtype.h"
#include "lprintf.h"
#include "m_cbuf.h"

void M_CBufInit(cbuf_t *cbuf, size_t obj_size) {
  cbuf->capacity = 0;
  cbuf->size     = 0;
  cbuf->obj_size = obj_size;
  cbuf->nodes    = NULL;
}

void M_CBufInitWithCapacity(cbuf_t *cbuf, size_t obj_size, int capacity) {
  M_CBufInit(cbuf, obj_size);
  M_CBufEnsureCapacity(cbuf, capacity);
}

dboolean M_CBufIsValidIndex(cbuf_t *cbuf, int index) {
  if (index >= 0 && index < cbuf->capacity && cbuf->used[index])
    return true;

  return false;
}

int M_CBufGetObjectCount(cbuf_t *cbuf) {
  return cbuf->size;
}

void M_CBufEnsureCapacity(cbuf_t *cbuf, int capacity) {
  if (cbuf->capacity < capacity) {
    int old_capacity = cbuf->capacity;

    cbuf->capacity = capacity;
    cbuf->used = realloc(cbuf->used, cbuf->capacity * sizeof(bool));

    if (cbuf->used == NULL)
      I_Error("M_CBufEnsureCapacity: Allocating buffer used list failed");

    cbuf->nodes = realloc(cbuf->nodes, cbuf->capacity * sizeof(void *));

    if (cbuf->nodes == NULL)
      I_Error("M_CBufEnsureCapacity: Allocating buffer nodes failed");

    for (int i = old_capacity; i < cbuf->capacity; i++) {
      cbuf->used[i] = false;
      cbuf->nodes[i] = malloc(cbuf->obj_size);

      if (cbuf->nodes[i] == NULL)
        I_Error("M_CBufEnsureCapacity: Allocating buffer node's obj failed");
    }
  }
}

int M_CBufAppend(cbuf_t *cbuf, void *obj) {
  int slot = M_CBufInsertAtFirstFreeSlot(cbuf, obj);

  if (slot != -1)
    return slot;

  return M_CBufAppendNew(cbuf, obj);
}

int M_CBufAppendNew(cbuf_t *cbuf, void *obj) {
  int index = cbuf->capacity;

  M_CBufEnsureCapacity(cbuf, cbuf->capacity + 1);
  return M_CBufInsert(cbuf, index, obj);
}

int M_CBufInsert(cbuf_t *cbuf, int index, void *obj) {
  if (index >= cbuf->capacity) {
    I_Error("M_CBufInsert: Insertion index %d out of bounds (>= %d)",
      index, cbuf->capacity
    );
  }

  memcpy(cbuf->nodes[index], obj, cbuf->obj_size);
  cbuf->used[index] = true;
  cbuf->size++;

  return index;
}

int M_CBufInsertAtFirstFreeSlot(cbuf_t *cbuf, void *obj) {
  for (int i = 0; i < cbuf->capacity; i++) {
    if (!cbuf->used[i]) {
      M_CBufInsert(cbuf, i, obj);
      return i;
    }
  }

  return -1;
}

dboolean M_CBufIter(cbuf_t *cbuf, int *index, void **obj) {
  for (int i = (*index) + 1; i < cbuf->capacity; i++) {
    if (cbuf->used[i]) {
      *index = i;
      *obj = cbuf->nodes[i];
      return true;
    }
  }

  *index = -1;
  return false;
}

void* M_CBufGet(cbuf_t *cbuf, int index) {
  if (!M_CBufIsValidIndex(cbuf, index))
    return NULL;

  return cbuf->nodes[index];
}

dboolean M_CBufPop(cbuf_t *cbuf, void *obj) {
  void *buffered_object = M_CBufGet(cbuf, 0);

  if (buffered_object == NULL)
    return false;

  memcpy(obj, buffered_object, cbuf->obj_size);
  M_CBufRemove(cbuf, 0);
  M_CBufConsolidate(cbuf);

  return true;
}

void* M_CBufGetFirstFreeSlot(cbuf_t *cbuf) {
  for (int i = 0; i < cbuf->capacity; i++) {
    if (!cbuf->used[i]) {
      cbuf->used[i] = true;
      cbuf->size++;
      return cbuf->nodes[i];
    }
  }

  return NULL;
}

void* M_CBufGetNewSlot(cbuf_t *cbuf) {
  int index = cbuf->capacity;

  M_CBufEnsureCapacity(cbuf, cbuf->capacity + 1);
  cbuf->used[index] = true;
  cbuf->size++;
  return cbuf->nodes[index];
}

void* M_CBufGetFirstFreeOrNewSlot(cbuf_t *cbuf) {
  void *slot = M_CBufGetFirstFreeSlot(cbuf);

  if (slot == NULL)
    slot = M_CBufGetNewSlot(cbuf);

  return slot;
}

void M_CBufRemove(cbuf_t *cbuf, int index) {
  if (M_CBufIsValidIndex(cbuf, index)) {
    cbuf->used[index] = false;
    cbuf->size--;
    memset(cbuf->nodes[index], 0, cbuf->obj_size);
  }
}

void M_CBufConsolidate(cbuf_t *cbuf) {
  int d = 0;
  int s = 0;
  void *tmp = NULL;

  while (true) {
    while (d < cbuf->capacity && cbuf->used[d])
      d++;

    if (d >= cbuf->capacity)
      return;

    if (s < d)
      s = d + 1;

    while (s < cbuf->capacity && (!cbuf->used[s]))
      s++;

    if (s >= cbuf->capacity)
      return;

    tmp = cbuf->nodes[d];
    cbuf->nodes[d] = cbuf->nodes[s];
    cbuf->nodes[s] = tmp;
    cbuf->used[d] = true;
    cbuf->used[s] = false;
  }
}

void M_CBufClear(cbuf_t *cbuf) {
  for (int i = 0; i < cbuf->capacity; i++)
    M_CBufRemove(cbuf, i);
}

void M_CBufFree(cbuf_t *cbuf) {
  for (int i = 0; i < cbuf->capacity; i++)
    free(cbuf->nodes[i]);

  free(cbuf->nodes);
  M_CBufInit(cbuf, 0);
}

/* vi: set et ts=2 sw=2: */

