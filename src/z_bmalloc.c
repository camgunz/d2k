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
#include "z_bmalloc.h"
#include "lprintf.h"

typedef struct bmalpool_s {
  struct bmalpool_s *nextpool;
  size_t             blocks;
  byte               used[0];
} bmalpool_t;

inline static void* getelem(bmalpool_t *p, size_t size, size_t n) {
  return (((byte*)p) + sizeof(bmalpool_t) + sizeof(byte)*(p->blocks) + size*n);
}

inline static PUREFUNC int iselem(const bmalpool_t *pool, size_t size, const void* p) {
  // CPhipps - need portable # of bytes between pointers
  int dif = (const char*)p - (const char*)pool;

  dif -= sizeof(bmalpool_t);
  dif -= pool->blocks;

  if (dif < 0)
    return -1;

  dif /= size;
  return (((size_t)dif >= pool->blocks) ? -1 : dif);
}

enum {
  unused_block = 0,
  used_block = 1
};

void* Z_BMalloc(struct block_memory_alloc_s *pzone) {
  register bmalpool_t **pool = (bmalpool_t **)&(pzone->firstpool);

  while (*pool != NULL) {
    // Scan for unused marker
    byte *p = memchr((*pool)->used, unused_block, (*pool)->blocks);

    if (p) {
      int n = p - (*pool)->used;
#ifdef SIMPLECHECKS
      if ((n < 0) || ((size_t)n >= (*pool)->blocks))
        I_Error("Z_BMalloc: memchr returned pointer outside of array");
#endif
      (*pool)->used[n] = used_block;
      return getelem(*pool, pzone->size, n);
    }
    else {
      pool = &((*pool)->nextpool);
    }
  }
  {
    // Nothing available, must allocate a new pool
    bmalpool_t *newpool;

    // CPhipps: Allocate new memory, initialised to 0

    *pool = newpool = Z_Calloc(
      sizeof(*newpool) + (sizeof(byte) + pzone->size)*(pzone->perpool),
      1,
      pzone->tag,
      NULL
    );
    newpool->nextpool = NULL; // NULL = (void*)0 so this is redundant

    // Return element 0 from this pool to satisfy the request
    newpool->used[0] = used_block;
    newpool->blocks = pzone->perpool;
    return getelem(newpool, pzone->size, 0);
  }
}

void Z_BFree(struct block_memory_alloc_s *pzone, void* p) {
  register bmalpool_t **pool = (bmalpool_t**)&(pzone->firstpool);

  while (*pool != NULL) {
    int n = iselem(*pool, pzone->size, p);

    if (n >= 0) {
#ifdef SIMPLECHECKS
      if ((*pool)->used[n] == unused_block)
        I_Error("Z_BFree: Refree in zone %s", pzone->desc);
#endif
      (*pool)->used[n] = unused_block;
      if (memchr(((*pool)->used), used_block, (*pool)->blocks) == NULL) {
        // Block is all unused, can be freed
        bmalpool_t *oldpool = *pool;
        *pool = (*pool)->nextpool;
        Z_Free(oldpool);
      }
      return;
    }
    else {
      pool = &((*pool)->nextpool);
    }
  }
  I_Error("Z_BFree: Free not in zone %s", pzone->desc);
}

/* vi: set et ts=2 sw=2: */

