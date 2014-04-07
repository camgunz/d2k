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

#ifndef M_BUF_H__
#define M_BUF_H__

typedef struct buf_s {
  size_t capacity;
  size_t size;
  size_t cursor;
  char *data;
} buf_t;

void     M_BufferInit(buf_t *buf);
void     M_BufferInitWithCapacity(buf_t *buf, size_t capacity);

size_t   M_BufferGetCapacity(buf_t *buf);
size_t   M_BufferGetSize(buf_t *buf);
size_t   M_BufferGetCursor(buf_t *buf);
char*    M_BufferGetData(buf_t *buf);

void     M_BufferEnsureCapacity(buf_t *buf, size_t capacity);
void     M_BufferEnsureTotalCapacity(buf_t *buf, size_t capacity);

void     M_BufferCopy(buf_t *dst, buf_t *src);

void     M_BufferSetData(buf_t *buf, void *data, size_t size);
void     M_BufferSetString(buf_t *buf, char *data, size_t length);
dboolean M_BufferSetFile(buf_t *buf, const char *filename);

dboolean M_BufferSeek(buf_t *buf, size_t pos);
dboolean M_BufferSeekBackward(buf_t *buf, size_t count);
dboolean M_BufferSeekForward(buf_t *buf, size_t count);
byte     M_BufferPeek(buf_t *buf);

void     M_BufferWrite(buf_t *buf, void *data, size_t size);
void     M_BufferWriteByte(buf_t *buf, byte b);
void     M_BufferWriteBytes(buf_t *buf, void *data, size_t count);
void     M_BufferWriteShort(buf_t *buf, short s);
void     M_BufferWriteShorts(buf_t *buf, void *data, size_t count);
void     M_BufferWriteInt(buf_t *buf, int i);
void     M_BufferWriteInts(buf_t *buf, void *data, size_t count);
void     M_BufferWriteLong(buf_t *buf, int_64_t l);
void     M_BufferWriteLongs(buf_t *buf, void *data, size_t count);
void     M_BufferWriteString(buf_t *buf, char *data, size_t length);
void     M_BufferWriteZeros(buf_t *buf, size_t count);

dboolean M_BufferEqualsString(buf_t *buf, const char *s);
dboolean M_BufferEqualsData(buf_t *buf, const void *d, size_t size);

dboolean M_BufferRead(buf_t *buf, void *data, size_t size);
dboolean M_BufferReadByte(buf_t *buf, byte *b);
dboolean M_BufferReadBytes(buf_t *buf, byte *b, size_t count);
dboolean M_BufferReadShort(buf_t *buf, short *s);
dboolean M_BufferReadShorts(buf_t *buf, short *s, size_t count);
dboolean M_BufferReadInt(buf_t *buf, int *i);
dboolean M_BufferReadInts(buf_t *buf, int *i, size_t count);
dboolean M_BufferReadLong(buf_t *buf, int_64_t *l);
dboolean M_BufferReadLongs(buf_t *buf, int_64_t *l, size_t count);
dboolean M_BufferReadString(buf_t *buf, char *s, size_t length);
dboolean M_BufferReadStringDup(buf_t *buf, char **s);
dboolean M_BufferCopyString(buf_t *dst, buf_t *src);

void     M_BufferCompact(buf_t *buf);
void     M_BufferZero(buf_t *buf);
void     M_BufferClear(buf_t *buf);
void     M_BufferFree(buf_t *buf);

#endif

/* vi: set et ts=2 sw=2: */

