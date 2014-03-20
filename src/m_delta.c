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

#include "doomdef.h"
#include "doomtype.h"
#include "lprintf.h"
#include "m_buf.h"
#include "m_delta.h"

extern int gametic;

#define PRINT_DELTA_STATS
#define BLKSIZE 1024

static int write_delta_data(void *priv, mmbuffer_t *mb, int nbuf);

static delta_stats_t delta_stats = {0, 0, 0.0};
static dboolean xdiff_initialized = false;
static dboolean have_initial_state = false;
static buf_t state = { 0, 0, NULL };
static buf_t cur_delta = { 0, 0, NULL };
static xdemitcb_t ecb = { &cur_delta, write_delta_data };

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
  size_t delta_size;
  buf_t *delta = (buf_t *)priv;

  for (delta_size = 0, i = 0; i < nbuf; i++)
    delta_size += mb[i].size;

  M_BufferEnsureTotalSize(delta, delta_size);

  for (i = 0; i < nbuf; i++)
    M_BufferAppend(delta, (byte *)mb[i].ptr, mb[i].size);

  return 0;
}

static void update_delta_size_average(long size) {
  delta_stats.delta_count++;
  delta_stats.total_delta_size += size;
  delta_stats.average_delta_size =
    (double)delta_stats.total_delta_size / delta_stats.delta_count;
}

static void set_state_buffer(byte *game_state, size_t state_size) {
  M_BufferClear(&state);
  M_BufferAppend(&state, game_state, state_size);
  state.size = state_size;
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

static void build_mmfile(mmfile_t *mmf, byte *data, size_t size) {
  if ((xdl_init_mmfile(mmf, BLKSIZE, XDL_MMF_ATOMIC)) != 0)
    I_Error("Error initializing mmfile");

  /* CG 2014/03/16: TODO: Check return value and don't fuck it up */
  xdl_write_mmfile(mmf, ((const void *)data), size);

  /*
   * CG 2014/3/13: Writing to a memory file can set errno to EAGAIN, so clear
   *               it here
   */
  errno = 0;
}

static dboolean check_mmfile_compact(mmfile_t *mmf, mmfile_t *mmc) {
  if (!xdl_mmfile_iscompact(mmf)) {
    printf("Compacting state.\n");
    if (xdl_mmfile_compact(mmf, mmc, BLKSIZE, XDL_MMF_ATOMIC) < 0) {
      perror("");
      I_Error("Error compacting state.\n");
    }
    return false;
  }
  return true;
}

dboolean M_RegisterGameState(byte *game_state, size_t state_size) {
  static int saves = 0;

  mmfile_t cs, ccs, is, cis;
  mmfile_t *csp = &cs, *isp = &is;

#ifdef PRINT_DELTA_STATS
  if (++saves == TICRATE) {
    delta_stats_t *ds = M_GetDeltaStats();

    printf("Delta stats: %lu, %lu, %.2f (%.2f B/s)\n",
      ds->delta_count,
      ds->total_delta_size,
      ds->average_delta_size,
      ds->average_delta_size * TICRATE * 64
    );
    saves = 0;
  }
#endif

  initialize_xdiff();

  if (!have_initial_state) {
    set_state_buffer(game_state, state_size);
    have_initial_state = true;
    return true;
  }

  build_mmfile(&cs, state.data, state.size);
  build_mmfile(&is, game_state, state_size);

  if (xdl_mmfile_cmp(&cs, &is) != 0) {
    if (!check_mmfile_compact(&cs, &ccs))
      csp = &ccs;

    if (!check_mmfile_compact(&is, &cis))
      isp = &cis;

    if (xdl_rabdiff(csp, isp, &ecb) != 0) {
      perror("");
      I_Error("Error generating delta.");
    }

    update_delta_size_average(cur_delta.size);

    set_state_buffer(game_state, state_size);
  }

  xdl_free_mmfile(&cs);
  xdl_free_mmfile(&is);

  return true;
}

delta_stats_t* M_GetDeltaStats(void) {
  return &delta_stats;
}

/* vi: set cindent et ts=2 sw=2: */

