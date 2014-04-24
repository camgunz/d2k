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

#include "z_zone.h"

#include <msgpack.h>

#include "g_game.h"
#include "lprintf.h"
#include "m_pbuf.h"

#define DECLARE_SIGNED_TYPE(tname, ctype, name, size)                         \
  const char *pbv_ ## tname ## _name = name;                                  \
  int64_t     pbv_ ## tname ## _size_limit_low = -(size);                     \
  int64_t     pbv_ ## tname ## _size_limit_high = (size >> 1);                \
  static const char* pbv_ ## tname ## _get_size_limit_range(void) {           \
    static char buf[50];                                                      \
    snprintf(buf, sizeof(buf), "%" PRId64 " <==> %" PRId64,                   \
      pbv_ ## tname ## _size_limit_low,                                       \
      pbv_ ## tname ## _size_limit_high                                       \
    );                                                                        \
    return buf;                                                               \
  }                                                                           \
  static dboolean pbv_ ## tname ## _type(msgpack_object *o) {                 \
    return ((o->type == MSGPACK_OBJECT_POSITIVE_INTEGER) ||                   \
            (o->type == MSGPACK_OBJECT_NEGATIVE_INTEGER));                    \
  }                                                                           \
  static dboolean pbv_ ## tname ## _size(msgpack_object *o) {                 \
    return pbv_ ## tname ## _size_limit_low <=                                \
           o->via.i64 <=                                                      \
           pbv_ ## tname ## _size_limit_high;                                 \
  }

#define DECLARE_UNSIGNED_TYPE(tname, ctype, name, size)                       \
  const char *pbv_ ## tname ## _name = name;                                  \
  uint64_t    pbv_ ## tname ## _size_limit_low = 0;                           \
  uint64_t    pbv_ ## tname ## _size_limit_high = size;                       \
  static const char* pbv_ ## tname ## _get_size_limit_range(void) {           \
    static char buf[50];                                                      \
    snprintf(buf, sizeof(buf), "%" PRIu64 " <==> %" PRIu64,                   \
      pbv_ ## tname ## _size_limit_low,                                       \
      pbv_ ## tname ## _size_limit_high                                       \
    );                                                                        \
    return buf;                                                               \
  }                                                                           \
  static dboolean pbv_ ## tname ## _type(msgpack_object *o) {                 \
    return o->type == MSGPACK_OBJECT_POSITIVE_INTEGER;                        \
  }                                                                           \
  static dboolean pbv_ ## tname ## _size(msgpack_object *o) {                 \
    return pbv_ ## tname ## _size_limit_low <=                                \
           o->via.u64 <=                                                      \
           pbv_ ## tname ## _size_limit_high;                                 \
  }

#define DECLARE_TYPE(tname, name, mp_type)                                    \
  const char *pbv_ ## tname ## _name = name;                                  \
  const char *pbv_ ## tname ## _size_limit = 0;                               \
  char        pbv_ ## tname ## _size_limit_low = 0;                           \
  char        pbv_ ## tname ## _size_limit_high = 0;                          \
  static const char* pbv_ ## tname ## _get_size_limit_range(void) {           \
    return "0 <==> 0";                                                        \
  }                                                                           \
  static dboolean pbv_ ## tname ## _type(msgpack_object *o) {                 \
    return o->type == MSGPACK_OBJECT_ ## mp_type;                             \
  }                                                                           \
  static dboolean pbv_ ## tname ## _size(msgpack_object *o) {                 \
    return true;                                                              \
  }

#define unpack(pbuf)                                                          \
  if (!msgpack_unpacker_next(pbuf->unpacker, pbuf->result)) {                 \
    doom_printf("%s: Unpacking error\n", __func__);                           \
    return false;                                                             \
  }

#define validate_type(obj, type)                                              \
  if (!(pbv_ ## type ## _type((obj)))) {                                      \
    doom_printf("%s: Invalid message: object is not %s.\n",                   \
      __func__, pbv_ ## type ## _name                                         \
    );                                                                        \
    return false;                                                             \
  }

#define validate_size(obj, type)                                              \
  if (!(pbv_ ## type ## _size((obj)))) {                                      \
    doom_printf("%s: Invalid message: object is too large (%s)\n",            \
      __func__, pbv_ ## type ## _get_size_limit_range()                       \
    );                                                                        \
    return false;                                                             \
  }

#define validate_range(obj, type, min, max)                                   \
  if (!(pbv_ ## type ## _range((obj), (min), (max)))) {                       \
    doom_printf("%s: Invalid message: %s is out of range (%s, %s)\n",         \
      __func__, #min, #max                                                    \
    );                                                                        \
    return false;                                                             \
  }

#define validate_array_size(arr, asize)                                       \
  if (arr.size > asize) {                                                     \
    doom_printf("%s: Invalid message: Array too large (%u > %u)\n",           \
      __func__, arr.size, asize                                               \
    );                                                                        \
    return false;                                                             \
  }

#define validate_type_and_size(object, type)                                  \
  validate_type(object, type)                                                 \
  validate_size(object, type)

#define unpack_and_validate(pbuf, object, type)                               \
  unpack(pbuf);                                                               \
  object = &pbuf->result->data;                                               \
  validate_type_and_size(object, type)

#define unpack_and_validate_range(pbuf, object, type, min, max)               \
  unpack_and_validate(pbuf, type)                                             \
  validate_range(object, type, min, max)

DECLARE_SIGNED_TYPE(char, char, "a char", 0xFF);
DECLARE_UNSIGNED_TYPE(uchar, unsigned char, "an unsigned char", 0xFF);
DECLARE_SIGNED_TYPE(short, short, "a short", 0xFFFF);
DECLARE_UNSIGNED_TYPE(ushort, unsigned short, "an unsigned short", 0xFFFF);
DECLARE_SIGNED_TYPE(int, int, "an int", 0xFFFFFFFF);
DECLARE_UNSIGNED_TYPE(uint, unsigned int, "an unsigned int", 0xFFFFFFFF);
DECLARE_SIGNED_TYPE(long, int_64_t, "a long", 0xFFFFFFFFFFFFFFFF);
DECLARE_SIGNED_TYPE(ulong, uint_64_t, "an unsigned long", 0xFFFFFFFFFFFFFFFF);
DECLARE_TYPE(double, "a double", DOUBLE);
DECLARE_TYPE(null, "a NULL value", NIL);
DECLARE_TYPE(boolean, "a boolean", BOOLEAN);
DECLARE_TYPE(raw, "raw bytes", RAW);
DECLARE_TYPE(array, "an array", ARRAY);
DECLARE_TYPE(map, "a map", MAP);

#define READ_ONLY(pbuf)                                                       \
  if (pbuf->mode != PBUF_MODE_READ)                                           \
    I_Error("%s: Packed buffer is not in read mode.\n", __func__)

#define WRITE_ONLY(pbuf)                                                      \
  if (pbuf->mode != PBUF_MODE_WRITE)                                          \
    I_Error("%s: Packed buffer is not in write mode.\n", __func__)

#define CHECK_MODE(mode)                                                      \
  if (mode != PBUF_MODE_READ && mode != PBUF_MODE_WRITE)                      \
    I_Error("%s: Invalid mode %d.\n", __func__, mode)

#define UNPACK(pbuf, type)                                                    \
  msgpack_object *obj = NULL;                                                 \
  READ_ONLY(pbuf);                                                            \
  unpack_and_validate(pbuf, obj, type)

void d2k_shut_up_clang(void) {
  pbv_null_type(NULL);
  pbv_null_size(NULL);
  pbv_null_get_size_limit_range();
}

static int buf_write(void *data, const char *buf, unsigned int len) {
  M_BufferWrite((buf_t *)data, (char *)buf, len);

  return 0;
}

void M_PBufInit(pbuf_t *pbuf, pbuf_mode_t mode) {
  CHECK_MODE(mode);

  if (mode == PBUF_MODE_READ) {
    pbuf->packer = msgpack_packer_new((void *)&pbuf->buf, buf_write);

    if (pbuf->packer == NULL)
      I_Error("M_PBufInit: Error initializing packed buffer's packer.");

    pbuf->unpacker = msgpack_unpacker_new(MSGPACK_UNPACKER_INIT_BUFFER_SIZE);

    if (pbuf->unpacker == NULL)
      I_Error("M_PBufInit: Error initializing packed buffer's unpacker.");

    pbuf->result = calloc(1, sizeof(msgpack_unpacked));
    msgpack_unpacked_init(pbuf->result);
  }
  else if (mode == PBUF_MODE_WRITE) {
    M_BufferInit(&pbuf->buf);
  }
  else {
    I_Error("Invalid mode %d.\n", mode);
  }

  pbuf->mode = mode;
}

void M_PBufInitWithCapacity(pbuf_t *pbuf, size_t capacity) {
  M_BufferInitWithCapacity(&pbuf->buf, capacity);
  pbuf->mode = PBUF_MODE_WRITE;
}

size_t M_PBufGetCapacity(pbuf_t *pbuf) {
  CHECK_MODE(pbuf->mode);

  if (pbuf->mode == PBUF_MODE_READ)
    return pbuf->unpacker->used + pbuf->unpacker->free;
  else if (pbuf->mode == PBUF_MODE_WRITE)
    return M_BufferGetCapacity(&pbuf->buf);
  else
    return 0;
}

size_t M_PBufGetSize(pbuf_t *pbuf) {
  CHECK_MODE(pbuf->mode);

  if (pbuf->mode == PBUF_MODE_READ)
    return pbuf->unpacker->used;
  else if (pbuf->mode == PBUF_MODE_WRITE)
    return M_BufferGetSize(&pbuf->buf);
  else
    return 0;
}

size_t M_PBufGetCursor(pbuf_t *pbuf) {
  CHECK_MODE(pbuf->mode);

  if (pbuf->mode == PBUF_MODE_READ)
    return pbuf->unpacker->parsed;
  else if (pbuf->mode == PBUF_MODE_WRITE)
    return M_BufferGetCursor(&pbuf->buf);
  else
    return 0;
}

buf_t* M_PBufGetBuffer(pbuf_t *pbuf) {
  CHECK_MODE(pbuf->mode);

  return &pbuf->buf;
}

char* M_PBufGetData(pbuf_t *pbuf) {
  CHECK_MODE(pbuf->mode);

  if (pbuf->mode == PBUF_MODE_READ)
    return msgpack_unpacker_buffer(pbuf->unpacker);
  else if (pbuf->mode == PBUF_MODE_WRITE)
    return M_BufferGetData(&pbuf->buf);
  else
    return 0;
}

void M_PBufEnsureCapacity(pbuf_t *pbuf, size_t capacity) {
  CHECK_MODE(pbuf->mode);

  if (pbuf->mode == PBUF_MODE_READ) {
    if (!msgpack_unpacker_reserve_buffer(pbuf->unpacker, capacity)) {
      I_Error(
        "M_PBufEnsureCapacity: Error reserving packed buffer capacity.\n"
      );
    }
  }
  else if (pbuf->mode == PBUF_MODE_WRITE) {
    M_BufferEnsureCapacity(&pbuf->buf, capacity);
  }
}

void M_PBufEnsureTotalCapacity(pbuf_t *pbuf, size_t capacity) {
  CHECK_MODE(pbuf->mode);

  if (pbuf->mode == PBUF_MODE_READ) {
    if (!msgpack_unpacker_expand_buffer(pbuf->unpacker, capacity)) {
      I_Error(
        "M_PBufEnsureTotalCapacity: Error expanding packed buffer capacity.\n"
      );
    }
  }
  else if (pbuf->mode == PBUF_MODE_WRITE) {
    M_BufferEnsureTotalCapacity(&pbuf->buf, capacity);
  }
}

void M_PBufCopy(pbuf_t *dst, pbuf_t *src) {
  M_PBufSetData(dst, M_PBufGetData(src), M_PBufGetSize(src));
}

void M_PBufSetData(pbuf_t *pbuf, void *data, size_t size) {
  CHECK_MODE(pbuf->mode);

  M_PBufClear(pbuf);
  M_PBufEnsureCapacity(pbuf, size);

  if (pbuf->mode == PBUF_MODE_READ) {
    memcpy(msgpack_unpacker_buffer(pbuf->unpacker), data, size);
    msgpack_unpacker_buffer_consumed(pbuf->unpacker, size);
  }
  else {
    M_BufferWrite(&pbuf->buf, data, size);
  }
}

dboolean M_PBufSetFile(pbuf_t *pbuf, const char *filename) {
  FILE *fp = NULL;
  size_t size = 0;
  size_t items_read = 0;
  dboolean out = false;

  CHECK_MODE(pbuf->mode);

  if ((fp = fopen(filename, "rb")) == NULL)
    return false;

  fseek(fp, 0, SEEK_END);
  size = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  M_PBufClear(pbuf);
  M_PBufEnsureCapacity(pbuf, size);

  items_read = fread(M_PBufGetData(pbuf), sizeof(byte), size, fp);

  if (items_read == size) {
    if (pbuf->mode == PBUF_MODE_READ) {
      msgpack_unpacker_buffer_consumed(pbuf->unpacker, size);
    }
    else if (pbuf->mode == PBUF_MODE_WRITE) {
      pbuf->buf.cursor = size;
      pbuf->buf.size = size;
    }
    out = true;
  }
  else {
    M_PBufClear(pbuf);
    out = false;
  }

  fclose(fp);

  return out;
}

void M_PBufWriteChar(pbuf_t *pbuf, char c) {
  WRITE_ONLY(pbuf);
  msgpack_pack_int8(pbuf->packer, c);
}

void M_PBufWriteCharArray(pbuf_t *pbuf, buf_t *chars) {
  size_t count = M_BufferGetSize(chars) / sizeof(char);

  M_PBufWriteArray(pbuf, count);

  for (int i = 0; i < count; i++)
    M_PBufWriteChar(pbuf, ((char *)chars->data)[i]);
}

void M_PBufWriteUChar(pbuf_t *pbuf, unsigned char c) {
  WRITE_ONLY(pbuf);
  msgpack_pack_uint8(pbuf->packer, c);
}

void M_PBufWriteUCharArray(pbuf_t *pbuf, buf_t *uchars) {
  size_t count = M_BufferGetSize(uchars) / sizeof(unsigned char);

  M_PBufWriteArray(pbuf, count);

  for (int i = 0; i < count; i++)
    M_PBufWriteUChar(pbuf, ((unsigned char *)uchars->data)[i]);
}

void M_PBufWriteShort(pbuf_t *pbuf, short s) {
  WRITE_ONLY(pbuf);
  msgpack_pack_int16(pbuf->packer, s);
}

void M_PBufWriteShortArray(pbuf_t *pbuf, buf_t *shorts) {
  size_t count = M_BufferGetSize(shorts) / sizeof(short);

  M_PBufWriteArray(pbuf, count);

  for (int i = 0; i < count; i++)
    M_PBufWriteShort(pbuf, ((short *)shorts->data)[i]);
}

void M_PBufWriteUShort(pbuf_t *pbuf, unsigned short s) {
  WRITE_ONLY(pbuf);
  msgpack_pack_uint16(pbuf->packer, s);
}

void M_PBufWriteUShortArray(pbuf_t *pbuf, buf_t *ushorts) {
  size_t count = M_BufferGetSize(ushorts) / sizeof(unsigned short);

  M_PBufWriteArray(pbuf, count);

  for (int i = 0; i < count; i++)
    M_PBufWriteUShort(pbuf, ((unsigned short *)ushorts->data)[i]);
}

void M_PBufWriteInt(pbuf_t *pbuf, int i) {
  WRITE_ONLY(pbuf);
  msgpack_pack_int32(pbuf->packer, i);
}

void M_PBufWriteIntArray(pbuf_t *pbuf, buf_t *ints) {
  size_t count = M_BufferGetSize(ints) / sizeof(int);

  M_PBufWriteArray(pbuf, count);

  for (int i = 0; i < count; i++)
    M_PBufWriteInt(pbuf, ((int *)ints->data)[i]);
}

void M_PBufWriteUInt(pbuf_t *pbuf, unsigned int i) {
  WRITE_ONLY(pbuf);
  msgpack_pack_uint32(pbuf->packer, i);
}

void M_PBufWriteUIntArray(pbuf_t *pbuf, buf_t *uints) {
  size_t count = M_BufferGetSize(uints) / sizeof(unsigned int);

  M_PBufWriteArray(pbuf, count);

  for (int i = 0; i < count; i++)
    M_PBufWriteUInt(pbuf, ((unsigned int *)uints->data)[i]);
}

void M_PBufWriteLong(pbuf_t *pbuf, int_64_t l) {
  WRITE_ONLY(pbuf);
  msgpack_pack_int64(pbuf->packer, l);
}

void M_PBufWriteLongArray(pbuf_t *pbuf, buf_t *longs) {
  size_t count = M_BufferGetSize(longs) / sizeof(int_64_t);

  M_PBufWriteArray(pbuf, count);

  for (int i = 0; i < count; i++)
    M_PBufWriteLong(pbuf, ((int_64_t *)longs->data)[i]);
}

void M_PBufWriteULong(pbuf_t *pbuf, uint_64_t l) {
  WRITE_ONLY(pbuf);
  msgpack_pack_uint64(pbuf->packer, l);
}

void M_PBufWriteULongArray(pbuf_t *pbuf, buf_t *ulongs) {
  size_t count = M_BufferGetSize(ulongs) / sizeof(uint_64_t);

  M_PBufWriteArray(pbuf, count);

  for (int i = 0; i < count; i++)
    M_PBufWriteULong(pbuf, ((uint_64_t *)ulongs->data)[i]);
}

void M_PBufWriteDouble(pbuf_t *pbuf, double d) {
  WRITE_ONLY(pbuf);
  msgpack_pack_double(pbuf->packer, d);
}

void M_PBufWriteDoubleArray(pbuf_t *pbuf, buf_t *doubles) {
  size_t count = M_BufferGetSize(doubles) / sizeof(double);

  M_PBufWriteArray(pbuf, count);

  for (int i = 0; i < count; i++)
    M_PBufWriteDouble(pbuf, ((double *)doubles->data)[i]);
}

void M_PBufWriteBool(pbuf_t *pbuf, dboolean b) {
  WRITE_ONLY(pbuf);

  if (b)
    msgpack_pack_true(pbuf->packer);
  else
    msgpack_pack_false(pbuf->packer);
}

void M_PBufWriteBoolArray(pbuf_t *pbuf, buf_t *bools) {
  size_t count = M_BufferGetSize(bools) / sizeof(dboolean);

  M_PBufWriteArray(pbuf, count);

  for (int i = 0; i < count; i++)
    M_PBufWriteBool(pbuf, ((dboolean *)bools->data)[i]);
}

void M_PBufWriteNil(pbuf_t *pbuf) {
  WRITE_ONLY(pbuf);
  msgpack_pack_nil(pbuf->packer);
}

void M_PBufWriteArray(pbuf_t *pbuf, unsigned int array_size) {
  WRITE_ONLY(pbuf);
  msgpack_pack_array(pbuf->packer, array_size);
}

void M_PBufWriteMap(pbuf_t *pbuf, unsigned int map_size) {
  WRITE_ONLY(pbuf);
  msgpack_pack_map(pbuf->packer, map_size);
}

void M_PBufWriteBytes(pbuf_t *pbuf, void *data, size_t size) {
  WRITE_ONLY(pbuf);
  msgpack_pack_raw(pbuf->packer, size);
  msgpack_pack_raw_body(pbuf->packer, data, size);
}

void M_PBufWriteString(pbuf_t *pbuf, char *data, size_t length) {
  M_PBufWriteBytes(pbuf, data, length);
}

void M_PBufWriteStringArray(pbuf_t *pbuf, obuf_t *obuf) {
  M_PBufWriteArray(pbuf, M_OBufGetObjectCount(obuf));

  OBUF_FOR_EACH(obuf, entry) {
    char *s = entry.obj;
    size_t length = strlen(s);

    M_PBufWriteString(pbuf, s, length);
  }
}

dboolean M_PBufReadChar(pbuf_t *pbuf, char *c) {
  UNPACK(pbuf, char);

  *c = (char)obj->via.i64;
  return true;
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
  UNPACK(pbuf, uchar);

  *c = (unsigned char)obj->via.u64;
  return true;
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
  UNPACK(pbuf, short);

  *s = (short)obj->via.i64;
  return true;
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
  UNPACK(pbuf, ushort);

  *s = (unsigned short)obj->via.u64;
  return true;
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
  UNPACK(pbuf, int);

  *i = (int)obj->via.i64;
  return true;
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
  UNPACK(pbuf, uint);

  *i = (unsigned int)obj->via.u64;
  return true;
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
  UNPACK(pbuf, long);

  *l = obj->via.i64;
  return true;
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
  UNPACK(pbuf, ulong);

  *l = obj->via.u64;
  return true;
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

dboolean M_PBufReadDouble(pbuf_t *pbuf, double *d) {
  UNPACK(pbuf, double);

  *d = obj->via.dec;
  return true;
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
  UNPACK(pbuf, boolean);

  if (obj->via.boolean)
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
  UNPACK(pbuf, null);

  return true;
}

dboolean M_PBufReadArray(pbuf_t *pbuf, unsigned int *array_size) {
  UNPACK(pbuf, array);

  *array_size = obj->via.array.size;
  return true;
}

dboolean M_PBufReadMap(pbuf_t *pbuf, unsigned int *map_size) {
  UNPACK(pbuf, map);

  *map_size = obj->via.map.size;
  return true;
}

dboolean M_PBufReadBytes(pbuf_t *pbuf, buf_t *buf) {
  UNPACK(pbuf, raw);

  if ((size_t)obj->via.raw.size > 0x00FFFFFF) { /* CG: 16MB */
    doom_printf("M_PBufReadString: String too long.\n");
    return false;
  }

  M_BufferSetData(
    &pbuf->buf, (char *)obj->via.raw.ptr, (size_t)obj->via.raw.size
  );

  return true;
}

dboolean M_PBufReadString(pbuf_t *pbuf, buf_t *buf, size_t limit) {
  UNPACK(pbuf, raw);

  if ((size_t)obj->via.raw.size > limit) {
    doom_printf("M_PBufReadString: String too long.\n");
    return false;
  }

  M_BufferSetData(
    &pbuf->buf, (char *)obj->via.raw.ptr, (size_t)obj->via.raw.size
  );
  M_BufferWriteUChar(buf, 0);
  return true;
}

dboolean M_PBufReadStringArray(pbuf_t *pbuf, obuf_t *obuf,
                                             size_t string_count_limit,
                                             size_t string_size_limit) {
  size_t string_count = 0;
  UNPACK(pbuf, array);

  string_count = obj->via.array.size;

  if (string_count > string_count_limit) {
    doom_printf("M_PBufReadStringArray: Too many strings.\n");
    return false;
  }

  M_OBufClear(obuf);
  M_OBufEnsureCapacity(obuf, string_count);

  for (int i = 0; i < string_count; i++) {
    buf_t *buf = M_BufferNew();

    if (!M_PBufReadString(pbuf, buf, string_size_limit))
      return false;

    M_OBufAppend(obuf, buf);
  }

#if 0
  for (int i = 0; i < string_count; i++) {
    char *s = NULL;
    msgpack_object *ps = obj->via.array.ptr + i;

    validate_type(ps, raw);
    if (ps->via.raw.size > string_size_limit) {
      doom_printf("M_PBufReadStringArray: String too long.\n");
      return false;
    }

    buf_entry = calloc(ps->via.raw.size + 1, sizeof(char));

    if (buf_entry == NULL)
      I_Error("M_PBufReadStringArray: calloc returned NULL.\n");

    memcpy(s, ps->via.raw.ptr, ps->via.raw.size);
    M_OBufAppend(obuf, buf_entry);
  }
#endif

  return true;
}

dboolean M_PBufAtEOF(pbuf_t *pbuf) {
  return M_PBufGetCursor(pbuf) == M_PBufGetSize(pbuf);
}

void M_PBufCompact(pbuf_t *pbuf) {
  CHECK_MODE(pbuf->mode);

  /* CG: This is a no-op in READ mode. */

  if (pbuf->mode == PBUF_MODE_WRITE)
    M_BufferCompact(&pbuf->buf);
}

void M_PBufZero(pbuf_t *pbuf) {
  CHECK_MODE(pbuf->mode);

  if (pbuf->mode == PBUF_MODE_READ) {
    msgpack_unpacker_reset_zone(pbuf->unpacker);
    msgpack_unpacked_init(pbuf->result);
  }
  else if (pbuf->mode == PBUF_MODE_WRITE) {
    M_BufferZero(&pbuf->buf);
  }
}

void M_PBufClear(pbuf_t *pbuf) {
  CHECK_MODE(pbuf->mode);

  if (pbuf->mode == PBUF_MODE_READ) {
    msgpack_unpacker_reset(pbuf->unpacker);
    msgpack_unpacked_destroy(pbuf->result);
  }
  else if (pbuf->mode == PBUF_MODE_WRITE) {
    M_BufferClear(&pbuf->buf);
  }
}

void M_PBufFree(pbuf_t *pbuf) {
  CHECK_MODE(pbuf->mode);

  if (pbuf->mode == PBUF_MODE_READ) {
    msgpack_unpacker_free(pbuf->unpacker);
    pbuf->unpacker = NULL;

    msgpack_unpacked_destroy(pbuf->result);
    free(pbuf->result);
    pbuf->result = NULL;
  }
  else if (pbuf->mode == PBUF_MODE_WRITE) {
    M_BufferFree(&pbuf->buf);
  }
}

/* vi: set et ts=2 sw=2: */

