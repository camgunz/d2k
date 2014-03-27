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

#ifndef M_OBUF_H__
#define M_OBUF_H__

typedef struct objbuf_s {
  int capacity;
  void **objects;
} objbuf_t;

/*
 * Allocates and initializes an object buffer.
 */
void M_ObjBufferInit(objbuf_t **obuf);

/*
 * Allocates and initializes an object buffer with the specified number of
 * object spots available; allows avoiding several initial allocations.
 */
void M_ObjBufferInitWithCapacity(objbuf_t **obuf, int capacity);

/*
 * Appends an object to the buffer, allocating a new slot.
 */
void M_ObjBufferAppend(objbuf_t *obuf, void *obj);

/*
 * Inserts an object into the buffer, but will not allocate a new slot if the
 * index is out of bounds; rather I_Error is called.
 */
void M_ObjBufferInsert(objbuf_t *obuf, int index, void *obj);

/*
 * Inserts an object into the first free slot and returns the index at which
 * the object can be found.  If no slot was available, -1 is returned.
 */
int  M_ObjBufferInsertAtFirstFreeSlot(objbuf_t *obuf, void *obj);

/*
 * Inserts an object into the first free slot.  If no free slot was available,
 * allocates a new slot at the end of the buffer and inserts the object there.
 * Returns the index at which the object can be found.
 */
int  M_ObjBufferInsertAtFirstFreeSlotOrAppend(objbuf_t *obuf, void *obj);

/*
 * Iterates over an object buffer.  Passing index -1 returns the first non-NULL
 * object in obj, passing a different index returns the first non-NULL object
 * with a greater index in obj.  Returns true if a non-NULL object was found.
 */
dboolean M_ObjBufferIter(objbuf_t *obuf, int *index, void **obj) {

/*
 * Consolidates all occupied slots to the front of the buffer; no empty space
 * will be at the beginning of the buffer or between objects; all empty space
 * will be at the end of the buffer (if any is available).
 *
 * WARNING: This will change the indices of contained objects.
 *
 */
void M_ObjBufferConsolidate(objbuf_t *obuf);

/*
 * Removes the object at the specified index from the buffer.  Object is not
 * freed before it is removed.
 */
void M_ObjBufferRemove(objbuf_t *obuf, int index);

/*
 * Ensures the buffer is at least the specified capacity; if not, it is
 * reallocated to the new capacity.
 */
void M_ObjBufferEnsureCapacity(objbuf_t *obuf, int capacity);

/*
 * Returns the total number of objects contained in the buffer.
 *
 * (for capacity, simply use buffer->capacity)
 *
 */
int  M_ObjBufferGetObjectCount(objbuf_t *obuf);

/*
 * Removes all objects from the buffer.  Objects are not freed before they are
 * removed.
 */
void M_ObjBufferClear(objbuf_t *obuf);

/*
 * Removes all objects from the buffer.  Objects are freed before they are
 * removed.
 */
void M_ObjBufferFreeEntriesAndClear(objbuf_t *obuf);

/*
 * Frees the buffer's internal list, effectively removing all objects from the
 * buffer.  Objects are not freed before they are removed.
 */
void M_ObjBufferFree(objbuf_t *obuf);

#endif

