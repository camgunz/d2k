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

#ifndef M_PBUF_H__
#define M_PBUF_H__

typedef struct pbuf_s {
  buf_t buf;
  cmp_ctx_t cmp;
} pbuf_t;

void M_PBufInit(pbuf_t *pbuf);
void M_PBufInitWithCapacity(pbuf_t *pbuf, size_t capacity);

size_t M_PBufGetCapacity(pbuf_t *pbuf);
size_t M_PBufGetSize(pbuf_t *pbuf);
size_t M_PBufGetCursor(pbuf_t *pbuf);
buf_t* M_PBufGetBuffer(pbuf_t *pbuf);
char*  M_PBufGetData(pbuf_t *pbuf);
char*  M_PBufGetDataAtCursor(pbuf_t *pbuf);

void M_PBufEnsureCapacity(pbuf_t *pbuf, size_t capacity);
void M_PBufEnsureTotalCapacity(pbuf_t *pbuf, size_t capacity);

void     M_PBufCopy(pbuf_t *dst, pbuf_t *src);
void     M_PBufSetData(pbuf_t *pbuf, void *data, size_t size);
dboolean M_PBufSetFile(pbuf_t *pbuf, const char *filename);

dboolean M_PBufSeek(pbuf_t *pbuf, size_t pos);
dboolean M_PBufSeekBackward(pbuf_t *pbuf, size_t count);
dboolean M_PBufSeekForward(pbuf_t *pbuf, size_t count);

dboolean M_PBufWriteChar(pbuf_t *pbuf, char c);
dboolean M_PBufWriteCharArray(pbuf_t *pbuf, buf_t *chars);
dboolean M_PBufWriteUChar(pbuf_t *pbuf, unsigned char c);
dboolean M_PBufWriteUCharArray(pbuf_t *pbuf, buf_t *uchars);
dboolean M_PBufWriteShort(pbuf_t *pbuf, short s);
dboolean M_PBufWriteShortArray(pbuf_t *pbuf, buf_t *shorts);
dboolean M_PBufWriteUShort(pbuf_t *pbuf, unsigned short s);
dboolean M_PBufWriteUShortArray(pbuf_t *pbuf, buf_t *ushorts);
dboolean M_PBufWriteInt(pbuf_t *pbuf, int i);
dboolean M_PBufWriteIntArray(pbuf_t *pbuf, buf_t *ints);
dboolean M_PBufWriteUInt(pbuf_t *pbuf, unsigned int i);
dboolean M_PBufWriteUIntArray(pbuf_t *pbuf, buf_t *uints);
dboolean M_PBufWriteLong(pbuf_t *pbuf, int_64_t l);
dboolean M_PBufWriteLongArray(pbuf_t *pbuf, buf_t *longs);
dboolean M_PBufWriteULong(pbuf_t *pbuf, uint_64_t l);
dboolean M_PBufWriteULongArray(pbuf_t *pbuf, buf_t *ulongs);
dboolean M_PBufWriteDouble(pbuf_t *pbuf, double d);
dboolean M_PBufWriteDoubleArray(pbuf_t *pbuf, buf_t *doubles);
dboolean M_PBufWriteBool(pbuf_t *pbuf, dboolean b);
dboolean M_PBufWriteBoolArray(pbuf_t *pbuf, buf_t *bools);
dboolean M_PBufWriteNil(pbuf_t *pbuf);
dboolean M_PBufWriteArray(pbuf_t *pbuf, unsigned int array_size);
dboolean M_PBufWriteMap(pbuf_t *pbuf, unsigned int map_size);
dboolean M_PBufWriteBytes(pbuf_t *pbuf, const void *data, size_t size);
dboolean M_PBufWriteString(pbuf_t *pbuf, const char *data, size_t length);
dboolean M_PBufWriteStringArray(pbuf_t *pbuf, obuf_t *obuf);

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

const char* M_PBufGetError(pbuf_t *pbuf);

void M_PBufPrint(pbuf_t *pbuf);

#endif

