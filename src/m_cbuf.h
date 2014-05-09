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

#ifndef M_CBUF_H__
#define M_CBUF_H__

#define CBUF_FOR_EACH(cb, cin) for (                                         \
  cbufiternode_t (cin) = {-1, NULL}; M_CBufIter((cb), &cin.index, &cin.obj); \
)

typedef struct cbufnode_s {
  dboolean used;
  void *obj;
} cbufnode_t;

typedef struct cobjbuf_s {
  int capacity;
  size_t obj_size;
  cbufnode_t *nodes;
} cbuf_t;

typedef struct cbufiternode_s {
  int index;
  void *obj;
} cbufiternode_t;

void     M_CBufInit(cbuf_t *cbuf, size_t obj_sz);
void     M_CBufInitWithCapacity(cbuf_t *cbuf, size_t obj_sz, int capacity);
dboolean M_CBufIsValidIndex(cbuf_t *cbuf, int index);
int      M_CBufGetObjectCount(cbuf_t *cbuf);
void     M_CBufEnsureCapacity(cbuf_t *cbuf, int capacity);
void     M_CBufAppend(cbuf_t *cbuf, void *obj);
void     M_CBufInsert(cbuf_t *cbuf, int index, void *obj);
int      M_CBufInsertAtFirstFreeSlot(cbuf_t *cbuf, void *obj);
int      M_CBufInsertAtFirstFreeSlotOrAppend(cbuf_t *cbuf, void *obj);
dboolean M_CBufIter(cbuf_t *cbuf, int *index, void **obj);
void*    M_CBufGet(cbuf_t *cbuf, int index);
void*    M_CBufGetFirstFreeSlot(cbuf_t *cbuf);
void*    M_CBufGetNewSlot(cbuf_t *cbuf);
void*    M_CBufGetFirstFreeOrNewSlot(cbuf_t *cbuf);
void     M_CBufRemove(cbuf_t *cbuf, int index);
void     M_CBufConsolidate(cbuf_t *cbuf);
void     M_CBufClear(cbuf_t *cbuf);
void     M_CBufFree(cbuf_t *cbuf);

#endif

/* vi: set et sw=2 ts=2: */

