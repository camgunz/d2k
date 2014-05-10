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

#define BLKSIZE 1024

static dboolean xdiff_initialized = false;

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

  M_BufferEnsureCapacity(delta, delta_size);

  for (i = 0; i < nbuf; i++)
    M_BufferWrite(delta, (char *)mb[i].ptr, mb[i].size);

  return 0;
}

static void build_mmfile(mmfile_t *mmf, char *data, size_t size) {
  char *blk;

  if ((xdl_init_mmfile(mmf, BLKSIZE, XDL_MMF_ATOMIC)) != 0)
    I_Error("Error initializing mmfile");

  blk = xdl_mmfile_writeallocate(mmf, size);
  if (blk == NULL)
    I_Error("Error allocating mmfile buffer");

  memcpy(blk, data, size);
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
  int res = 0;

  ecb.priv = delta;
  ecb.outf = write_to_buffer;

  mmb1.ptr = (char *)b1->data;
  mmb1.size = (long)b1->size;

  mmb2.ptr = (char *)b2->data;
  mmb2.size = (long)b2->size;

  M_BufferClear(delta);

  res = xdl_rabdiff_mb(&mmb1, &mmb2, &ecb);
  if (res != 0) {
    perror("");
    I_Error("M_BuildDelta: Error building delta: %d", res);
  }
}

dboolean M_ApplyDelta(buf_t *b1, buf_t *b2, buf_t *delta) {
  mmfile_t cs, is;
  xdemitcb_t ecb;
  buf_t delta_copy;
  dboolean res = false;

  ecb.priv = b2;
  ecb.outf = write_to_buffer;

  build_mmfile(&cs, b1->data, b1->size);
  build_mmfile(&is, delta->data, delta->size);

  M_BufferInitWithCapacity(&delta_copy, M_BufferGetSize(delta));
  M_BufferCopy(&delta_copy, delta);

  if (xdl_bpatch(&cs, &is, &ecb) == 0) {
    res = true;
  }
  else {
    perror("M_BuildDelta: Error applying delta");
    M_BufferPrint(delta);
    M_BufferPrint(&delta_copy);
    M_BufferPrint(b1);
    M_BufferPrint(b2);
  }

  M_BufferFree(&delta_copy);

  xdl_free_mmfile(&cs);
  xdl_free_mmfile(&is);

  return res;
}

/* vi: set et ts=2 sw=2: */

