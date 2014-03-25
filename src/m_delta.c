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

#define BLKSIZE 1024

static void* wrap_malloc(void *priv, unsigned int size) {
  return malloc(size);
}

static void wrap_free(void *priv, void *ptr) {
  free(ptr);
}

static void* wrap_realloc(void *priv, void *ptr, unsigned int size) {
  return realloc(ptr, size);
}

static int write_to_buffer(void *priv, mmbuffer_t *mb, int nbuf) {
  int i;
  size_t delta_size;
  buf_t *delta = (buf_t *)priv;

  for (delta_size = 0, i = 0; i < nbuf; i++)
    delta_size += mb[i].size;

  M_BufferClear(delta);
  M_BufferEnsureTotalSize(delta, delta_size);

  for (i = 0; i < nbuf; i++)
    M_BufferAppend(delta, (byte *)mb[i].ptr, mb[i].size);

  return 0;
}

static void build_mmfile(mmfile_t *mmf, byte *data, size_t size) {
  long bytes_written = 0;

  if ((xdl_init_mmfile(mmf, BLKSIZE, XDL_MMF_ATOMIC)) != 0)
    I_Error("Error initializing mmfile");

  while (bytes_written < size) {
    bytes_written += xdl_write_mmfile(
      mmf, ((const void *)(data + bytes_written)), size - bytes_written
    );
  }

  /*
   * CG 2014/3/13: Writing to a memory file can set errno to EAGAIN, so clear
   *               it here
   */
  errno = 0;
}

static mmfile_t* check_mmfile_compact(mmfile_t *mmf, mmfile_t *mmc) {
  if (!xdl_mmfile_iscompact(mmf)) {
    printf("Compacting state.\n");
    if (xdl_mmfile_compact(mmf, mmc, BLKSIZE, XDL_MMF_ATOMIC) < 0) {
      perror("");
      I_Error("Error compacting state.\n");
    }
    return mmc;
  }
  return mmf;
}

void M_InitDeltas(void) {
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

void M_BuildDelta(buf_t *b1, buf_t *b2, buf_t *delta) {
  mmbuffer_t mmb1, mmb2;
  xdemitcb_t ecb;

  ecb.priv = delta;
  ecb.outf = write_delta_data;

  mmb1.ptr = (char *)b1->data;
  mmb1.size = (long)b1->size;

  mmb2.ptr = (char *)b2->data;
  mmb2.size = (long)b2->size;

  if (xdl_rabdiff_mb(&mmb1, &mmb2, &ecb) != 0) {
    perror("");
    I_Error("M_BuildData: Error building delta");
  }

  return true;
}

void M_ApplyDelta(buf_t *b1, buf_t *b2, buf_t *delta) {
  mmfile_t cs, ccs, is, cis;
  mmfile_t *csp = &cs, *isp = &is;
  xdemitcb_t ecb;

  ecb.priv = delta;
  ecb.outf = write_to_buffer;

  build_mmfile(&cs, b1->data, b1->size);
  build_mmfile(&is, b2->data, b2->size);

  csp = check_mmfile_compact(&cs, &ccs);
  isp = check_mmfile_compact(&is, &cis);

  if (xdl_bpatch(csp, isp, &ecb) != 0) {
    perror("");
    I_Error("M_BuildData: Error building delta");
  }

  xdl_free_mmfile(&cs);
  if (csp == &ccs)
    xdl_free_mmfile(&ccs);
  xdl_free_mmfile(&is);
  if (isp == &cis)
    xdl_free_mmfile(&cis);

  return true;
}

/* vi: set et ts=2 sw=2: */

