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

#include <glib.h>

#include "lprintf.h"

static GHashTable *id_hash = NULL;
static uint32_t max_id = 0;

void P_IdentInit(void) {
  id_hash = g_hash_table_new(NULL, NULL);
}

void P_IdentGetID(void *obj, uint32_t *obj_id) {
  uint32_t id;

  max_id++;

  if (max_id == 0)
    I_Error("P_IdentGetID: ID number rolled over");

  id = max_id;

  if (!g_hash_table_insert(id_hash, GUINT_TO_POINTER(id), obj))
    I_Error("P_IdentGetID: ID %d already assigned", id);

  *obj_id = id;
}

void P_IdentAssignID(void *obj, uint32_t obj_id) {
  if (obj_id == 0)
    I_Error("P_IdentAssignID: ID is 0");

  max_id = MAX(max_id, obj_id);

  if (!g_hash_table_insert(id_hash, GUINT_TO_POINTER(obj_id), obj))
    I_Error("P_IdentAssignID: ID %d already assigned", obj_id);
}

void P_IdentReleaseID(uint32_t *obj_id) {
  uint32_t id = *obj_id;

  if (id == 0)
    I_Error("P_IdentReleaseID: ID %d not assigned", id);

  g_hash_table_remove(id_hash, GUINT_TO_POINTER(id));

  *obj_id = 0;
}

void* P_IdentLookup(uint32_t id) {
  return g_hash_table_lookup(id_hash, GUINT_TO_POINTER(id));
}

void P_IdentReset(void) {
  g_hash_table_remove_all(id_hash);
  max_id = 0;
}

/* vi: set et ts=2 sw=2: */

