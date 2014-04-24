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
 *  A packed buffer
 *
 *-----------------------------------------------------------------------------
 */

#ifndef M_PBUF_H__
#define M_PBUF_H__

struct msgpack_packer;
struct msgpack_unpacker;
struct msgpack_unpacked;

typedef enum {
  PBUF_MODE_NONE,
  PBUF_MODE_READ,
  PBUF_MODE_WRITE
} pbuf_mode_t;

typedef struct pbuf_s {
  pbuf_mode_t mode;
  buf_t buf;
  struct msgpack_packer *packer;
  struct msgpack_unpacker *unpacker;
  struct msgpack_unpacked *result;
} pbuf_t;

void M_PBufInit(pbuf_t *pbuf, pbuf_mode_t mode);
void M_PBufInitWithCapacity(pbuf_t *pbuf, size_t capacity);

size_t M_PBufGetCapacity(pbuf_t *pbuf);
size_t M_PBufGetSize(pbuf_t *pbuf);
size_t M_PBufGetCursor(pbuf_t *pbuf);
buf_t* M_PBufGetBuffer(pbuf_t *pbuf);
char*  M_PBufGetData(pbuf_t *pbuf);

void M_PBufEnsureCapacity(pbuf_t *pbuf, size_t capacity);
void M_PBufEnsureTotalCapacity(pbuf_t *pbuf, size_t capacity);

void     M_PBufCopy(pbuf_t *dst, pbuf_t *src);
void     M_PBufSetData(pbuf_t *pbuf, void *data, size_t size);
dboolean M_PBufSetFile(pbuf_t *pbuf, const char *filename);

void M_PBufWriteChar(pbuf_t *pbuf, char c);
void M_PBufWriteCharArray(pbuf_t *pbuf, buf_t *chars);
void M_PBufWriteUChar(pbuf_t *pbuf, unsigned char c);
void M_PBufWriteUCharArray(pbuf_t *pbuf, buf_t *uchars);
void M_PBufWriteShort(pbuf_t *pbuf, short s);
void M_PBufWriteShortArray(pbuf_t *pbuf, buf_t *shorts);
void M_PBufWriteUShort(pbuf_t *pbuf, unsigned short s);
void M_PBufWriteUShortArray(pbuf_t *pbuf, buf_t *ushorts);
void M_PBufWriteInt(pbuf_t *pbuf, int i);
void M_PBufWriteIntArray(pbuf_t *pbuf, buf_t *ints);
void M_PBufWriteUInt(pbuf_t *pbuf, unsigned int i);
void M_PBufWriteUIntArray(pbuf_t *pbuf, buf_t *uints);
void M_PBufWriteLong(pbuf_t *pbuf, int_64_t l);
void M_PBufWriteLongArray(pbuf_t *pbuf, buf_t *longs);
void M_PBufWriteULong(pbuf_t *pbuf, uint_64_t l);
void M_PBufWriteULongArray(pbuf_t *pbuf, buf_t *ulongs);
void M_PBufWriteDouble(pbuf_t *pbuf, double d);
void M_PBufWriteDoubleArray(pbuf_t *pbuf, buf_t *doubles);
void M_PBufWriteBool(pbuf_t *pbuf, dboolean b);
void M_PBufWriteBoolArray(pbuf_t *pbuf, buf_t *bools);
void M_PBufWriteNil(pbuf_t *pbuf);
void M_PBufWriteArray(pbuf_t *pbuf, unsigned int array_size);
void M_PBufWriteMap(pbuf_t *pbuf, unsigned int map_size);
void M_PBufWriteBytes(pbuf_t *pbuf, void *data, size_t size);
void M_PBufWriteString(pbuf_t *pbuf, char *data, size_t length);
void M_PBufWriteStringArray(pbuf_t *pbuf, obuf_t *obuf);

dboolean M_PBufReadChar(pbuf_t *pbuf, char *c);
dboolean M_PBufReadCharArray(pbuf_t *pbuf, buf_t *chars, size_t limit);
dboolean M_PBufReadUChar(pbuf_t *pbuf, unsigned char *c);
dboolean M_PBufReadUCharArray(pbuf_t *pbuf, buf_t *uchars, size_t limit);
dboolean M_PBufReadShort(pbuf_t *pbuf, short *s);
dboolean M_PBufReadShortArray(pbuf_t *pbuf, buf_t *shorts, size_t limit);
dboolean M_PBufReadUShort(pbuf_t *pbuf, unsigned short *s);
dboolean M_PBufReadUShortArray(pbuf_t *pbuf, buf_t *ushorts,
                                             size_t limit);
dboolean M_PBufReadInt(pbuf_t *pbuf, int *i);
dboolean M_PBufReadIntArray(pbuf_t *pbuf, buf_t *ints, size_t limit);
dboolean M_PBufReadUInt(pbuf_t *pbuf, unsigned int *i);
dboolean M_PBufReadUIntArray(pbuf_t *pbuf, buf_t *uints, size_t limit);
dboolean M_PBufReadLong(pbuf_t *pbuf, int_64_t *l);
dboolean M_PBufReadLongArray(pbuf_t *pbuf, buf_t *longs, size_t limit);
dboolean M_PBufReadULong(pbuf_t *pbuf, uint_64_t *l);
dboolean M_PBufReadULongArray(pbuf_t *pbuf, buf_t *ulongs, size_t limit);
dboolean M_PBufReadDouble(pbuf_t *pbuf, double *d);
dboolean M_PBufReadDoubleArray(pbuf_t *pbuf, buf_t *doubles,
                                             size_t limit);
dboolean M_PBufReadBool(pbuf_t *pbuf, dboolean *b);
dboolean M_PBufReadBoolArray(pbuf_t *pbuf, buf_t *bools, size_t limit);
dboolean M_PBufReadNil(pbuf_t *pbuf);
dboolean M_PBufReadArray(pbuf_t *pbuf, unsigned int *array_size);
dboolean M_PBufReadMap(pbuf_t *pbuf, unsigned int *map_size);
dboolean M_PBufReadBytes(pbuf_t *pbuf, buf_t *buf);
dboolean M_PBufReadString(pbuf_t *pbuf, buf_t *buf, size_t limit);
dboolean M_PBufReadStringArray(pbuf_t *pbuf, obuf_t *obuf,
                                             size_t string_count_limit,
                                             size_t string_size_limit);
dboolean M_PBufAtEOF(pbuf_t *pbuf);

void M_PBufCompact(pbuf_t *pbuf);
void M_PBufZero(pbuf_t *pbuf);
void M_PBufClear(pbuf_t *pbuf);
void M_PBufFree(pbuf_t *pbuf);

#endif

/* vi: set et ts=2 sw=2: */

