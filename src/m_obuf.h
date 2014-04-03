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

#define OBUF_FOR_EACH(ob, i, ot, on) \
  for (int (i) = -1, (ot) (on) = NULL; M_OBufIter(&(i), (void **)&on);)

typedef struct obuf_s {
  int capacity;
  void **objects;
} obuf_t;

void     M_OBufInit(obuf_t *obuf);
void     M_OBufInitWithCapacity(obuf_t *obuf, int capacity);
dboolean M_OBufIsValidIndex(obuf_t *obuf, int index);
int      M_OBufGetObjectCount(obuf_t *obuf);
void     M_OBufEnsureCapacity(obuf_t *obuf, int capacity);
void     M_OBufAppend(obuf_t *obuf, void *obj);
void     M_OBufInsert(obuf_t *obuf, int index, void *obj);
int      M_OBufInsertAtFirstFreeSlot(obuf_t *obuf, void *obj);
int      M_OBufInsertAtFirstFreeSlotOrAppend(obuf_t *obuf, void *obj);
dboolean M_OBufIter(obuf_t *obuf, int *index, void **obj);
void*    M_OBufGet(obuf_t *obuf, int index);
void     M_OBufRemove(obuf_t *obuf, int index);
void     M_OBufConsolidate(obuf_t *obuf);
void     M_OBufClear(obuf_t *obuf);
void     M_OBufFreeEntriesAndClear(obuf_t *obuf);
void     M_OBufFree(obuf_t *obuf);

#endif

/* vi: set et sw=2 ts=2: */

