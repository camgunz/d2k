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
 * Why build this when id_hash exists?  While it's true that hash table lookups
 * are O(1) and array lookups are O(N), it's still faster to search
 * sequentially through an array until you reach thousands of entries.
 */

#define ID_LIST_INITIAL_ALLOC 64

typedef struct {
  uint32_t id;
  void *obj;
} id_obj_tuple_t;

static bool id_list_contains(id_list_t *idlist, uint32_t id) {
  for (size_t i = 0; i < idlist->objs->len; i++) {
    id_obj_tuple_t *tuple = &g_array_index(idlist->objs, id_obj_tuple_t, i);

    if (tuple->id == id) {
      return true;
    }
  }

  return false;
}

static void* id_list_lookup(id_list_t *idlist, uint32_t id) {
  for (size_t i = 0; i < idlist->objs->len; i++) {
    id_obj_tuple_t *tuple = &g_array_index(idlist->objs, id_obj_tuple_t, i);

    if (tuple->id == id) {
      return tuple->obj;
    }
  }

  return NULL;
}

static void id_list_insert(id_list_t *idlist, uint32_t id, void *obj) {
  id_obj_tuple_t tuple;

  if (id_list_contains(idlist, id)) {
    I_Error("id_list_insert: Cannot insert duplicate IDs into an ID list");
  }

  tuple.id = id;
  tuple.obj = obj;

  g_array_append_val(idlist->objs, tuple);
}

static bool id_list_remove(id_list_t *idlist, uint32_t id) {
  for (size_t i = 0; i < idlist->objs->len; i++) {
    id_obj_tuple_t *tuple = &g_array_index(idlist->objs, id_obj_tuple_t, i);

    if (tuple->id == id) {
      g_array_remove_index_fast(idlist->objs, i);
      return true;
    }
  }

  return false;
}

void M_IDListInit(id_list_t *idlist) {
  idlist->objs = g_array_sized_new(
    false,
    true,
    sizeof(id_obj_tuple_t),
    ID_LIST_INITIAL_ALLOC
  );
  idlist->recycled_ids = g_array_sized_new(
    false,
    true,
    sizeof(uint32_t),
    ID_LIST_INITIAL_ALLOC
  );
  idlist->max_id = 0;
}

uint32_t M_IDListGetNewID(id_list_t *idlist) {
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
    id = idlist->max_id++;

    if (id == 0) {
      I_Error("M_IDListGetNewID: ID number rolled over");
    }
  }

  return id;
}

void M_IDListAssignID(id_list_t *idlist, void *obj, uint32_t obj_id) {
  if (obj_id == 0) {
    I_Error("M_IDListAssignID: ID is 0");
  }

  idlist->max_id = MAX(idlist->max_id, obj_id);

  if (id_list_contains(idlist, obj_id)) {
    I_Error("M_IDListAssignID: ID %d already assigned", obj_id);
  }

  id_list_insert(idlist, obj_id, obj);
}

void M_IDListReleaseID(id_list_t *idlist, uint32_t id) {
  if (id == 0) {
    I_Error("M_IDListReleaseID: Cannot release ID 0");
  }

  id_list_remove(idlist, id);
}

void* M_IDListLookupObj(id_list_t *idlist, uint32_t id) {
  return id_list_lookup(idlist, id);
}

void M_IDListReset(id_list_t *idlist) {
  g_array_remove_range(idlist->objs, 0, idlist->objs->len);
  g_array_remove_range(idlist->recycled_ids, 0, idlist->recycled_ids->len);
  idlist->max_id = 0;
}

void M_IDListFree(id_list_t *idlist) {
  g_array_free(idlist->objs, true);
  g_array_free(idlist->recycled_ids, true);
  idlist->max_id = 0;
}

/* vi: set et ts=2 sw=2: */
