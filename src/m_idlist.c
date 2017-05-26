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

#include "doomstat.h"
#include "m_idlist.h"

/*
 * This is easier on memory than id_arr.  Additionally it's easier to keep
 * sorted, so depending on load, id_list may be faster than id_arr.
 *
 * Essentially if you don't expect very many entries, id_list will be faster
 * and smaller for you.  If you need deterministic iteration order, you have to
 * use id_list even if it will be slower at larger sizes.
 */

#define ID_LIST_INITIAL_ALLOC 64

static int compare_objs(gconstpointer a, gconstpointer b, gpointer data) {
  id_list_t *idlist = (id_list_t *)data;

  uint32_t ida = idlist->get_id((void *)a);
  uint32_t idb = idlist->get_id((void *)b);

  if (ida < idb) {
    return -1;
  }

  if (ida > idb) {
    return 1;
  }

  return 0;
}

static bool id_list_contains(id_list_t *idlist, uint32_t id) {
  for (GList *ls = idlist->objs; ls; ls = g_list_next(ls)) {
    if (id == idlist->get_id(ls->data)) {
      return true;
    }
  }

  return false;
}

static void* id_list_lookup(id_list_t *idlist, uint32_t id) {
  for (GList *ls = idlist->objs; ls; ls = g_list_next(ls)) {
    if (id == idlist->get_id(ls->data)) {
      return ls->data;
    }
  }

  return NULL;
}

static void id_list_insert(id_list_t *idlist, void *obj) {
  uint32_t id = idlist->get_id(obj);

  if (id_list_contains(idlist, id)) {
    I_Error("id_list_insert: Cannot insert duplicate IDs into an ID list");
  }

  idlist->objs = g_list_insert_sorted_with_data(
    idlist->objs,
    obj,
    compare_objs,
    (void *)idlist
  );

  idlist->len++;
}

static bool id_list_remove_obj(id_list_t *idlist, void *obj) {
  GList *node = g_list_find(idlist->objs, obj);

  if (!node) {
    return false;
  }

  idlist->objs = g_list_delete_link(idlist->objs, node);
  idlist->len--;

  return true;
}

static bool id_list_remove_id(id_list_t *idlist, uint32_t id) {
  GList *node = NULL;
  
  for (GList *ls = idlist->objs; ls; ls = g_list_next(ls)) {
    if (idlist->get_id(ls->data) == id) {
      node = ls;
      break;
    }
  }

  if (!node) {
    return false;
  }

  idlist->objs = g_list_delete_link(idlist->objs, node);
  idlist->len--;

  return true;
}

void M_IDListInit(id_list_t *idlist, M_GetIDFunc get_id,
                                     GDestroyNotify free_obj) {
  idlist->objs = NULL;
  idlist->recycled_ids = g_array_sized_new(
    false,
    true,
    sizeof(uint32_t),
    ID_LIST_INITIAL_ALLOC
  );
  idlist->max_id = 0;
  idlist->len = 0;
  idlist->get_id = get_id;
  idlist->free_obj = free_obj;
}

uint32_t M_IDListAdd(id_list_t *idlist, void *obj) {
  uint32_t id;

  if (idlist->recycled_ids->len > 0) {
    id = g_array_index(
      idlist->recycled_ids,
      uint32_t,
      idlist->recycled_ids->len - 1
    );

    idlist->recycled_ids = g_array_remove_index_fast(
      idlist->recycled_ids,
      idlist->recycled_ids->len - 1
    );
  }
  else {
    idlist->max_id++;
    id = idlist->max_id;

    if (id == 0) {
      I_Error("M_IDListGetNewID: ID number rolled over");
    }
  }

  id_list_insert(idlist, obj);

  return id;
}

void M_IDListAssign(id_list_t *idlist, void *obj, uint32_t id) {
  if (id == 0) {
    I_Error("M_IDListAssignID: ID is 0");
  }

  if (id_list_contains(idlist, id)) {
    I_Error("M_IDListAssignID: ID %d already assigned", id);
  }

  idlist->max_id = MAX(idlist->max_id, id);

  id_list_insert(idlist, obj);
}

bool M_IDListRemove(id_list_t *idlist, void *obj) {
  return id_list_remove_obj(idlist, obj);
}

bool M_IDListRemoveID(id_list_t *idlist, uint32_t id) {
  if (id == 0) {
    I_Error("M_IDListRemoveID: Cannot remove object with ID 0");
  }

  return id_list_remove_id(idlist, id);
}

void* M_IDListLookup(id_list_t *idlist, uint32_t id) {
  return id_list_lookup(idlist, id);
}

bool M_IDListIterate(id_list_t *idlist, GList **node, void **obj,
                                                      void *wraparound) {
  if ((!(*node)) && (!(*obj))) {
    if (wraparound) {
      I_Error(
        "M_IDListIterate: Cannot wrap an iterator that starts at the "
        "beginning"
      );
    }

    (*node) = idlist->objs;
  }
  else if (!(*node)) {
    (*node) = g_list_find(idlist->objs, (*obj));

    if ((*node)) {
      (*node) = g_list_next((*node));
    }
  }
  else if ((*obj)) {
    (*node) = g_list_next((*node));
  }
  else {
    I_Error("M_IDListIterate: iterator has node but no object");
  }

  if (!(*node)) {
    if (!wraparound) {
      return false;
    }

    (*node) = idlist->objs;

    if (!(*node)) {
      return false;
    }
  }

  if ((*node)->data == wraparound) {
    return false;
  }

  (*obj) = (*node)->data;

  return true;
}

uint32_t M_IDListGetSize(id_list_t *idlist) {
  return idlist->len;
}

void M_IDListReset(id_list_t *idlist) {
  g_list_free_full(idlist->objs, idlist->free_obj);
  g_array_remove_range(idlist->recycled_ids, 0, idlist->recycled_ids->len);
  idlist->len = 0;
  idlist->max_id = 0;
}

void M_IDListFree(id_list_t *idlist) {
  g_list_free_full(idlist->objs, idlist->free_obj);
  g_array_free(idlist->recycled_ids, true);
  idlist->len = 0;
  idlist->max_id = 0;
}

/* vi: set et ts=2 sw=2: */
