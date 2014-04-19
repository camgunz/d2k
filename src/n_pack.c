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
 *
 *
 *-----------------------------------------------------------------------------
 */

#include "z_zone.h"

#include <enet/enet.h>
#include <msgpack.h>

#include "d_ticcmd.h"
#include "doomstat.h"
#include "g_game.h"
#include "lprintf.h"
#include "p_pspr.h"
#include "p_user.h"
#include "w_wad.h"

#include "n_net.h"
#include "n_main.h"
#include "n_state.h"
#include "n_peer.h"
#include "n_proto.h"

#define MAX_ARRAY_SIZE 128

#define DECLARE_SIGNED_TYPE(tname, ctype, name, size)                         \
  const char *npv_ ## tname ## _name = name;                                  \
  int64_t     npv_ ## tname ## _size_limit_low = -(size);                     \
  int64_t     npv_ ## tname ## _size_limit_high = (size >> 1);                \
  static const char* npv_ ## tname ## _get_size_limit_range(void) {           \
    static char buf[50];                                                      \
    snprintf(buf, sizeof(buf), "%" PRIi64 " <==> %" PRIi64,                   \
      npv_ ## tname ## _size_limit_low,                                       \
      npv_ ## tname ## _size_limit_high                                       \
    );                                                                        \
    return buf;                                                               \
  }                                                                           \
  static dboolean npv_ ## tname ## _type(msgpack_object *o) {                 \
    return ((o->type == MSGPACK_OBJECT_POSITIVE_INTEGER) ||                   \
            (o->type == MSGPACK_OBJECT_NEGATIVE_INTEGER));                    \
  }                                                                           \
  static dboolean npv_ ## tname ## _range(msgpack_object *o, ctype min,       \
                                                             ctype max) {     \
    return o->via.i64 >= min && o->via.i64 <= max;                            \
  }                                                                           \
  static dboolean npv_ ## tname ## _size(msgpack_object *o) {                 \
    return npv_ ## tname ## _size_limit_low <=                                \
           o->via.i64 <=                                                      \
           npv_ ## tname ## _size_limit_high;                                 \
  }

#define DECLARE_UNSIGNED_TYPE(tname, ctype, name, size)                       \
  const char *npv_ ## tname ## _name = name;                                  \
  uint64_t    npv_ ## tname ## _size_limit_low = 0;                           \
  uint64_t    npv_ ## tname ## _size_limit_high = size;                       \
  static const char* npv_ ## tname ## _get_size_limit_range(void) {           \
    static char buf[50];                                                      \
    snprintf(buf, sizeof(buf), "%" PRIu64 " <==> %" PRIu64,                   \
      npv_ ## tname ## _size_limit_low,                                       \
      npv_ ## tname ## _size_limit_high                                       \
    );                                                                        \
    return buf;                                                               \
  }                                                                           \
  static dboolean npv_ ## tname ## _type(msgpack_object *o) {                 \
    return o->type == MSGPACK_OBJECT_POSITIVE_INTEGER;                        \
  }                                                                           \
  static dboolean npv_ ## tname ## _range(msgpack_object *o, ctype min,       \
                                                             ctype max) {     \
    return o->via.u64 >= min && o->via.u64 <= max;                            \
  }                                                                           \
  static dboolean npv_ ## tname ## _size(msgpack_object *o) {                 \
    return npv_ ## tname ## _size_limit_low <=                                \
           o->via.u64 <=                                                      \
           npv_ ## tname ## _size_limit_high;                                 \
  }

#define DECLARE_TYPE(tname, name, mp_type)                                    \
  const char *npv_ ## tname ## _name = name;                                  \
  const char *npv_ ## tname ## _size_limit = 0;                               \
  char        npv_ ## tname ## _size_limit_low = 0;                           \
  char        npv_ ## tname ## _size_limit_high = 0;                          \
  static const char* npv_ ## tname ## _get_size_limit_range(void) {           \
    return "0 <==> 0";                                                        \
  }                                                                           \
  static dboolean npv_ ## tname ## _type(msgpack_object *o) {                 \
    return o->type == MSGPACK_OBJECT_ ## mp_type;                             \
  }                                                                           \
  static dboolean npv_ ## tname ## _size(msgpack_object *o) {                 \
    return true;                                                              \
  }

#define unpack(name)                                                          \
  if (!msgpack_unpacker_next(&pac, &result)) {                                \
    doom_printf("%s: Error unpacking %s\n", __func__, name);                  \
    return false;                                                             \
  }

#define validate_type(obj, name, type)                                        \
  if (!(npv_ ## type ## _type((obj)))) {                                      \
    doom_printf("%s: Invalid packet: %s is not %s.\n",                        \
      __func__, name, npv_ ## type ## _name                                   \
    );                                                                        \
    return false;                                                             \
  }

#define validate_size(obj, name, type)                                        \
  if (!(npv_ ## type ## _size((obj)))) {                                      \
    doom_printf("%s: Invalid packet: %s is too large (%s)\n",                 \
      __func__, name, npv_ ## type ## _get_size_limit_range()                 \
    );                                                                        \
    return false;                                                             \
  }

#define validate_range(obj, name, type, min, max)                             \
  if (!(npv_ ## type ## _range((obj), (min), (max)))) {                       \
    doom_printf("%s: Invalid packet: %s is out of range (%s, %s)\n",          \
      __func__, name, #min, #max                                              \
    );                                                                        \
    return false;                                                             \
  }

#define validate_array_size(arr, name, asize)                                 \
  if (arr.size > asize) {                                                     \
    doom_printf("%s: Invalid packet: Array %s too large (%u > %u)\n",         \
      __func__, name, arr.size, asize                                         \
    );                                                                        \
    return false;                                                             \
  }

#define validate_type_and_size(object, name, type)                            \
  validate_type(object, name, type)                                           \
  validate_size(object, name, type)

#define validate_player(object)                                               \
  validate_type_and_size(object, "player number", short)                      \
  if (object->via.i64 >= MAXPLAYERS) {                                        \
    doom_printf("%s: Invalid player number\n", __func__);                     \
    return false;                                                             \
  }

#define validate_recipient(object)                                            \
  validate_type_and_size(object, "recipient number", short)                   \
  if (object->via.i64 != -1 && object->via.i64 >= MAXPLAYERS) {               \
    doom_printf("%s: Invalid recipient number\n", __func__);                  \
    return false;                                                             \
  }

#define unpack_and_validate(object, name, type)                               \
  unpack(name);                                                               \
  object = &result.data;                                                      \
  validate_type_and_size(object, name, type)

#define unpack_and_validate_range(object, name, type, min, max)               \
  unpack_and_validate(object, name, type)                                     \
  validate_range(object, name, type, min, max)

#define unpack_and_validate_player(object)                                    \
  unpack("player number");                                                    \
  object = &result.data;                                                      \
  validate_player(object)

#define unpack_and_validate_array(o, a, name, ename, etype, ectype, buf, sz)  \
  unpack_and_validate(o, name, array)                                         \
  validate_array_size(a, name, sz)                                            \
  M_BufferClear((buf));                                                       \
  M_BufferEnsureTotalCapacity(&(buf), a.size);                                \
  for (int i = 0; i < a.size; i++) {                                          \
    msgpack_object *array_entry = a.ptr + i;                                  \
    validate_type_and_size(array_entry, ename, etype);                        \
    M_BufferWrite(buf, array_entry->via.raw.ptr, sizeof(ectype));             \
  }

#define unpack_and_validate_string_array(obj, arr, name, ename, obuf, sz)     \
  unpack_and_validate(obj, name, array)                                       \
  validate_array_size(arr, name, sz)                                          \
  M_OBufClear(obuf);                                                          \
  M_OBufEnsureCapacity(obuf, arr.size);                                       \
  for (int i = 0; i < arr.size; i++) {                                        \
    char *buf_entry = NULL;                                                   \
    msgpack_object *array_entry = arr.ptr + i;                                \
    validate_type(array_entry, ename, raw);                                   \
    buf_entry = calloc(array_entry->via.raw.size + 1, sizeof(char));          \
    memcpy(buf_entry, array_entry->via.raw.ptr, array_entry->via.raw.size);   \
    M_OBufAppend(obuf, buf_entry);                                            \
  }

#define unpack_and_validate_player_array(obj, arr, name, buf, sz)             \
  unpack_and_validate(obj, name, array);                                      \
  validate_array_size(arr, name, sz);                                         \
  M_BufferClear((buf));                                                       \
  M_BufferEnsureTotalCapacity(buf, arr.size * sizeof(short));                 \
  for (int i = 0; i < arr.size; i++) {                                        \
    validate_player((&(arr.ptr[i])));                                         \
    M_BufferWrite(buf, (char *)&arr.ptr[i].via.i64, sizeof(short));           \
  }

#ifndef msgpack_pack_char
#define msgpack_pack_char msgpack_pack_int8
#endif

#ifndef msgpack_pack_signed_char
#define msgpack_pack_signed_char msgpack_pack_int8
#endif

#ifndef msgpack_pack_unsigned_char
#define msgpack_pack_unsigned_char msgpack_pack_uint8
#endif

#define MAX_RESOURCE_NAMES 1000

DECLARE_SIGNED_TYPE(char, char, "a char", 0xFF);
DECLARE_UNSIGNED_TYPE(uchar, unsigned char, "an unsigned char", 0xFF);
DECLARE_SIGNED_TYPE(short, short, "a short", 0xFFFF);
DECLARE_UNSIGNED_TYPE(ushort, unsigned short, "an unsigned short", 0xFFFF);
DECLARE_SIGNED_TYPE(int, int, "an int", 0xFFFFFFFF);
DECLARE_UNSIGNED_TYPE(uint, unsigned int, "an unsigned int", 0xFFFFFFFF);
DECLARE_TYPE(double, "a double", DOUBLE);
static dboolean npv_double_range(msgpack_object *o, double min, double max) {
  return o->via.dec >= min && o->via.dec <= max;
}
DECLARE_TYPE(null, "a NULL value", NIL);
DECLARE_TYPE(boolean, "a boolean", BOOLEAN);
DECLARE_TYPE(raw, "raw bytes", RAW);
DECLARE_TYPE(array, "an array", ARRAY);
DECLARE_TYPE(map, "a map", MAP);

static msgpack_unpacker    pac;
static msgpack_unpacked    result;
static msgpack_object     *obj;
static msgpack_object_map *map = NULL;

static void pack_commands(netpeer_t *np, unsigned short playernum) {
  netticcmd_t *n = NULL;
  byte command_count = 0;
  cbuf_t *commands = NULL;

  msgpack_pack_unsigned_short(np->upk, playernum);

  if (np->playernum == playernum) {
    msgpack_pack_int(np->upk, np->command_tic);
    return;
  }

  commands = P_GetPlayerCommands(playernum);

  CBUF_FOR_EACH(commands, entry) {
    netticcmd_t *ncmd = (netticcmd_t *)entry.obj;

    if (ncmd->tic > np->command_tic) {
      command_count++;
    }
  }

  msgpack_pack_unsigned_char(np->upk, command_count);

  if (command_count == 0)
    return;

  CBUF_FOR_EACH(commands, entry) {
    netticcmd_t *ncmd = (netticcmd_t *)entry.obj;

    if (ncmd->tic <= np->command_tic)
      continue;

    if (n == NULL)
      printf("Packing commands [%d => ", ncmd->tic);

    n = ncmd;

    msgpack_pack_int(np->upk, ncmd->tic);
    msgpack_pack_signed_char(np->upk, ncmd->cmd.forwardmove);
    msgpack_pack_signed_char(np->upk, ncmd->cmd.sidemove);
    msgpack_pack_short(np->upk, ncmd->cmd.angleturn);
    msgpack_pack_short(np->upk, ncmd->cmd.consistancy);
    msgpack_pack_unsigned_char(np->upk, ncmd->cmd.chatchar);
    msgpack_pack_unsigned_char(np->upk, ncmd->cmd.buttons);
  }

  if (n != NULL)
    printf("%d]\n", n->tic);

}

void N_ShutUpClang(void) {
  npv_char_range(NULL, 0, 0);
  npv_short_range(NULL, 0, 0);
  npv_ushort_range(NULL, 0, 0);
  npv_null_type(NULL);
  npv_null_size(NULL);
  npv_null_get_size_limit_range();
}

void N_InitPacker(void) {
  msgpack_unpacker_init(&pac, MSGPACK_UNPACKER_INIT_BUFFER_SIZE);
  msgpack_unpacked_init(&result);
}

void N_LoadPacketData(void *data, size_t data_size) {
  msgpack_unpacker_reserve_buffer(&pac, data_size);
  memcpy(msgpack_unpacker_buffer(&pac), data, data_size);
  msgpack_unpacker_buffer_consumed(&pac, data_size);
}

dboolean N_LoadNewMessage(netpeer_t *np, byte *message_type) {
  if (!msgpack_unpacker_next(&pac, &result))
    return false;

  obj = &result.data;

  validate_type_and_size(obj, "message type", uchar);

  *message_type = (byte)obj->via.u64;

  return true;
}

void N_PackSetup(netpeer_t *np) {
  game_state_t *gs = N_GetLatestState();
  unsigned short player_count = 0;

  doom_printf("Packing setup (%d)\n", gametic);

  for (int i = 0; i < MAXPLAYERS; i++) {
    if (playeringame[i]) {
      player_count++;
    }
  }

  msgpack_pack_unsigned_char(np->rpk, nm_setup);
  msgpack_pack_int(np->rpk, netsync);
  msgpack_pack_unsigned_short(np->rpk, player_count);
  msgpack_pack_unsigned_short(np->rpk, np->playernum);

  msgpack_pack_array(np->rpk, M_OBufGetObjectCount(&resource_files_buf));
  OBUF_FOR_EACH(&resource_files_buf, entry) {
    char *resource_name = entry.obj;
    size_t length = strlen(resource_name);

    msgpack_pack_raw(np->rpk, length);
    msgpack_pack_raw_body(np->rpk, resource_name, length);
  }

  msgpack_pack_array(np->rpk, M_OBufGetObjectCount(&deh_files_buf));
  OBUF_FOR_EACH(&deh_files_buf, entry) {
    char *deh_name = entry.obj;
    size_t length = strlen(deh_name);

    msgpack_pack_raw(np->rpk, length);
    msgpack_pack_raw_body(np->rpk, deh_name, length);
  }

  printf(
    "N_PackSetup: Sent game state at %d (player count: %d).\n",
    gs->tic, player_count
  );

  msgpack_pack_unsigned_int(np->rpk, gs->tic);
  msgpack_pack_raw(np->rpk, gs->data.size);
  msgpack_pack_raw_body(np->rpk, gs->data.data, gs->data.size);
}

dboolean N_UnpackSetup(netpeer_t *np, net_sync_type_e *sync_type,
                                      unsigned short *player_count,
                                      unsigned short *playernum) {
  net_sync_type_e m_sync_type = NET_SYNC_TYPE_NONE;
  unsigned short m_player_count = 0;
  unsigned short m_playernum = 0;
  game_state_t *gs = N_GetNewState();

  unpack_and_validate_range(
    obj, "net sync type", int, NET_SYNC_TYPE_COMMAND, NET_SYNC_TYPE_DELTA
  );
  m_sync_type = (net_sync_type_e)obj->via.u64;

  unpack_and_validate(obj, "player count", ushort);
  m_player_count = (unsigned short)obj->via.u64;

  unpack_and_validate(obj, "consoleplayer", ushort);
  m_playernum = (unsigned short)obj->via.u64;

  unpack_and_validate_string_array(
    obj,
    obj->via.array,
    "resource names",
    "resource name",
    &resource_files_buf,
    MAX_RESOURCE_NAMES
  );

  unpack_and_validate_string_array(
    obj,
    obj->via.array,
    "deh names",
    "deh name",
    &deh_files_buf,
    MAX_RESOURCE_NAMES
  );

  unpack_and_validate(obj, "game state tic", int);
  gs->tic = (int)obj->via.i64;

  unpack_and_validate(obj, "game state data", raw);

  M_BufferSetData(
    &gs->data, (char *)obj->via.raw.ptr, (size_t)obj->via.raw.size
  );

  *sync_type = m_sync_type;
  *player_count = m_player_count;
  *playernum = m_playernum;

  N_SetLatestState(gs);

  return true;
}

void N_PackAuthResponse(netpeer_t *np, auth_level_e auth_level) {
  doom_printf("Packing auth response\n");

  msgpack_pack_unsigned_char(np->rpk, nm_authresponse);
  msgpack_pack_unsigned_char(np->rpk, auth_level);
}

dboolean N_UnpackAuthResponse(netpeer_t *np, auth_level_e *auth_level) {
  unpack_and_validate_range(
    obj, "authorization level", uint, 0, AUTH_LEVEL_MAX - 1
  );

  *auth_level = (auth_level_e)obj->via.u64;

  return true;
}

void N_PackServerMessage(netpeer_t *np, char *message) {
  size_t length = strlen(message) * sizeof(char);

  doom_printf("Packing server message\n");

  msgpack_pack_unsigned_char(np->rpk, nm_servermessage);
  msgpack_pack_raw(np->rpk, length);
  msgpack_pack_raw_body(np->rpk, message, length);
}

dboolean N_UnpackServerMessage(netpeer_t *np, buf_t *buf) {
  unpack_and_validate(obj, "server message content", raw);

  M_BufferSetString(buf, (char *)obj->via.raw.ptr, (size_t)obj->via.raw.size);

  return true;
}

void N_PackSync(netpeer_t *np) {
  msgpack_pack_unsigned_char(np->upk, nm_sync);

  if (DELTACLIENT)
    msgpack_pack_int(np->upk, np->state_tic);

  if (SERVER) {
    unsigned short player_count = 0;

    for (int i = 0; i < N_GetPeerCount(); i++) {
      if (N_GetPeer(i) != NULL) {
        player_count++;
      }
    }

    msgpack_pack_unsigned_short(np->upk, player_count);

    for (int i = 0; i < N_GetPeerCount(); i++) {
      netpeer_t *snp = N_GetPeer(i);

      if (snp != NULL)
        pack_commands(np, snp->playernum);
    }
  }
  else {
    msgpack_pack_unsigned_short(np->upk, 1);
    pack_commands(np, consoleplayer);
  }
}

dboolean N_UnpackSync(netpeer_t *np, dboolean *update_sync) {
  unsigned short player_count = 0;
  int m_state_tic = -1;
  int m_command_tic = -1;
  dboolean m_update_sync = false;

  *update_sync = false;

  if (DELTASERVER) {
    unpack_and_validate(obj, "state tic", int);
    m_state_tic = (int)obj->via.i64;

    if (np->state_tic != m_state_tic)
      m_update_sync = true;
  }

  unpack_and_validate(obj, "player count", ushort);
  player_count = (unsigned short)obj->via.u64;

  for (int i = 0; i < player_count; i++) {
    unsigned short playernum = 0;
    byte command_count = 0;
    cbuf_t *commands = NULL;

    unpack_and_validate_player(obj);
    playernum = (unsigned short)obj->via.u64;

    if (SERVER && np->playernum != playernum) {
      doom_printf(
        "N_UnpackPlayerCommands: Erroneously received player commands for %d "
        "from player %d\n",
        playernum,
        np->playernum
      );
      return false;
    }

    if (CMDCLIENT && playernum == consoleplayer) {
      unpack_and_validate(obj, "command tic", int);
      m_command_tic = (int)obj->via.i64;

      if (np->command_tic != m_command_tic)
        m_update_sync = true;

      continue;
    }

  /*
   * CG: TODO: Add a limit to the number of commands accepted here.  uchar
   *           limits this to 255 commands, but in reality that's > 7 seconds,
   *           which is still far too long.  Quake has an "sv_maxlag" setting
   *           (or something), that may be preferable to a static limit... but
   *           I think having an upper bound on that setting is still prudent.
   */
    unpack_and_validate(obj, "command count", uchar);
    command_count = (byte)obj->via.u64;

    commands = P_GetPlayerCommands(playernum);

    M_CBufEnsureCapacity(commands, command_count);
    M_CBufConsolidate(commands);

    netticcmd_t *n = NULL;

    while (command_count--) {
      unpack_and_validate(obj, "command tic", int);
      m_command_tic = (int)obj->via.i64;

      if (m_command_tic > np->command_tic) {
        netticcmd_t *ncmd = M_CBufGetFirstFreeOrNewSlot(commands);

        if (n == NULL)
          printf("Unpacking commands [%d => ", ncmd->tic);

        n = ncmd;

        ncmd->tic = m_command_tic;

        unpack_and_validate(obj, "command forward value", char);
        ncmd->cmd.forwardmove = (signed char)obj->via.i64;

        unpack_and_validate(obj, "command side value", char);
        ncmd->cmd.sidemove = (signed char)obj->via.i64;

        unpack_and_validate(obj, "command angle value", short);
        ncmd->cmd.angleturn = (signed short)obj->via.i64;

        unpack_and_validate(obj, "command consistancy value", short);
        ncmd->cmd.consistancy = (signed short)obj->via.i64;

        unpack_and_validate(obj, "command chatchar value", uchar);
        ncmd->cmd.chatchar = (byte)obj->via.u64;

        unpack_and_validate(obj, "command buttons value", uchar);
        ncmd->cmd.buttons = (byte)obj->via.u64;

      }
      else {
        unpack_and_validate(obj, "command forward value", char);
        unpack_and_validate(obj, "command side value", char);
        unpack_and_validate(obj, "command angle value", short);
        unpack_and_validate(obj, "command consistancy value", short);
        unpack_and_validate(obj, "command chatchar value", uchar);
        unpack_and_validate(obj, "command buttons value", uchar);
      }
    }

    if (n != NULL)
      printf("%d]\n", n->tic);
  }

  if (SERVER && np->command_tic < m_command_tic)
    m_update_sync = true;

  if (m_state_tic != -1)
    np->state_tic = m_state_tic;

  if (m_command_tic != -1)
    np->command_tic = m_command_tic;

  if (m_update_sync)
    *update_sync = m_update_sync;

  printf("Received sync (%d): [%d, %d] (%d).\n",
    gametic, np->command_tic, np->state_tic, m_update_sync
  );

  return true;
}

void N_PackDeltaSync(netpeer_t *np) {
  printf("Sending sync to %d (%d): [%d, %d].\n",
    np->playernum, gametic, np->command_tic, np->state_tic
  );

  msgpack_pack_unsigned_char(np->upk, nm_sync);
  msgpack_pack_int(np->upk, np->delta.from_tic);
  msgpack_pack_int(np->upk, np->delta.to_tic);
  msgpack_pack_raw(np->upk, np->delta.data.size);
  msgpack_pack_raw_body(np->upk, np->delta.data.data, np->delta.data.size);
}

dboolean N_UnpackDeltaSync(netpeer_t *np, dboolean *update_sync) {
  int m_delta_from_tic = 0;
  int m_delta_to_tic = 0;
  dboolean m_update_sync = false;

  *update_sync = false;

  unpack_and_validate(obj, "delta from tic", int);
  m_delta_from_tic = (int)obj->via.i64;

  unpack_and_validate(obj, "delta to tic", int);
  m_delta_to_tic = (int)obj->via.i64;

  unpack_and_validate(obj, "delta data", raw);

  if (np->state_tic < m_delta_to_tic) {
    np->delta.from_tic = m_delta_from_tic;
    np->delta.to_tic = m_delta_to_tic;
    np->state_tic = m_delta_to_tic;
    M_BufferSetData(
      &np->delta.data, (char *)obj->via.raw.ptr, (size_t)obj->via.raw.size
    );

    m_update_sync = true;
  }

  printf("Received sync (%d): [%d, %d] (%d).\n",
    gametic, np->command_tic, np->state_tic, m_update_sync
  );

  *update_sync = m_update_sync;

  return true;
}

void N_PackPlayerMessage(netpeer_t *np, unsigned short sender,
                                        size_t recipient_count,
                                        buf_t *recipients,
                                        char *message) {
  size_t length = strlen(message) * sizeof(char);
  short *ra = (short *)recipients->data;

  doom_printf("Packing player message\n");

  msgpack_pack_unsigned_char(np->rpk, nm_playermessage);
  msgpack_pack_unsigned_short(np->rpk, sender);
  msgpack_pack_array(np->rpk, recipient_count);

  for (int i = 0; i < recipient_count; i++)
    msgpack_pack_short(np->rpk, ra[i]);

  msgpack_pack_raw(np->rpk, length);
  msgpack_pack_raw_body(np->rpk, message, length);
}

dboolean N_UnpackPlayerMessage(netpeer_t *np, unsigned short *sender,
                                              size_t *recipient_count,
                                              buf_t *recipients,
                                              buf_t *buf) {
  size_t m_recipient_count = 0;
  unsigned short m_sender = 0;

  unpack_and_validate_player(obj);
  m_sender = (unsigned short)obj->via.u64;

  unpack_and_validate_player_array(
    obj, obj->via.array, "player message recipients", recipients, 0xFF
  );
  m_recipient_count = obj->via.array.size;

  unpack_and_validate(obj, "player message content", raw);

  M_BufferSetString(buf, (char *)obj->via.raw.ptr, (size_t)obj->via.raw.size);

  *sender = m_sender;
  *recipient_count = m_recipient_count;

  return true;
}

dboolean N_UnpackPlayerPreferenceChange(netpeer_t *np, short *playernum,
                                                       int *tic,
                                                       size_t *count) {
  short m_playernum = 0;
  int m_tic = 0;

  unpack_and_validate_player(obj);
  m_playernum = (short)obj->via.i64;

  unpack_and_validate(obj, "player preference change tic", int);
  m_tic = (int)obj->via.i64;

  unpack_and_validate(obj, "player preference change map", map);

  map = &obj->via.map;

  *playernum = m_playernum;
  *tic = m_tic;
  *count = map->size;

  return true;
}

dboolean N_UnpackPlayerPreferenceName(netpeer_t *np, size_t pref_index,
                                                     buf_t *buf) {
  msgpack_object_kv *pair = NULL;
  
  if (pref_index >= map->size) {
    doom_printf(
      "N_UnpackPlayerPreferenceName: Attempted to index past preference "
      "count\n"
    );
    return false;
  }

  pair = map->ptr + pref_index;

  validate_type(&pair->key, "player preference name", raw);

  obj = &pair->val;

  M_BufferSetString(
    buf, (char *)pair->key.via.raw.ptr, (size_t)pair->key.via.raw.size
  );

  return true;
}

void N_PackNameChange(netpeer_t *np, short playernum, char *new_name) {
  size_t length = strlen(new_name) * sizeof(char);

  doom_printf("Packing name change\n");

  msgpack_pack_unsigned_char(np->rpk, nm_playerpreferencechange);
  msgpack_pack_int(np->rpk, gametic);
  msgpack_pack_short(np->rpk, playernum);
  msgpack_pack_map(np->rpk, 2);
  msgpack_pack_raw(np->rpk, 4);
  msgpack_pack_raw_body(np->rpk, "name", 4);
  msgpack_pack_raw(np->rpk, length);
  msgpack_pack_raw_body(np->rpk, new_name, length);
}

dboolean N_UnpackNameChange(netpeer_t *np, buf_t *buf) {
  unpack_and_validate(obj, "new name", raw);

  M_BufferSetString(buf, (char *)obj->via.raw.ptr, (size_t)obj->via.raw.size);

  return true;
}

void N_PackTeamChange(netpeer_t *np, short playernum, byte new_team) {
  doom_printf("Packing team change\n");

  msgpack_pack_unsigned_char(np->rpk, nm_playerpreferencechange);
  msgpack_pack_int(np->rpk, gametic);
  msgpack_pack_short(np->rpk, playernum);
  msgpack_pack_map(np->rpk, 2);
  msgpack_pack_raw(np->rpk, 4);
  msgpack_pack_raw_body(np->rpk, "team", 4);
  msgpack_pack_unsigned_char(np->rpk, new_team);
}

dboolean N_UnpackTeamChange(netpeer_t *np, byte *new_team) {
  int team_count = 0; /* CG: TODO: teams */

  if (team_count > 0) {
    unpack_and_validate_range(obj, "team index", uchar, 0, team_count - 1);
    *new_team = (byte)obj->via.u64;
  }
  else {
    *new_team = 0;
  }

  return true;
}

void N_PackPWOChange(netpeer_t *np, short playernum) {
  doom_printf("Packing PWO change\n");

  msgpack_pack_unsigned_char(np->rpk, nm_playerpreferencechange);
  msgpack_pack_int(np->rpk, gametic);
  msgpack_pack_short(np->rpk, playernum);
  msgpack_pack_map(np->rpk, 2);
  msgpack_pack_raw(np->rpk, 3);
  msgpack_pack_raw_body(np->rpk, "pwo", 3);
  /* CG: TODO */
}

dboolean N_UnpackPWOChange(netpeer_t *np) {
  /* CG: TODO */
  return false;
}

void N_PackWSOPChange(netpeer_t *np, short playernum, byte new_wsop_flags) {
  doom_printf("Packing WSOP change\n");

  msgpack_pack_unsigned_char(np->rpk, nm_playerpreferencechange);
  msgpack_pack_int(np->rpk, gametic);
  msgpack_pack_short(np->rpk, playernum);
  msgpack_pack_map(np->rpk, 2);
  msgpack_pack_raw(np->rpk, 4);
  msgpack_pack_raw_body(np->rpk, "wsop", 4);
  msgpack_pack_unsigned_char(np->rpk, new_wsop_flags);
}

dboolean N_UnpackWSOPChange(netpeer_t *np, byte *new_wsop_flags) {
  unpack_and_validate_range(
    obj, "new WSOP value", uchar, 0, 2 << (WSOP_MAX - 2)
  );

  *new_wsop_flags = (byte)obj->via.u64;

  return true;
}

void N_PackBobbingChange(netpeer_t *np, short playernum,
                                        double new_bobbing_amount) {
  doom_printf("Packing bobbing change\n");

  msgpack_pack_unsigned_char(np->rpk, nm_playerpreferencechange);
  msgpack_pack_int(np->rpk, gametic);
  msgpack_pack_short(np->rpk, playernum);
  msgpack_pack_map(np->rpk, 2);
  msgpack_pack_raw(np->rpk, 7);
  msgpack_pack_raw_body(np->rpk, "bobbing", 7);
  msgpack_pack_double(np->rpk, new_bobbing_amount);
}

dboolean N_UnpackBobbingchanged(netpeer_t *np, double *new_bobbing_amount) {
  unpack_and_validate_range(obj, "new bobbing amount", double, 0.0, 1.0);

  *new_bobbing_amount = obj->via.dec;

  return true;
}

void N_PackAutoaimChange(netpeer_t *np, short playernum,
                                        dboolean new_autoaim_enabled) {
  doom_printf("Packing autoaim change\n");

  msgpack_pack_unsigned_char(np->rpk, nm_playerpreferencechange);
  msgpack_pack_int(np->rpk, gametic);
  msgpack_pack_short(np->rpk, playernum);
  msgpack_pack_map(np->rpk, 2);
  msgpack_pack_raw(np->rpk, 7);
  msgpack_pack_raw_body(np->rpk, "autoaim", 7);
  if (new_autoaim_enabled)
    msgpack_pack_true(np->rpk);
  else
    msgpack_pack_false(np->rpk);
}

dboolean N_UnpackAutoaimChange(netpeer_t *np, dboolean *new_autoaim_enabled) {
  unpack_and_validate(obj, "new Autoaim value", boolean);

  *new_autoaim_enabled = obj->via.boolean;

  return true;
}

void N_PackWeaponSpeedChange(netpeer_t *np, short playernum,
                                            byte new_weapon_speed) {
  doom_printf("Packing weapon speed change\n");

  msgpack_pack_unsigned_char(np->rpk, nm_playerpreferencechange);
  msgpack_pack_int(np->rpk, gametic);
  msgpack_pack_short(np->rpk, playernum);
  msgpack_pack_map(np->rpk, 2);
  msgpack_pack_raw(np->rpk, 12);
  msgpack_pack_raw_body(np->rpk, "weapon_speed", 12);
  msgpack_pack_unsigned_char(np->rpk, new_weapon_speed);
}

dboolean N_UnpackWeaponSpeedChange(netpeer_t *np, byte *new_weapon_speed) {

  unpack_and_validate(obj, "new weapon speed value", uchar);

  *new_weapon_speed = (byte)obj->via.u64;

  return true;
}

void N_PackColorChange(netpeer_t *np, short playernum, byte new_red,
                                                       byte new_green,
                                                       byte new_blue) {
  doom_printf("Packing color change\n");

  msgpack_pack_unsigned_char(np->rpk, nm_playerpreferencechange);
  msgpack_pack_int(np->rpk, gametic);
  msgpack_pack_short(np->rpk, playernum);
  msgpack_pack_map(np->rpk, 2);
  msgpack_pack_raw(np->rpk, 5);
  msgpack_pack_raw_body(np->rpk, "color", 5);
  msgpack_pack_unsigned_int(np->rpk, (new_red   << 24) |
                                     (new_green << 16) |
                                     (new_blue  <<  8));
}

dboolean N_UnpackColorChange(netpeer_t *np, byte *new_red, byte *new_green,
                                            byte *new_blue) {
  unpack_and_validate(obj, "new color", uint);

  *new_red   = (obj->via.u64 >> 24) & 0xFF;
  *new_green = (obj->via.u64 >> 16) & 0xFF;;
  *new_blue  = (obj->via.u64 >>  8) & 0xFF;;

  return true;
}

void N_PackColormapChange(netpeer_t *np, short playernum, int new_color) {
  doom_printf("Packing colormap change\n");

  msgpack_pack_unsigned_char(np->rpk, nm_playerpreferencechange);
  msgpack_pack_int(np->rpk, gametic);
  msgpack_pack_short(np->rpk, playernum);
  msgpack_pack_map(np->rpk, 2);
  msgpack_pack_raw(np->rpk, 8);
  msgpack_pack_raw_body(np->rpk, "colormap", 8);
  msgpack_pack_int(np->rpk, new_color);
}

dboolean N_UnpackColormapChange(netpeer_t *np, short *playernum,
                                               int *new_color) {
  short m_playernum = -1;
  int   m_new_color = -1;

  unpack_and_validate_player(obj);
  m_playernum = (short)obj->via.i64;

  unpack_and_validate(obj, "new color", uchar);

  /* CG: TODO: Validate color value is reasonable */
  m_new_color = (int)obj->via.i64;

  *playernum = m_playernum;
  *new_color = m_new_color;

  return true;
}

void N_PackSkinChange(netpeer_t *np, short playernum) {
  doom_printf("Packing skin change\n");

  msgpack_pack_unsigned_char(np->rpk, nm_playerpreferencechange);
  msgpack_pack_int(np->rpk, gametic);
  msgpack_pack_short(np->rpk, playernum);
  msgpack_pack_map(np->rpk, 2);
  msgpack_pack_raw(np->rpk, 9);
  msgpack_pack_raw_body(np->rpk, "skin_name", 9);
  /* CG: TODO */
}

dboolean N_UnpackSkinChange(netpeer_t *np) {
  /* CG: TODO */
  return false;
}

void N_PackAuthRequest(netpeer_t *np, char *password) {
  doom_printf("Packing auth request\n");

  size_t length = strlen(password) * sizeof(char);

  msgpack_pack_unsigned_char(np->rpk, nm_authrequest);
  msgpack_pack_raw(np->rpk, length);
  msgpack_pack_raw_body(np->rpk, password, length);
}

dboolean N_UnpackAuthRequest(netpeer_t *np, buf_t *buf) {
  unpack_and_validate(obj, "authorization request password", raw);

  M_BufferSetString(buf, (char *)obj->via.raw.ptr, (size_t)obj->via.raw.size);

  return true;
}

void N_PackRCONCommand(netpeer_t *np, char *command) {
  doom_printf("Packing RCON\n");

  size_t length = strlen(command) * sizeof(char);

  msgpack_pack_unsigned_char(np->rpk, nm_rconcommand);
  msgpack_pack_raw(np->rpk, length);
  msgpack_pack_raw_body(np->rpk, command, length);
}

dboolean N_UnpackRCONCommand(netpeer_t *np, buf_t *buf) {
  unpack_and_validate(obj, "RCON command", raw);

  M_BufferSetString(buf, (char *)obj->via.raw.ptr, (size_t)obj->via.raw.size);

  return true;
}

void N_PackVoteRequest(netpeer_t *np, char *command) {
  doom_printf("Packing vote request\n");

  size_t length = strlen(command) * sizeof(char);

  msgpack_pack_unsigned_char(np->rpk, nm_voterequest);
  msgpack_pack_raw(np->rpk, length);
  msgpack_pack_raw_body(np->rpk, command, length);
}

dboolean N_UnpackVoteRequest(netpeer_t *np, buf_t *buf) {
  unpack_and_validate(obj, "vote request command", raw);

  M_BufferSetString(buf, (char *)obj->via.raw.ptr, (size_t)obj->via.raw.size);

  return true;
}

/* vi: set et ts=2 sw=2: */

