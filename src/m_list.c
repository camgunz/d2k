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
#include "m_list.h"

static listnode_t* make_node(void *obj) {
  listnode_t *node = calloc(1, sizeof(listnode_t));

  node->prev = node;
  node->next = node;
  node->obj  = obj;

  return node;
}

static void insert_head(list_t *ls, void *obj) {
  ls->nodes = make_node(obj);

  ls->size++;
}

static void insert_before(list_t *ls, listnode_t *node, listnode_t *new_node) {
  new_node->prev   = node->prev;
  new_node->next   = node;
  node->prev->next = new_node;
  node->prev       = new_node;

  ls->size++;
}

void M_ListInit(list_t *ls) {
  M_ListClear(ls);
}

dboolean M_ListIsEmpty(list_t *ls) {
  return ls->nodes == NULL;
}

void M_ListCopy(list_t *dst, list_t *src) {
  M_ListFreeEntriesAndClear(dst);
  M_ListConcat(dst, src);
}

void M_ListConcat(list_t *dst, list_t *src) {
  listnode_t *node = NULL;
  void *obj = NULL;

  while (M_ListIterObj(src, &node, &obj))
    M_ListAppend(dst, obj);
}

void M_ListPrepend(list_t *ls, void *obj) {
  if (ls->nodes == NULL)
    insert_head(ls, obj);
  else
    M_ListInsertBefore(ls, ls->nodes, obj);

  if (ls->nodes != ls->nodes->prev)
    M_ListSetHead(ls, ls->nodes->prev);
}

void M_ListPushFront(list_t *ls, void *obj) {
  M_ListPrepend(ls, obj);
}

void M_ListAppend(list_t *ls, void *obj) {
  if (ls->nodes == NULL)
    insert_head(ls, obj);
  else
    M_ListInsertAfter(ls, ls->nodes->prev, obj);
}

void M_ListPush(list_t *ls, void *obj) {
  M_ListAppend(ls, obj);
}

void M_ListInsertBefore(list_t *ls, listnode_t *node, void *obj) {
  insert_before(ls, node, make_node(obj));
}

void M_ListInsertAfter(list_t *ls, listnode_t *node, void *obj) {
  insert_before(ls, node->next, make_node(obj));
}

void M_ListSetHead(list_t *ls, listnode_t *node) {
  ls->nodes = node;
}

dboolean M_ListIter(list_t *ls, listnode_t **node) {
  listnode_t *n = *node;

  if (ls->size == 0)
    return false;

  if (n == NULL) {
    *node = ls->nodes;
    return true;
  }

  if (n->next == ls->nodes)
    return false;

  *node = n->next;

  return true;
}

dboolean M_ListIterObj(list_t *ls, listnode_t **node, void **obj) {
  listnode_t *n = *node;

  if (ls->size == 0)
    return false;

  if (n == NULL) {
    *node = ls->nodes;
    *obj  = ls->nodes->obj;
    return true;
  }

  if (n->next == ls->nodes)
    return false;

  *node = n->next;
  *obj  = (*node)->obj;

  return true;
}

listnode_t* M_ListGetNode(list_t *ls, void *obj) {
  listnode_t *node = NULL;
  void *nobj = NULL;

  while (M_ListIterObj(ls, &node, &nobj)) {
    if (nobj == obj)
      return node;
  }

  return NULL;
}

int M_ListGetIndex(list_t *ls, void *obj) {
  listnode_t *node = NULL;
  void *nobj = NULL;

  for (int i = 0; M_ListIterObj(ls, &node, &nobj); i++) {
    if (nobj == obj) {
      return i;
    }
  }

  return -1;
}

listnode_t* M_ListRemove(list_t *ls, listnode_t *node) {
  listnode_t *prev = node->prev;
  listnode_t *next = node->next;

  if (node == prev) {
    ls->nodes = NULL;
    prev = NULL;
  }
  else {
    next->prev = prev;
    prev->next = next;
  }

  if (node == ls->nodes)
    ls->nodes = next;

  free(node);

  ls->size--;

  return prev;
}

void* M_ListPop(list_t *ls) {
  void *obj;

  if (M_ListIsEmpty(ls))
    return NULL;

  obj = ls->nodes->obj;
  M_ListRemove(ls, ls->nodes);

  return obj;
}

void* M_ListPopBack(list_t *ls) {
  void *obj;

  if (M_ListIsEmpty(ls))
    return NULL;

  obj = ls->nodes->prev->obj;
  M_ListRemove(ls, ls->nodes->prev);

  return obj;
}

void* M_ListPeek(list_t *ls) {
  if (M_ListIsEmpty(ls))
    return NULL;

  return ls->nodes->obj;
}

void M_ListClear(list_t *ls) {
  ls->size = 0;
  ls->nodes = NULL;
}

void M_ListFree(list_t *ls) {
  LIST_FOR_EACH(ls, entry)
    entry.node = M_ListRemove(ls, entry.node);

  M_ListClear(ls);
}

void M_ListFreeEntriesAndClear(list_t *ls) {
  LIST_FOR_EACH(ls, entry) {
    free(entry.obj);
    entry.node = M_ListRemove(ls, entry.node);
  }

  M_ListClear(ls);
}

/* vi: set et ts=2 sw=2: */

