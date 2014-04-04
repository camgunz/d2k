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
 *  A simple linked list
 *
 *-----------------------------------------------------------------------------
 */

#ifndef M_LIST_H__
#define M_LIST_H__

#define LIST_FOR_EACH(ls, lin) for (      \
  listiternode_t (lin) = {NULL, NULL};    \
  M_ListIterObj(ls, &lin.node, &lin.obj); \
)

typedef struct listnode_s {
  struct listnode_s *prev;
  struct listnode_s *next;
  void              *obj;
} listnode_t;

typedef struct list_s {
  int         size;
  listnode_t *nodes;
} list_t;

typedef struct listiternode_s {
  listnode_t *node;
  void *obj;
} listiternode_t;

void        M_ListInit(list_t *ls);
dboolean    M_ListIsEmpty(list_t *ls);
void        M_ListCopy(list_t *dst, list_t *src);
void        M_ListConcat(list_t *dst, list_t *src);
void        M_ListPrepend(list_t *ls, void *obj);
void        M_ListPushFront(list_t *ls, void *obj);
void        M_ListAppend(list_t *ls, void *obj);
void        M_ListPush(list_t *ls, void *obj);
void        M_ListInsertBefore(list_t *ls, listnode_t *node, void *obj);
void        M_ListInsertAfter(list_t *ls, listnode_t *node, void *obj);
void        M_ListSetHead(list_t *ls, listnode_t *new_head);
dboolean    M_ListIter(list_t *ls, listnode_t **node);
dboolean    M_ListIterObj(list_t *ls, listnode_t **node, void **obj);
listnode_t* M_ListGetNode(list_t *ls, void *obj);
int         M_ListGetIndex(list_t *ls, void *obj);
listnode_t* M_ListRemove(list_t *ls, listnode_t *node);
void*       M_ListPop(list_t *ls);
void*       M_ListPopBack(list_t *ls);
void*       M_ListPeek(list_t *ls);
void        M_ListClear(list_t *ls);
void        M_ListFree(list_t *ls);
void        M_ListFreeEntriesAndClear(list_t *ls);

#endif

/* vi: set et ts=2 sw=2: */

