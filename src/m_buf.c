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
 *  A simple buffer
 *
 *-----------------------------------------------------------------------------
 */

#include "z_zone.h"
#include "l_printf.h"
#include "m_buf.h"

void M_BufferInit(buf_t **buf) {
  (*buf) = calloc(1, sizeof(buf_t));

  if ((*buf) == NULL)
    I_Error("M_BufferInit: Allocating buffer failed");
}

void M_BufferInitWithCapacity(buf_t **buf, size_t size) {
  (*buf) = calloc(1, sizeof(buf_t));

  if ((*buf) == NULL)
    I_Error("M_BufferInitWithCapacity: Allocating buffer failed");

  (*buf)->size = 0;
  (*buf)->capacity = size;
  (*buf)->data = calloc(size, sizeof(byte));
}

void M_BufferCopy(buf_t *dest, buf_t *src) {
  M_BufferEnsureTotalCapacity(dest, src->size);
  M_BufferClear(dest);
  M_BufferAppend(dest, src->data, src->size);
}

void M_BufferSetData(buf_t *buf, byte *data, size_t size) {
  M_BufferClear(buf);
  memcpy(buf->data, data, size);
  buf->size = size;
}

void M_BufferSetString(buf_t *buf, byte *data, size_t length) {
  size_t size = length + 1;

  M_BufferClear(buf);
  M_BufferEnsureCapacity(buf, size);
  memcpy(buf->data, data, size);
  buf->data + length = 0;
  buf->size = size;
}

void M_BufferAppend(buf_t *buf, byte *data, size_t size) {
  M_BufferEnsureCapacity(buf, size);
  memcpy(buf->data + buf->size, data, size);
  buf->size += size;
}

void M_BufferEnsureCapacity(buf_t *buf, size_t size) {
  if (buf->capacity < buf->size + size) {
    size_t old_capacity = buf->capacity;

    buf->capacity = buf->size + size;
    buf->data = realloc(buf->data, buf->capacity * sizeof(byte));

    if (buf->data == NULL)
      I_Error("M_BufferEnsureCapacity: Allocating buffer data failed");

    memset(buf->data + old_capacity, 0, buf->capacity - old_capacity);
  }
}

void M_BufferEnsureTotalCapacity(buf_t *buf, size_t size) {
  if (buf->capacity < size) {
    size_t old_capacity = buf->capacity;

    buf->capacity = size;
    buf->data = realloc(buf->data, buf->capacity * sizeof(byte));

    if (buf->data == NULL)
      I_Error("M_BufferEnsureCapacity: Allocating buffer data failed");

    memset(buf->data + old_capacity, 0, buf->capacity - old_capacity);
  }
}

void M_BufferCompact(buf_t *buf) {
  if (buf->size < buf->capacity) {
    byte *new_buf = calloc(buf->size, sizeof(byte));

    if (buf->data == NULL)
      I_Error("M_BufferCompact: Allocating new buffer data failed");

    memcpy(new_buf, buf->data, buf->size);
    free(buf->data);
    buf->data = new_buf;
    buf->capacity = buf->size;
  }
}

void M_BufferClear(buf_t *buf) {
  buf->size = 0;
  memset(buf->data, 0, buf->capacity);
}

void M_BufferFree(buf_t *buf) {
  buf->size = 0;
  buf->capacity = 0;
  free(buf->data);
}
