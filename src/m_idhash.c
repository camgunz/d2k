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
#include "m_idhash.h"

#define ID_HASH_INITIAL_ALLOC 64

void M_IDHashInit(id_hash_t *idhash, GDestroyNotify free_obj) {
  idhash->objs = g_hash_table_new_full(NULL, NULL, NULL, free_obj);
  idhash->recycled_ids = g_array_sized_new(
    false,
    true,
    sizeof(uint32_t),
    ID_HASH_INITIAL_ALLOC
  );
  idhash->max_id = 0;
}

uint32_t M_IDHashGetNewID(id_hash_t *idhash, void *obj) {
  uint32_t id;

  if (idhash->recycled_ids->len > 0) {
    id = g_array_index(
      idhash->recycled_ids,
      uint32_t,
      idhash->recycled_ids->len - 1
    );

    idhash->recycled_ids = g_array_remove_index_fast(
      idhash->recycled_ids,
      idhash->recycled_ids->len - 1
    );
  }
  else {
    idhash->max_id++;
    id = idhash->max_id;

    if (id == 0) {
      I_Error("M_IDHashGetNewID: ID number rolled over");
    }
  }

  g_hash_table_insert(idhash->objs, GUINT_TO_POINTER(id), obj);

  return id;
}

void M_IDHashAssignID(id_hash_t *idhash, void *obj, uint32_t id) {
  if (id == 0) {
    I_Error("M_IDHashAssignID: ID is 0");
  }

  idhash->max_id = MAX(idhash->max_id, id);

  if (g_hash_table_contains(idhash->objs, GUINT_TO_POINTER(id))) {
    I_Error("M_IDHashAssignID: ID %u already assigned", id);
  }

  g_hash_table_insert(idhash->objs, GUINT_TO_POINTER(id), obj);
}

void M_IDHashReleaseID(id_hash_t *idhash, uint32_t id) {
  if (id == 0) {
    I_Error("M_IDHashReleaseID: Cannot release ID 0");
  }

  g_hash_table_remove(idhash->objs, GUINT_TO_POINTER(id));
  g_array_append_val(
    idhash->recycled_ids,
    GUINT_TO_POINTER(id)
  );
}

void* M_IDHashLookupObj(id_hash_t *idhash, uint32_t id) {
  return g_hash_table_lookup(idhash->objs, GUINT_TO_POINTER(id));
}

bool M_IDHashIterate(id_hash_t *idhash, id_hash_iterator_t *iterator) {
  gpointer id;

  if (!iterator->initialized) {
    g_hash_table_iter_init(&iterator->iterator, idhash->objs);
  }

  if (!g_hash_table_iter_next(&iterator->iterator, &id, &iterator->obj)) {
    return false;
  }

  iterator->id = *(uint32_t *)id;
  return true;
}

uint32_t M_IDHashGetSize(id_hash_t *idhash) {
  return g_hash_table_size(idhash->objs);
}

void M_IDHashReset(id_hash_t *idhash) {
  g_hash_table_remove_all(idhash->objs);
  g_array_remove_range(idhash->recycled_ids, 0, idhash->recycled_ids->len);
  idhash->max_id = 0;
}

void M_IDHashFree(id_hash_t *idhash) {
  g_hash_table_destroy(idhash->objs);
  g_array_free(idhash->recycled_ids, true);
  idhash->max_id = 0;
}

/* vi: set et ts=2 sw=2: */
