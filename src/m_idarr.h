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

typedef struct {
  GArray *objs;
  GArray *recycled_ids;
  uint32_t max_id;
  GDestroyNotify free_obj;
} id_list_t;

void     M_IDListInit(id_list_t *idlist, GDestroyNotify free_obj);
uint32_t M_IDListGetNewID(id_list_t *idlist, void *obj);
void     M_IDListAssignID(id_list_t *idlist, void *obj, uint32_t id);
void     M_IDListReleaseID(id_list_t *idlist, uint32_t id);
void*    M_IDListLookupObj(id_list_t *idlist, uint32_t id);
bool     M_IDListIterate(id_list_t *idlist, size_t *index, void **obj);
uint32_t M_IDListGetSize(id_list_t *idlist);
void     M_IDListReset(id_list_t *idlist);
void     M_IDListFree(id_list_t *idlist);

#endif

/* vi: set et ts=2 sw=2: */
