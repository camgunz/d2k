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

#include "doomtype.h"
#include "lprintf.h"
#include "m_buf.h"
#include "m_swap.h"

union short_bytes { byte b[2]; short s; };
union int_bytes   { byte b[4]; int i; };
union long_bytes  { byte b[8]; int_64_t l; };

static void check_cursor(buf_t *buf) {
  if (buf->cursor > buf->size)
    buf->size = buf->cursor;
}

void M_BufferInit(buf_t *buf) {
  buf->size     = 0;
  buf->capacity = 0;
  buf->cursor   = 0;
  buf->data     = NULL;
}

void M_BufferInitWithCapacity(buf_t *buf, size_t capacity) {
  M_BufferInit(buf);
  M_BufferEnsureTotalCapacity(buf, capacity);
}

size_t M_BufferGetCapacity(buf_t *buf) {
  return buf->capacity;
}

size_t M_BufferGetSize(buf_t *buf) {
  return buf->size;
}

size_t M_BufferGetCursor(buf_t *buf) {
  return buf->cursor;
}

char* M_BufferGetData(buf_t *buf) {
  return buf->data + buf->cursor;
}

void M_BufferEnsureCapacity(buf_t *buf, size_t capacity) {
  if (buf->capacity < buf->cursor + capacity) {
    size_t old_capacity = buf->capacity;

    buf->capacity = buf->cursor + capacity;
    buf->data = realloc(buf->data, buf->capacity * sizeof(byte));

    if (buf->data == NULL)
      I_Error("M_BufferEnsureCapacity: Allocating buffer data failed");

    memset(buf->data + old_capacity, 0, buf->capacity - old_capacity);
  }
}

void M_BufferEnsureTotalCapacity(buf_t *buf, size_t capacity) {
  if (buf->capacity < capacity) {
    size_t old_capacity = buf->capacity;

    buf->capacity = capacity;
    buf->data = realloc(buf->data, buf->capacity * sizeof(byte));

    if (buf->data == NULL)
      I_Error("M_BufferEnsureCapacity: Allocating buffer data failed");

    memset(buf->data + old_capacity, 0, buf->capacity - old_capacity);
  }
}

void M_BufferCopy(buf_t *dst, buf_t *src) {
  M_BufferEnsureTotalCapacity(dst, src->size);
  M_BufferClear(dst);
  M_BufferWrite(dst, src->data, src->size);
}

void M_BufferSetData(buf_t *buf, void *data, size_t size) {
  M_BufferClear(buf);
  M_BufferWrite(buf, data, size);
}

void M_BufferSetString(buf_t *buf, char *data, size_t length) {
  M_BufferClear(buf);
  M_BufferWriteString(buf, data, length);
}

dboolean M_BufferSetFile(buf_t *buf, const char *filename) {
  FILE *fp = NULL;
  size_t length = 0;
  size_t bytes_read = 0;
  dboolean out = false;

  if ((fp = fopen(filename, "rb")) == NULL)
    return false;

  fseek(fp, 0, SEEK_END);
  length = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  M_BufferClear(buf);
  M_BufferEnsureTotalCapacity(buf, length);

  if (fread(buf->data, sizeof(byte), length - bytes_read, fp) == length) {
    buf->cursor = length;
    buf->size = length;
    out = true;
  }
  else {
    M_BufferClear(buf);
    out = false;
  }

  fclose(fp);
  return out;
}

dboolean M_BufferSeek(buf_t *buf, size_t pos) {
  if (pos > buf->size)
    return false;

  buf->cursor = pos;
  return true;
}

dboolean M_BufferSeekBackward(buf_t *buf, size_t count) {
  if (count > buf->cursor)
    return false;

  buf->cursor -= count;
  return true;
}

dboolean M_BufferSeekForward(buf_t *buf, size_t count) {
  if (buf->cursor + count > buf->size)
    return false;

  buf->cursor += count;
  return true;
}

byte M_BufferPeek(buf_t *buf) {
  return *(buf->data + buf->cursor);
}

void M_BufferWrite(buf_t *buf, void *data, size_t size) {
  M_BufferEnsureCapacity(buf, size);
  memcpy(buf->data + buf->cursor, data, size);
  buf->cursor += size;

  check_cursor(buf);
}

void M_BufferWriteByte(buf_t *buf, byte b) {
  M_BufferWriteBytes(buf, &b, 1);
}

void M_BufferWriteBytes(buf_t *buf, void *data, size_t count) {
  M_BufferWrite(buf, data, count * sizeof(byte));
}

void M_BufferWriteShort(buf_t *buf, short s) {
  M_BufferWriteShorts(buf, &s, 1);
}

void M_BufferWriteShorts(buf_t *buf, void *data, size_t count) {
  short *shorts = (short *)data;

  M_BufferEnsureCapacity(buf, count * sizeof(short));

  for (int i = 0; i < count; i++) {
    union short_bytes sb;

    sb.s = doom_l16(shorts[i]);
    M_BufferWriteBytes(buf, sb.b, 2);
  }
}

void M_BufferWriteInt(buf_t *buf, int i) {
  M_BufferWriteInts(buf, &i, 1);
}

void M_BufferWriteInts(buf_t *buf, void *data, size_t count) {
  int *ints = (int *)data;

  M_BufferEnsureCapacity(buf, count * sizeof(int));

  for (int i = 0; i < count; i++) {
    union int_bytes ib;

    ib.i = doom_l32(ints[i]);
    M_BufferWriteBytes(buf, ib.b, 4);
  }
}

void M_BufferWriteLong(buf_t *buf, int_64_t l) {
  M_BufferWriteLongs(buf, &l, 1);
}

void M_BufferWriteLongs(buf_t *buf, void *data, size_t count) {
  int_64_t *longs = (int_64_t *)data;

  M_BufferEnsureCapacity(buf, count * sizeof(int_64_t));

  for (int i = 0; i < count; i++) {
    union long_bytes lb;

    lb.l = doom_l64(longs[i]);
    M_BufferWriteBytes(buf, lb.b, 8);
  }
}

void M_BufferWriteString(buf_t *buf, char *data, size_t length) {
  size_t size = length + 1;

  M_BufferEnsureCapacity(buf, size);
  strncpy(buf->data + buf->cursor, data, size);
  buf->cursor += size;

  check_cursor(buf);
}

void M_BufferWriteZeros(buf_t *buf, size_t count) {
  M_BufferEnsureCapacity(buf, count);

  for (int i = 0; i < count; i++)
    buf->data[buf->cursor++] = 0;

  check_cursor(buf);
}

dboolean M_BufferEqualsString(buf_t *buf, const char *s) {
  if (strncmp(buf->data + buf->cursor, s, buf->size - buf->cursor) == 0)
    return true;

  return false;
}

dboolean M_BufferEqualsData(buf_t *buf, const void *d, size_t size) {
  if (buf->cursor + size >= buf->size)
    return false;

  if (memcmp(buf->data + buf->cursor, d, size) == 0)
    return true;

  return false;
}

dboolean M_BufferRead(buf_t *buf, void *data, size_t size) {
  if (buf->cursor + size >= buf->size)
    return false;

  memcpy(data, buf->data + buf->cursor, size);

  buf->cursor += size;
  return true;
}

dboolean M_BufferReadByte(buf_t *buf, byte *b) {
  return M_BufferReadBytes(buf, b, 1);
}

dboolean M_BufferReadBytes(buf_t *buf, byte *b, size_t count) {
  return M_BufferRead(buf, b, count * sizeof(byte));
}

dboolean M_BufferReadShort(buf_t *buf, short *s) {
  return M_BufferReadShorts(buf, s, 1);
}

dboolean M_BufferReadShorts(buf_t *buf, short *s, size_t count) {
  return M_BufferRead(buf, s, count * sizeof(short));
}

dboolean M_BufferReadInt(buf_t *buf, int *i) {
  return M_BufferReadInts(buf, i, 1);
}

dboolean M_BufferReadInts(buf_t *buf, int *i, size_t count) {
  return M_BufferRead(buf, i, count * sizeof(int));
}

dboolean M_BufferReadLong(buf_t *buf, int_64_t *l) {
  return M_BufferReadLongs(buf, l, 1);
}

dboolean M_BufferReadLongs(buf_t *buf, int_64_t *l, size_t count) {
  return M_BufferRead(buf, l, count * sizeof(int_64_t));
}

dboolean M_BufferReadString(buf_t *buf, char *s, size_t length) {
  return M_BufferRead(buf, s, length);
}

dboolean M_BufferReadStringDup(buf_t *buf, char **s) {
  char *d = buf->data + buf->cursor;
  size_t length = strlen(d);

  if (buf->cursor + length >= buf->size)
    return false;

  (*s) = strdup(buf->data + buf->cursor);
  return true;
}

dboolean M_BufferCopyString(buf_t *dst, buf_t *src) {
  char *s = src->data + src->cursor;
  size_t length = strlen(s);

  if (src->cursor + length >= src->size)
    return false;

  M_BufferWriteString(dst, s, length);
  return true;
}

void M_BufferCompact(buf_t *buf) {
  if (buf->size < buf->capacity) {
    char *new_buf = calloc(buf->size, sizeof(byte));

    if (buf->data == NULL)
      I_Error("M_BufferCompact: Allocating new buffer data failed");

    memcpy(new_buf, buf->data, buf->size);
    free(buf->data);
    buf->data = new_buf;
    buf->capacity = buf->size;
    if (buf->cursor >= buf->size)
      buf->cursor = buf->size - 1;
  }
}

void M_BufferZero(buf_t *buf) {
  memset(buf->data, 0, buf->capacity);
}

void M_BufferClear(buf_t *buf) {
  buf->size = 0;
  buf->cursor = 0;
  M_BufferZero(buf);
}

void M_BufferFree(buf_t *buf) {
  free(buf->data);
  memset(buf, 0, sizeof(buf_t));
  buf->data = NULL;
}

void M_BufferPrint(buf_t *buf) {
  printf("Buffer capacity, size and cursor: [%lu, %lu, %lu].\n",
    buf->capacity,
    buf->size,
    buf->cursor
  );

  for (size_t i = 0; i < buf->size; i++) {
    if (((i + 1) * 5) >= 80)
      printf("%4d\n", buf->data[i]);
    else
      printf("%4d ", buf->data[i]);
  }

  printf("\n");
}

/* vi: set et ts=2 sw=2: */

