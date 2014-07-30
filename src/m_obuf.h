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

#ifndef M_OBUF_H__
#define M_OBUF_H__

#define OBUF_FOR_EACH(ob, oin) for (                                         \
  obufiternode_t (oin) = {-1, NULL}; M_OBufIter((ob), &oin.index, &oin.obj); \
)

typedef struct obuf_s {
  int capacity;
  void **objects;
} obuf_t;

typedef struct obufiternode_s {
  int index;
  void *obj;
} obufiternode_t;

obuf_t*  M_OBufNew(void);
obuf_t*  M_OBufNewWithCapacity(int capacity);
void     M_OBufInit(obuf_t *obuf);
void     M_OBufInitWithCapacity(obuf_t *obuf, int capacity);
dboolean M_OBufIsValidIndex(obuf_t *obuf, int index);
int      M_OBufGetObjectCount(obuf_t *obuf);
void     M_OBufEnsureCapacity(obuf_t *obuf, int capacity);
void     M_OBufAppend(obuf_t *obuf, const void *obj);
void     M_OBufInsert(obuf_t *obuf, int index, const void *obj);
int      M_OBufInsertAtFirstFreeSlot(obuf_t *obuf, const void *obj);
int      M_OBufInsertAtFirstFreeSlotOrAppend(obuf_t *obuf, const void *obj);
dboolean M_OBufIter(obuf_t *obuf, int *index, void **obj);
void*    M_OBufGet(obuf_t *obuf, int index);
void     M_OBufRemove(obuf_t *obuf, int index);
void     M_OBufConsolidate(obuf_t *obuf);
void     M_OBufClear(obuf_t *obuf);
void     M_OBufFreeEntriesAndClear(obuf_t *obuf);
void     M_OBufFree(obuf_t *obuf);

#endif

