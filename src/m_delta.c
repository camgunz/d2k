/* Emacs style mode select   -*- C++ -*-
 *-----------------------------------------------------------------------------
 *
 *
 *  PrBoom: a Doom port merged with LxDoom and LSDLDoom
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 1999 by
 *  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
 *  Copyright (C) 1999-2000 by
 *  Jess Haas, Nicolas Kalkhof, Colin Phipps, Florian Schulze
 *  Copyright 2005, 2006 by
 *  Florian Schulze, Colin Phipps, Neil Stevens, Andrey Budko
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 *  02111-1307, USA.
 *
 * DESCRIPTION:
 *   Binary delta routines using LibXDiff.
 *
 *-----------------------------------------------------------------------------
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "z_zone.h"

#include <xdiff.h>

#include "doomtype.h"
#include "lprintf.h"
#include "m_delta.h"

#define BLKSIZE 2097152

typedef struct buf_s {
  size_t size;
  size_t alloc;
  byte *data;
} buf_t;

static int write_delta_data(void *priv, mmbuffer_t *mb, int nbuf);

static size_t delta_count = 0;
static double avg_delta_size = 0;
static dboolean xdiff_initialized = false;
static dboolean have_initial_state = false;
static buf_t state = { 0, 0, NULL };
static buf_t cur_delta = { 0, 0, NULL };
static bdiffparam_t bdp = { 32 };
static xdemitcb_t ecb = { &cur_delta, write_delta_data };

extern int gametic;

static void* wrap_malloc(void *priv, unsigned int size) {
  return malloc(size);
}

static void wrap_free(void *priv, void *ptr) {
  free(ptr);
}

static void* wrap_realloc(void *priv, void *ptr, unsigned int size) {
  return realloc(ptr, size);
}

static int write_delta_data(void *priv, mmbuffer_t *mb, int nbuf) {
  int i;
  byte *delta_p;
  buf_t *delta = (buf_t *)priv;

  for (delta->size = 0, i = 0; i < nbuf; i++)
    delta->size += mb[i].size;

  if (delta->size > delta->alloc) {
    delta->alloc = delta->size;
    delta->data = realloc(delta->data, delta->alloc);

    if (delta->data == NULL) {
      perror("Allocating delta data failed");
      return -1;
    }
  }

  for (delta_p = delta->data, i = 0; i < nbuf; i++) {
    memcpy(delta_p, mb[i].ptr, mb[i].size);
    delta_p += mb[i].size;
  }

  return 0;
}

static void update_delta_size_average(long size) {
  delta_count++;

  if (size > avg_delta_size)
    avg_delta_size += size / (double)delta_count;
  else if (size < avg_delta_size)
    avg_delta_size -= size / (double)delta_count;
}

static void set_state_buffer(byte *game_state, size_t state_size) {
  state.size = state_size;

  if (state.size > state.alloc) {
    state.alloc = state.size;
    state.data = realloc(state.data, state.alloc);

    if (state.data == NULL)
      I_Error("Allocating state data failed");
  }

  memcpy(state.data, game_state, state_size);
}

static void initialize_xdiff(void) {
  memallocator_t malt;

  if (xdiff_initialized)
    return;

  xdiff_initialized = true;

  malt.priv = NULL;
  malt.malloc = wrap_malloc;
  malt.free = wrap_free;
  malt.realloc = wrap_realloc;

  xdl_set_allocator(&malt);
}

dboolean M_RegisterGameState(byte *game_state, size_t state_size) {
  mmfile_t cur_state, in_state;

  initialize_xdiff();

  if (!have_initial_state) {
    set_state_buffer(game_state, state_size);
    have_initial_state = true;
    return true;
  }

  if ((xdl_init_mmfile(&cur_state, BLKSIZE, XDL_MMF_ATOMIC)) != 0)
    I_Error("Error initializing current state");

  if ((xdl_init_mmfile(&in_state, BLKSIZE, XDL_MMF_ATOMIC)) != 0)
    I_Error("Error initializing incoming state");

  xdl_write_mmfile(&cur_state, (const void *)state.data, state.size);
  xdl_write_mmfile(&in_state, (const void *)game_state, state_size);
  if (xdl_rabdiff(&cur_state, &in_state, &ecb) != 0)
    I_Error("Error creating delta");

  update_delta_size_average(cur_delta.size);

  set_state_buffer(game_state, state_size);

  xdl_free_mmfile(&cur_state);
  xdl_free_mmfile(&in_state);

  return true;
}

double M_GetAverageDeltaSize(void) {
  return avg_delta_size;
}

/* vi: set cindent et ts=2 sw=2: */

