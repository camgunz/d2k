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


#ifndef M_IDHASH_H__
#define M_IDHASH_H__

typedef struct {
  GHashTable *objs;
  GArray *recycled_ids;
  uint32_t max_id;
} id_hash_t;

typedef struct {
  bool initialized;
  GHashTableIter iterator;
  uint32_t id;
  void *obj;
} id_hash_iterator_t;

void     M_IDHashInit(id_hash_t *idhash, GDestroyNotify free_obj);
uint32_t M_IDHashGetNewID(id_hash_t *idhash, void *obj);
void     M_IDHashAssignID(id_hash_t *idhash, void *obj, uint32_t id);
void     M_IDHashReleaseID(id_hash_t *idhash, uint32_t id);
void*    M_IDHashLookupObj(id_hash_t *idhash, uint32_t id);
bool     M_IDHashIterate(id_hash_t *idhash, id_hash_iterator_t *iterator);
uint32_t M_IDHashGetSize(id_hash_t *idhash);
void     M_IDHashReset(id_hash_t *idhash);
void     M_IDHashFree(id_hash_t *idhash);

#endif

/* vi: set et ts=2 sw=2: */
