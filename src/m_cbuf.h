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


#ifndef M_CBUF_H__
#define M_CBUF_H__

#define CBUF_FOR_EACH(cb, cin) for (                                         \
  cbufiternode_t (cin) = {-1, NULL}; M_CBufIter((cb), &cin.index, &cin.obj); \
)

typedef struct cobjbuf_s {
  int capacity;
  int size;
  size_t obj_size;
  bool *used;
  void **nodes;
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
int      M_CBufAppend(cbuf_t *cbuf, void *obj);
int      M_CBufAppendNew(cbuf_t *cbuf, void *obj);
int      M_CBufInsert(cbuf_t *cbuf, int index, void *obj);
int      M_CBufInsertAtFirstFreeSlot(cbuf_t *cbuf, void *obj);
dboolean M_CBufIter(cbuf_t *cbuf, int *index, void **obj);
void*    M_CBufGet(cbuf_t *cbuf, int index);
dboolean M_CBufPop(cbuf_t *cbuf, void *obj);
void*    M_CBufGetFirstFreeSlot(cbuf_t *cbuf);
void*    M_CBufGetNewSlot(cbuf_t *cbuf);
void*    M_CBufGetFirstFreeOrNewSlot(cbuf_t *cbuf);
void     M_CBufRemove(cbuf_t *cbuf, int index);
void     M_CBufConsolidate(cbuf_t *cbuf);
void     M_CBufClear(cbuf_t *cbuf);
void     M_CBufFree(cbuf_t *cbuf);

#endif

/* vi: set et ts=2 sw=2: */

