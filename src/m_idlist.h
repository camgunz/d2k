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


#ifndef M_IDLIST_H__
#define M_IDLIST_H__

typedef uint32_t (*M_GetIDFunc)(void *data);

typedef struct {
  GList *objs;
  GArray *recycled_ids;
  uint32_t max_id;
  size_t len;
  M_GetIDFunc get_id;
  GDestroyNotify free_obj;
} id_list_t;

typedef struct {
  GList *node;
  void *obj;
  void *wraparound;
} id_list_iter_t;

#define IDLIST_FOR_EACH(_idlist, _it) \
  for (id_list_iter_t _it = {_idlist, NULL, NULL, NULL}; \
       M_IDListIterate(_idlist, &_it.node, &_id.obj, &_id.wraparound);)

#define IDLIST_FOR_EACH_AT(_idlist, _it, _obj) \
  for (id_list_iter_t _iter = {_idlist, NULL, _obj, _obj}; \
       M_IDListIterate(_idlist, &_it.node, &_id.obj, &_id.wraparound);)

void     M_IDListInit(id_list_t *idlist, M_GetIDFunc get_id,
                                         GDestroyNotify free_obj);
uint32_t M_IDListAdd(id_list_t *idlist, void *obj);
void     M_IDListAssign(id_list_t *idlist, void *obj, uint32_t id);
bool     M_IDListRemove(id_list_t *idlist, void *obj);
bool     M_IDListRemoveID(id_list_t *idlist, uint32_t id);
void*    M_IDListLookup(id_list_t *idlist, uint32_t id);
bool     M_IDListIterate(id_list_t *idlist, GList **node, void **obj,
                                                          void *wraparound);
uint32_t M_IDListGetCount(id_list_t *idlist);
void     M_IDListReset(id_list_t *idlist);
void     M_IDListFree(id_list_t *idlist);

#endif

/* vi: set et ts=2 sw=2: */
