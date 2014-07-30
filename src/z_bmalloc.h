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
/* vi: set et ts=2 sw=2:                                                     */
/*                                                                           */
/*****************************************************************************/

#ifndef Z_BMALLOC_H__
#define Z_BMALLOC_H__

struct block_memory_alloc_s {
  void  *firstpool;
  size_t size;
  size_t perpool;
  int    tag;
  const char *desc;
};

#define DECLARE_BLOCK_MEMORY_ALLOC_ZONE(name) extern struct block_memory_alloc_s name
#define IMPLEMENT_BLOCK_MEMORY_ALLOC_ZONE(name, size, tag, num, desc) \
struct block_memory_alloc_s name = { NULL, size, num, tag, desc}
#define NULL_BLOCK_MEMORY_ALLOC_ZONE(name) name.firstpool = NULL

void* Z_BMalloc(struct block_memory_alloc_s *pzone);

inline static void* Z_BCalloc(struct block_memory_alloc_s *pzone)
{ void *p = Z_BMalloc(pzone); memset(p,0,pzone->size); return p; }

void Z_BFree(struct block_memory_alloc_s *pzone, void* p);

#endif //__Z_BMALLOC__

