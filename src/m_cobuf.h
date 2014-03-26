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
 *  A buffer of objects that uses copies instead of pointers.
 *
 *-----------------------------------------------------------------------------
 */

#ifndef M_COBUF_H__
#define M_COBUF_H__

typedef struct cobjnode_s {
  dboolean used;
  void *obj;
} cobjnode_t;

typedef struct cobjbuf_s {
  int capacity;
  size_t obj_size;
  cobjnode *nodes;
} cobjbuf_t;

void M_CObjBufferInit(cobjbuf_t **cobuf, size_t obj_size);
void M_CObjBufferInitWithCapacity(cobjbuf_t **cobuf, size_t obj_size,
                                                     int capacity);
void M_CObjBufferAppend(cobjbuf_t *cobuf, void *obj);
void M_CObjBufferInsert(cobjbuf_t *cobuf, int index, void *obj);
int  M_CObjBufferInsertAtFirstFreeSlot(cobjbuf_t *cobuf, void *obj);
int  M_CObjBufferInsertAtFirstFreeSlotOrAppend(cobjbuf_t *cobuf, void *obj);
void M_CObjBufferConsolidate(cobjbuf_t *obuf);
void M_CObjBufferRemove(cobjbuf_t *cobuf, int index);
void M_CObjBufferEnsureCapacity(cobjbuf_t *cobuf, int capacity);
int  M_CObjBufferGetObjectCount(cobjbuf_t *cobuf);
void M_CObjBufferClear(cobjbuf_t *cobuf);
void M_CObjBufferFreeEntriesAndClear(cobjbuf_t *cobuf);
void M_CObjBufferFree(cobjbuf_t *cobuf);

#endif

