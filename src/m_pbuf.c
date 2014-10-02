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

#include "doomstat.h"
#include "g_game.h"
#include "lprintf.h"
#include "p_user.h"

static bool buf_read(cmp_ctx_t *ctx, void *data, size_t limit) {
  buf_t *buf = (buf_t *)ctx->buf;
  size_t pos = M_BufferGetCursor(buf);
  bool res = M_BufferRead(buf, data, limit);

  if (res && (M_BufferGetCursor(buf) - pos) == limit)
    return true;

  return false;
}

static size_t buf_write(cmp_ctx_t *ctx, const void *data, size_t count) {
  buf_t *buf = (buf_t *)ctx->buf;
  size_t pos = M_BufferGetCursor(buf);

  M_BufferWrite(buf, data, count);

  return M_BufferGetCursor(buf) - pos;
}

void M_PBufInit(pbuf_t *pbuf) {
  M_BufferInit(&pbuf->buf);
  cmp_init(&pbuf->cmp, &pbuf->buf, &buf_read, &buf_write);
}

void M_PBufInitWithCapacity(pbuf_t *pbuf, size_t capacity) {
  M_BufferInitWithCapacity(&pbuf->buf, capacity);
  cmp_init(&pbuf->cmp, &pbuf->buf, &buf_read, &buf_write);
}

pbuf_t* M_PBufNew(void) {
  pbuf_t *pbuf = malloc(sizeof(pbuf_t));

  if (pbuf == NULL)
    I_Error("M_PBufNew: Error allocating new pbuf");

  M_PBufInit(pbuf);

  return pbuf;
}

pbuf_t* M_PBufNewWithCapacity(size_t capacity) {
  pbuf_t *pbuf = malloc(sizeof(pbuf_t));

  if (pbuf == NULL)
    I_Error("M_PBufNewWithCapacity: Error allocating new pbuf");

  M_PBufInitWithCapacity(pbuf, capacity);

  return pbuf;
}

size_t M_PBufGetCapacity(pbuf_t *pbuf) {
  return M_BufferGetCapacity(&pbuf->buf);
}

size_t M_PBufGetSize(pbuf_t *pbuf) {
  return M_BufferGetSize(&pbuf->buf);
}

size_t M_PBufGetCursor(pbuf_t *pbuf) {
  return M_BufferGetCursor(&pbuf->buf);
}

buf_t* M_PBufGetBuffer(pbuf_t *pbuf) {
  return &pbuf->buf;
}

char* M_PBufGetData(pbuf_t *pbuf) {
  return M_BufferGetData(&pbuf->buf);
}

char* M_PBufGetDataAtCursor(pbuf_t *pbuf) {
  return M_BufferGetDataAtCursor(&pbuf->buf);
}

void M_PBufEnsureCapacity(pbuf_t *pbuf, size_t capacity) {
  M_BufferEnsureCapacity(&pbuf->buf, capacity);
}

void M_PBufEnsureTotalCapacity(pbuf_t *pbuf, size_t capacity) {
  M_BufferEnsureTotalCapacity(&pbuf->buf, capacity);
}

void M_PBufCopy(pbuf_t *dst, pbuf_t *src) {
  M_PBufSetData(dst, M_PBufGetData(src), M_PBufGetSize(src));
}

void M_PBufSetData(pbuf_t *pbuf, void *data, size_t size) {
  return M_BufferSetData(&pbuf->buf, data, size);
}

dboolean M_PBufSetFile(pbuf_t *pbuf, const char *filename) {
  return M_BufferSetFile(&pbuf->buf, filename);
}

dboolean M_PBufSeek(pbuf_t *pbuf, size_t pos) {
  return M_BufferSeek(&pbuf->buf, pos);
}

dboolean M_PBufSeekBackward(pbuf_t *pbuf, size_t count) {
  return M_BufferSeekBackward(&pbuf->buf, count);
}

dboolean M_PBufSeekForward(pbuf_t *pbuf, size_t count) {
  return M_BufferSeekForward(&pbuf->buf, count);
}

dboolean M_PBufWriteChar(pbuf_t *pbuf, char c) {
  return cmp_write_sint(&pbuf->cmp, c);
}

dboolean M_PBufWriteCharArray(pbuf_t *pbuf, buf_t *chars) {
  size_t count = M_BufferGetSize(chars) / sizeof(char);

  if (!M_PBufWriteArray(pbuf, count))
    return false;

  for (int i = 0; i < count; i++) {
    if (!M_PBufWriteChar(pbuf, ((char *)chars->data)[i]))
      return false;
  }

  return true;
}

dboolean M_PBufWriteUChar(pbuf_t *pbuf, unsigned char c) {
  return cmp_write_uint(&pbuf->cmp, c);
}

dboolean M_PBufWriteUCharArray(pbuf_t *pbuf, buf_t *uchars) {
  size_t count = M_BufferGetSize(uchars) / sizeof(unsigned char);

  if (!M_PBufWriteArray(pbuf, count))
    return false;

  for (int i = 0; i < count; i++) {
    if (!M_PBufWriteUChar(pbuf, ((unsigned char *)uchars->data)[i]))
      return false;
  }

  return true;
}

dboolean M_PBufWriteShort(pbuf_t *pbuf, short s) {
  return cmp_write_sint(&pbuf->cmp, s);
}

dboolean M_PBufWriteShortArray(pbuf_t *pbuf, buf_t *shorts) {
  size_t count = M_BufferGetSize(shorts) / sizeof(short);

  if (!M_PBufWriteArray(pbuf, count))
    return false;

  for (int i = 0; i < count; i++) {
    if (!M_PBufWriteShort(pbuf, ((short *)shorts->data)[i]))
      return false;
  }

  return true;
}

dboolean M_PBufWriteUShort(pbuf_t *pbuf, unsigned short s) {
  return cmp_write_uint(&pbuf->cmp, s);
}

dboolean M_PBufWriteUShortArray(pbuf_t *pbuf, buf_t *ushorts) {
  size_t count = M_BufferGetSize(ushorts) / sizeof(unsigned short);

  if (!M_PBufWriteArray(pbuf, count))
    return false;

  for (int i = 0; i < count; i++) {
    if (!M_PBufWriteUShort(pbuf, ((unsigned short *)ushorts->data)[i]))
      return false;
  }

  return true;
}

dboolean M_PBufWriteInt(pbuf_t *pbuf, int i) {
  return cmp_write_sint(&pbuf->cmp, i);
}

dboolean M_PBufWriteIntArray(pbuf_t *pbuf, buf_t *ints) {
  size_t count = M_BufferGetSize(ints) / sizeof(int);

  if (!M_PBufWriteArray(pbuf, count))
    return false;

  for (int i = 0; i < count; i++) {
    if (!M_PBufWriteInt(pbuf, ((int *)ints->data)[i]))
      return false;
  }

  return true;
}

dboolean M_PBufWriteUInt(pbuf_t *pbuf, unsigned int i) {
  return cmp_write_uint(&pbuf->cmp, i);
}

dboolean M_PBufWriteUIntArray(pbuf_t *pbuf, buf_t *uints) {
  size_t count = M_BufferGetSize(uints) / sizeof(unsigned int);

  if (!M_PBufWriteArray(pbuf, count))
    return false;

  for (int i = 0; i < count; i++) {
    if (!M_PBufWriteUInt(pbuf, ((unsigned int *)uints->data)[i]))
      return false;
  }

  return true;
}

dboolean M_PBufWriteLong(pbuf_t *pbuf, int_64_t l) {
  return cmp_write_uint(&pbuf->cmp, l);
}

dboolean M_PBufWriteLongArray(pbuf_t *pbuf, buf_t *longs) {
  size_t count = M_BufferGetSize(longs) / sizeof(int_64_t);

  if (!M_PBufWriteArray(pbuf, count))
    return false;

  for (int i = 0; i < count; i++) {
    if (!M_PBufWriteLong(pbuf, ((int_64_t *)longs->data)[i]))
      return false;
  }

  return true;
}

dboolean M_PBufWriteULong(pbuf_t *pbuf, uint_64_t l) {
  return cmp_write_uint(&pbuf->cmp, l);
}

dboolean M_PBufWriteULongArray(pbuf_t *pbuf, buf_t *ulongs) {
  size_t count = M_BufferGetSize(ulongs) / sizeof(uint_64_t);

  if (!M_PBufWriteArray(pbuf, count))
    return false;

  for (int i = 0; i < count; i++) {
    if (!M_PBufWriteULong(pbuf, ((uint_64_t *)ulongs->data)[i]))
      return false;
  }

  return true;
}

dboolean M_PBufWriteFloat(pbuf_t *pbuf, float f) {
  return cmp_write_float(&pbuf->cmp, f);
}

dboolean M_PBufWriteFloatArray(pbuf_t *pbuf, buf_t *floats) {
  size_t count = M_BufferGetSize(floats) / sizeof(float);

  if (!M_PBufWriteArray(pbuf, count))
    return false;

  for (int i = 0; i < count; i++) {
    if (!M_PBufWriteFloat(pbuf, ((float *)floats->data)[i]))
      return false;
  }

  return true;
}

dboolean M_PBufWriteDouble(pbuf_t *pbuf, double d) {
  return cmp_write_double(&pbuf->cmp, d);
}

dboolean M_PBufWriteDoubleArray(pbuf_t *pbuf, buf_t *doubles) {
  size_t count = M_BufferGetSize(doubles) / sizeof(double);

  if (!M_PBufWriteArray(pbuf, count))
    return false;

  for (int i = 0; i < count; i++) {
    if (!M_PBufWriteDouble(pbuf, ((double *)doubles->data)[i]))
      return false;
  }

  return true;
}

dboolean M_PBufWriteBool(pbuf_t *pbuf, dboolean b) {
  if (b)
    return cmp_write_true(&pbuf->cmp);

  return cmp_write_false(&pbuf->cmp);
}

dboolean M_PBufWriteBoolArray(pbuf_t *pbuf, buf_t *bools) {
  size_t count = M_BufferGetSize(bools) / sizeof(dboolean);

  if (!M_PBufWriteArray(pbuf, count))
    return false;

  for (int i = 0; i < count; i++) {
    if (!M_PBufWriteBool(pbuf, ((dboolean *)bools->data)[i]))
      return false;
  }

  return true;
}

dboolean M_PBufWriteNil(pbuf_t *pbuf) {
  return cmp_write_nil(&pbuf->cmp);
}

dboolean M_PBufWriteArray(pbuf_t *pbuf, unsigned int array_size) {
  return cmp_write_array(&pbuf->cmp, array_size);
}

dboolean M_PBufWriteMap(pbuf_t *pbuf, unsigned int map_size) {
  return cmp_write_map(&pbuf->cmp, map_size);
}

dboolean M_PBufWriteBytes(pbuf_t *pbuf, const void *data, size_t size) {
  return cmp_write_bin(&pbuf->cmp, data, size);
}

dboolean M_PBufWriteString(pbuf_t *pbuf, const char *data, size_t length) {
  return cmp_write_str(&pbuf->cmp, data, length);
}

dboolean M_PBufWriteStringArray(pbuf_t *pbuf, GPtrArray *strings) {
  if (!M_PBufWriteArray(pbuf, strings->len))
    return false;

  for (unsigned int i = 0; i < strings->len; i++) {
    char *s = g_ptr_array_index(strings, i);
    size_t length = strlen(s);

    if (!M_PBufWriteString(pbuf, s, length))
      return false;
  }

  return true;
}

dboolean M_PBufReadChar(pbuf_t *pbuf, char *c) {
  return cmp_read_char(&pbuf->cmp, (signed char *)c);
}

dboolean M_PBufReadCharArray(pbuf_t *pbuf, buf_t *chars, size_t limit) {
  char c = 0;
  unsigned int array_size = 0;

  if (!M_PBufReadArray(pbuf, &array_size))
    return false;

  if (array_size > limit)
    return false;

  M_BufferClear(chars);
  M_BufferEnsureTotalCapacity(chars, array_size * sizeof(char));

  while (array_size--) {
    if (!M_PBufReadChar(pbuf, &c))
      return false;

    M_BufferWriteChar(chars, c);
  }

  return true;
}

dboolean M_PBufReadUChar(pbuf_t *pbuf, unsigned char *c) {
  return cmp_read_uchar(&pbuf->cmp, c);
}

dboolean M_PBufReadUCharArray(pbuf_t *pbuf, buf_t *uchars, size_t limit) {
  unsigned char c = 0;
  unsigned int array_size = 0;

  if (!M_PBufReadArray(pbuf, &array_size))
    return false;

  if (array_size > limit)
    return false;

  M_BufferClear(uchars);
  M_BufferEnsureTotalCapacity(uchars, array_size * sizeof(unsigned char));

  while (array_size--) {
    if (!M_PBufReadUChar(pbuf, &c))
      return false;

    M_BufferWriteUChar(uchars, c);
  }

  return true;
}

dboolean M_PBufReadShort(pbuf_t *pbuf, short *s) {
  return cmp_read_short(&pbuf->cmp, s);
}

dboolean M_PBufReadShortArray(pbuf_t *pbuf, buf_t *shorts, size_t limit) {
  short s = 0;
  unsigned int array_size = 0;

  if (!M_PBufReadArray(pbuf, &array_size))
    return false;

  if (array_size > limit)
    return false;

  M_BufferClear(shorts);
  M_BufferEnsureTotalCapacity(shorts, array_size * sizeof(short));

  while (array_size--) {
    if (!M_PBufReadShort(pbuf, &s))
      return false;

    M_BufferWriteShort(shorts, s);
  }

  return true;
}

dboolean M_PBufReadUShort(pbuf_t *pbuf, unsigned short *s) {
  return cmp_read_ushort(&pbuf->cmp, s);
}

dboolean M_PBufReadUShortArray(pbuf_t *pbuf, buf_t *ushorts, size_t limit) {
  unsigned short s = 0;
  unsigned int array_size = 0;

  if (!M_PBufReadArray(pbuf, &array_size))
    return false;

  if (array_size > limit)
    return false;

  M_BufferClear(ushorts);
  M_BufferEnsureTotalCapacity(ushorts, array_size * sizeof(unsigned short));

  while (array_size--) {
    if (!M_PBufReadUShort(pbuf, &s))
      return false;

    M_BufferWriteUShort(ushorts, s);
  }

  return true;
}

dboolean M_PBufReadInt(pbuf_t *pbuf, int *i) {
  return cmp_read_int(&pbuf->cmp, i);
}

dboolean M_PBufReadIntArray(pbuf_t *pbuf, buf_t *ints, size_t limit) {
  int i = 0;
  unsigned int array_size = 0;

  if (!M_PBufReadArray(pbuf, &array_size))
    return false;

  if (array_size > limit)
    return false;

  M_BufferClear(ints);
  M_BufferEnsureTotalCapacity(ints, array_size * sizeof(int));

  while (array_size--) {
    if (!M_PBufReadInt(pbuf, &i))
      return false;

    M_BufferWriteInt(ints, i);
  }

  return true;
}

dboolean M_PBufReadUInt(pbuf_t *pbuf, unsigned int *i) {
  return cmp_read_uint(&pbuf->cmp, i);
}

dboolean M_PBufReadUIntArray(pbuf_t *pbuf, buf_t *uints, size_t limit) {
  unsigned int i = 0;
  unsigned int array_size = 0;

  if (!M_PBufReadArray(pbuf, &array_size))
    return false;

  if (array_size > limit)
    return false;

  M_BufferClear(uints);
  M_BufferEnsureTotalCapacity(uints, array_size * sizeof(unsigned int));

  while (array_size--) {
    if (!M_PBufReadUInt(pbuf, &i))
      return false;

    M_BufferWriteUInt(uints, i);
  }

  return true;
}

dboolean M_PBufReadLong(pbuf_t *pbuf, int_64_t *l) {
  return cmp_read_long(&pbuf->cmp, l);
}

dboolean M_PBufReadLongArray(pbuf_t *pbuf, buf_t *longs, size_t limit) {
  int_64_t l = 0;
  unsigned int array_size = 0;

  if (!M_PBufReadArray(pbuf, &array_size))
    return false;

  if (array_size > limit)
    return false;

  M_BufferClear(longs);
  M_BufferEnsureTotalCapacity(longs, array_size * sizeof(int_64_t));

  while (array_size--) {
    if (!M_PBufReadLong(pbuf, &l))
      return false;

    M_BufferWriteLong(longs, l);
  }

  return true;
}

dboolean M_PBufReadULong(pbuf_t *pbuf, uint_64_t *l) {
  return cmp_read_ulong(&pbuf->cmp, l);
}

dboolean M_PBufReadULongArray(pbuf_t *pbuf, buf_t *ulongs, size_t limit) {
  uint_64_t l = 0;
  unsigned int array_size = 0;

  if (!M_PBufReadArray(pbuf, &array_size))
    return false;

  if (array_size > limit)
    return false;

  M_BufferClear(ulongs);
  M_BufferEnsureTotalCapacity(ulongs, array_size * sizeof(uint_64_t));

  while (array_size--) {
    if (!M_PBufReadULong(pbuf, &l))
      return false;

    M_BufferWriteULong(ulongs, l);
  }

  return true;
}

dboolean M_PBufReadFloat(pbuf_t *pbuf, float *f) {
  return cmp_read_float(&pbuf->cmp, f);
}

dboolean M_PBufReadFloatArray(pbuf_t *pbuf, buf_t *floats, size_t limit) {
  float f = 0.0f;
  unsigned int array_size = 0;

  if (!M_PBufReadArray(pbuf, &array_size))
    return false;

  if (array_size > limit)
    return false;

  M_BufferClear(floats);
  M_BufferEnsureTotalCapacity(floats, array_size * sizeof(float));

  while (array_size--) {
    if (!M_PBufReadFloat(pbuf, &f))
      return false;

    M_BufferWriteFloat(floats, f);
  }

  return true;
}

dboolean M_PBufReadDouble(pbuf_t *pbuf, double *d) {
  return cmp_read_double(&pbuf->cmp, d);
}

dboolean M_PBufReadDoubleArray(pbuf_t *pbuf, buf_t *doubles, size_t limit) {
  double d = 0;
  unsigned int array_size = 0;

  if (!M_PBufReadArray(pbuf, &array_size))
    return false;

  if (array_size > limit)
    return false;

  M_BufferClear(doubles);
  M_BufferEnsureTotalCapacity(doubles, array_size * sizeof(double));

  while (array_size--) {
    if (!M_PBufReadDouble(pbuf, &d))
      return false;

    M_BufferWriteDouble(doubles, d);
  }

  return true;
}

dboolean M_PBufReadBool(pbuf_t *pbuf, dboolean *b) {
  uint8_t u8 = 0;

  if (!cmp_read_bool_as_u8(&pbuf->cmp, &u8))
    return false;

  if (u8)
    *b = true;
  else
    *b = false;

  return true;
}

dboolean M_PBufReadBoolArray(pbuf_t *pbuf, buf_t *bools, size_t limit) {
  dboolean b = 0;
  unsigned int array_size = 0;

  if (!M_PBufReadArray(pbuf, &array_size))
    return false;

  if (array_size > limit)
    return false;

  M_BufferClear(bools);
  M_BufferEnsureTotalCapacity(bools, array_size * sizeof(dboolean));

  while (array_size--) {
    if (!M_PBufReadBool(pbuf, &b))
      return false;

    M_BufferWriteBool(bools, b);
  }

  return true;
}

dboolean M_PBufReadNil(pbuf_t *pbuf) {
  return cmp_read_nil(&pbuf->cmp);
}

dboolean M_PBufReadArray(pbuf_t *pbuf, unsigned int *array_size) {
  return cmp_read_array(&pbuf->cmp, array_size);
}

dboolean M_PBufReadMap(pbuf_t *pbuf, unsigned int *map_size) {
  return cmp_read_map(&pbuf->cmp, map_size);
}

dboolean M_PBufReadBytes(pbuf_t *pbuf, buf_t *buf) {
  uint32_t size = 0;

  if (!cmp_read_bin_size(&pbuf->cmp, &size))
    return false;

  /*
   * CG: This is a hack to prevent overallocating like crazy: limit the size to
   *     16MB
   */
  if (size > 0x00FFFFFF) {
    P_Echo(consoleplayer, "M_PBufReadBytes: Tried to read more than 16MB.");
    return false;
  }

  M_BufferWrite(buf, M_PBufGetDataAtCursor(pbuf), size);

  M_BufferSeekForward(&pbuf->buf, size);

  return true;
}

dboolean M_PBufReadString(pbuf_t *pbuf, buf_t *buf, size_t limit) {
  cmp_object_t obj;

  if (!cmp_read_object(&pbuf->cmp, &obj)) {
    P_Printf(consoleplayer,
      "M_PBufReadString: Error reading from packed buffer: %s\n",
      cmp_strerror(&pbuf->cmp)
    );
    return false;
  }

  if (limit != 0 && obj.as.str_size > limit) {
    P_Echo(consoleplayer, "M_PBufReadString: String too long.");
    return false;
  }

  M_BufferWrite(buf, M_PBufGetDataAtCursor(pbuf), obj.as.str_size);
  M_BufferWriteUChar(buf, 0);

  M_BufferSeekForward(&pbuf->buf, obj.as.str_size);

  return true;
}

dboolean M_PBufReadStringArray(pbuf_t *pbuf, GPtrArray *strings,
                                             size_t string_count_limit,
                                             size_t string_size_limit) {
  uint32_t string_count = 0;

  if (!M_PBufReadArray(pbuf, &string_count))
    return false;

  if (string_count_limit > 0 && string_count > string_count_limit) {
    P_Echo(consoleplayer, "M_PBufReadStringArray: Too many strings.");
    return false;
  }

  for (unsigned int i = strings->len; i > 0; i--)
    g_ptr_array_remove_index_fast(strings, i - 1);

  for (unsigned int i = 0; i < string_count; i++) {
    cmp_object_t obj;
    char *s = NULL;

    if (!cmp_read_object(&pbuf->cmp, &obj)) {
      P_Printf(consoleplayer,
        "M_PBufReadStringArray: Error reading from packed buffer: %s\n",
        cmp_strerror(&pbuf->cmp)
      );
      return false;
    }

    if (string_size_limit > 0 && obj.as.str_size > string_size_limit) {
      P_Echo(consoleplayer, "M_PBufReadStringArray: String too long.");
      return false;
    }

    if (!(s = calloc(obj.as.str_size + 1, sizeof(char))))
      I_Error("M_PBufReadStringArray: Error allocating string");

    if (!M_BufferReadString(&pbuf->buf, s, obj.as.str_size)) {
      P_Echo(consoleplayer, "M_PBufReadStringArray: Error reading string");
      return false;
    }

    g_ptr_array_add(strings, s);
  }

  return true;
}

dboolean M_PBufAtEOF(pbuf_t *pbuf) {
  return M_PBufGetCursor(pbuf) == M_PBufGetSize(pbuf);
}

void M_PBufCompact(pbuf_t *pbuf) {
  M_BufferCompact(&pbuf->buf);
}

void M_PBufClear(pbuf_t *pbuf) {
  M_BufferClear(&pbuf->buf);
}

void M_PBufFree(pbuf_t *pbuf) {
  M_BufferFree(&pbuf->buf);
}

const char* M_PBufGetError(pbuf_t *pbuf) {
  return cmp_strerror(&pbuf->cmp);
}

void M_PBufPrint(pbuf_t *pbuf) {
  printf("PBuf capacity, size, cursor, address: [%zu, %zu, %zu, %p]: [\n",
    pbuf->buf.capacity, pbuf->buf.size, pbuf->buf.cursor, pbuf
  );
  for (int i = 0; i < MIN(24, pbuf->buf.size); i++) {
    printf("%02X ",
      ((const unsigned char *)(pbuf->buf.data + pbuf->buf.cursor))[i]
    );
  }
  printf("\n]\n");
}

/* vi: set et ts=2 sw=2: */

