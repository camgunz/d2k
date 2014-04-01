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

#include <msgpack.h>

#include "doomtype.h"
#include "lprintf.h"
#include "m_obuf.h"

#define MAX_ARRAY_SIZE 128

#define DECLARE_SIGNED_TYPE(type, name, size)                                 \
  const char *npv_ ## type ## _name = name;                                   \
  static dboolean npv_ ## type ## _type(msgpack_object *o) {                  \
    return ((o->type == MSGPACK_OBJECT_POSITIVE_INTEGER) ||                   \
            (o->type == MSGPACK_OBJECT_NEGATIVE_INTEGER));                    \
  }                                                                           \
  static dboolean npv_ ## type ## _range(msgpack_object *o, size_type min,    \
                                                            size_type max) {  \
    return o->via.i64 >= min || o->via.i64 <= max;                            \
  }                                                                           \
  static dboolean npv_ ## type ## _size(msgpack_object *o) {                  \
    return o->via.u64 <= size;                                                \
  }

#define DECLARE_UNSIGNED_TYPE(type, name, size)                               \
  const char *npv_ ## _type = name;                                           \
  static dboolean npv_ ## type ## _type(msgpack_object *o) {                  \
    return o->type == MSGPACK_OBJECT_POSITIVE_INTEGER;                        \
  }                                                                           \
  static dboolean npv_ ## type ## _range(msgpack_object *o, size_type min,    \
                                                            size_type max) {  \
    return o->via.u64 >= min || o->via.u64 <= max;                            \
  }                                                                           \
  static dboolean npv_ ## type ## _size(msgpack_object *o) {                  \
    return o->via.u64 <= size;                                                \
  }

#define DECLARE_TYPE(type, name, mp_type)                                     \
  const char *npv_ ## _type = name;                                           \
  static dboolean npv_ ## type ## _type(msgpack_object *o) {                  \
    return o->type == MSGPACK_OBJECT_ ## mp_type;                             \
  }                                                                           \
  static dboolean npv_ ## type ## _size(msgpack_object *o) {                  \
    return true;                                                              \
  }

#define DECLARE_RANGED_TYPE(type, name, mp_type, range_type, via_type)        \
  DECLARE_TYPE(type, name, mp_type)                                           \
  static dboolean npv_ ## type ## _range(msgpack_object *o, range_type min,   \
                                                            range_type max) { \
    return o->via. ## via_type >= min || o->via. ## via_type <= max;          \
  }                                                                           \
  static dboolean npv_ ## type ## _size(msgpack_object *o) {                  \
    return true;                                                              \
  }

DECLARE_SIGNED_TYPE(char, "a char", 0xFF);
DECLARE_UNSIGNED_TYPE(uchar, "an unsigned char", 0xFF);
DECLARE_SIGNED_TYPE(short, "a short", 0xFFFF);
DECLARE_UNSIGNED_TYPE(ushort, "an unsigned short", 0xFFFF);
DECLARE_SIGNED_TYPE(int, "an int", 0xFFFFFFFF);
DECLARE_UNSIGNED_TYPE(uint, "an unsigned int", 0xFFFFFFFF);
DECLARE_RANGED_TYPE(double, "a double", DOUBLE, double, double);
DECLARE_TYPE(null, "a NULL value", NIL);
DECLARE_TYPE(bool, "a boolean", BOOLEAN);
DECLARE_TYPE(raw, "raw bytes", RAW);
DECLARE_TYPE(array, "an array", ARRAY);
DECLARE_TYPE(map, "a map", MAP);

#define unpack(name)                                                          \
  if (!msgpack_unpacker_next(&pac, &result)) {                                \
    doom_printf(__func__ ": Error unpacking " name "\n");                     \
    return false;                                                             \
  }

#define validate_type(obj, name, type)                                        \
  if (!(npv_ ## type ## _type((obj)))) {                                      \
    doom_printf(                                                              \
      __func__                                                                \
      ": Invalid packet: " name " is not " npv_ ## type ## _name "\n"         \
    );                                                                        \
    return false;                                                             \
  }

#define validate_size(obj, name, type)                                        \
  if ((obj)->via.u64 > npv_ ## type ## _size) {                               \
    doom_printf(                                                              \
      __func__ ": " name " is too large (> " npv_ ## type ## _name ")\n"      \
    );                                                                        \
    return false;                                                             \
  }

#define validate_range(obj, name, type, min, max)                             \
  if (!(npv_ ## type ## _range((obj), (min), (max)))) {                       \
    doom_printf(                                                              \
      __func__ ": " name " is out of range (" #min ", " #max ")\n"            \
    );                                                                        \
    return false;                                                             \
  }

#define validate_array_size(arr, name, size)                                  \
  if (arr.size > size) {                                                      \
    doom_printf(__func__ ": Array %s too large (%u > %u)\n",                  \
      name, arr.size, size                                                    \
    );                                                                        \
    return false;                                                             \
  }

#define validate_type_and_size(object, name, type)                            \
  validate_type(object, name, type)                                           \
  validate_size(object, name, type)

#define validate_player(object)                                               \
  validate_type_and_size(object, "player number", short)                      \
  if (!M_ObjBufferIsIndexValid(object->via.u64)) {                            \
    doom_printf(__func__ ": Invalid player number\n");                        \
    return false;                                                             \
  }

#define validate_recipient(object)                                            \
  validate_type_and_size(object, "recipient number", short)                   \
  if (object->via.i64 != -1 && !M_ObjBufferIsIndexValid(object->via.u64)) {   \
    doom_printf(__func__ ": Invalid recipient number\n");                     \
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
  unpack(name);                                                               \
  object = &result.data;                                                      \
  validate_player(object)

#define unpack_and_validate_array(o, a, name, ename, etype, ectype, buf, sz)  \
  unpack_and_validate(o, name, array)                                         \
  validate_array_size(a, name, sz)                                            \
  M_BufferZero((buf));                                                        \
  M_BufferEnsureTotalSize(&(buf), a.size);                                    \
  for (int i = 0; i < a.size; i++) {                                          \
    msgpack_object *array_entry = a.ptr + i;                                  \
    validate_type_and_size(array_entry, ename, etype);                        \
    M_BufferAppend(buf, array_entry->via.raw.data, sizeof(ectype));           \
  }

#define unpack_and_validate_string_array(obj, arr, name, ename, obuf, sz)     \
  unpack_and_validate(obj, name, array)                                       \
  validate_array_size(arr, name, sz)                                          \
  M_OBufClear(obuf);                                                          \
  M_OBufEnsureSize(obuf, arr.size);                                           \
  for (int i = 0; i < arr.size; i++) {                                        \
    rune *buf_entry = NULL;                                                   \
    msgpack_object *array_entry = arr.ptr + i;                                \
    validate_type(array_entry, ename, raw);                                   \
    buf_entry = calloc(array_entry->via.raw.size + 1, sizeof(rune));          \
    memcpy(buf_entry, array_entry->via.raw.data, array_entry->via.raw.size);  \
    M_ObjBufferAppend(obuf, buf_entry);                                       \
  }

#define unpack_and_validate_player_array(obj, arr, name, buf, sz)             \
  unpack_and_validate(obj, name, array);                                      \
  validate_array_size(obj->via.array);                                        \
  m_recipient_count = obj->via.array.size;                                    \
  M_BufferZero(recipients);                                                   \
  M_BufferEnsureTotalCapacity(recipients, m_recipient_count * sizeof(short)); \
  for (int i = 0; i < m_recipient_count; i++) {                               \
    validate_player(obj->via.array.ptr[i]);                                   \
    M_BufferAppend(recipients, obj->via.array.ptr[i].data, sizeof(short));    \
  }

#define MAX_WAD_NAMES 1000

static msgpack_unpacker    pac;
static msgpack_unpacked    result;
static msgpack_object     *obj;
static msgpack_object_map *map = NULL;

void N_InitPacker(void) {
  msgpack_unpacker_init(&pac, MSGPACK_UNPACKER_INIT_BUFFER_SIZE);
  msgpack_unpacked_init(&result);
}

dboolean N_LoadNewMessage(netpeer_t *np, byte *message_type) {
  unpack_and_validate(obj, "message type", uchar);

  *message_type = (byte)obj->via.u64;

  return true;
}

void N_PackSetup(netpeer_t *np) {
  int index = -1;
  rune *resource_name = NULL;
  rune *deh_name = NULL;

  msgpack_pack_unsigned_char(np->rpk, nm_setup);
  msgpack_pack_unsigned_short(np->rpk, M_OBufGetObjectCount(&players));
  msgpack_pack_unsigned_short(np->rpk, np->playernum);

  msgpack_pack_array(np->rpk, M_OBufGetObjectCount(&resource_files));
  while (M_OBufIter(&resource_files, &index, &resource_name)) {
    size_t length = strlen(resource_name);

    msgpack_pack_raw(np->rpk, length);
    msgpack_pack_raw_body(np->rpk, resource_name, length);
  }

  msgpack_pack_array(np->rpk, M_OBufGetObjectCount(&deh_files));
  while (M_OBufIter(&deh_files, &index, &deh_name)) {
    size_t length = strlen(deh_name);

    msgpack_pack_raw(np->rpk, length);
    msgpack_pack_raw_body(np->rpk, deh_name, length);
  }
}

dboolean N_UnpackSetup(netpeer_t *np, unsigned short &player_count,
                                      unsigned short &playernum) {
  unsigned short m_player_count = 0;
  unsigned short m_playernum = 0;

  unpack_and_validate(obj, "player count", ushort);
  m_player_count = (unsigned short)obj->via.u64;

  unpack_and_validate(obj, "consoleplayer", ushort);
  m_playernum = (unsigned short)obj->via.u64;

  unpack_and_validate_string_array(
    obj->via.array,
    "resource names",
    "resource name",
    &resource_names,
    MAX_RESOURCE_NAMES
  );

  unpack_and_validate_string_array(
    obj->via.array,
    "deh names",
    "deh name",
    &deh_names,
    MAX_RESOURCE_NAMES
  );

  return true;
}

void N_PackFullState(netpeer_t *np, buf_t *buf) {
  msgpack_pack_unsigned_char(np->rpk, nm_gamestate);
  msgpack_pack_unsigned_int(np->rpk, gametic);
  msgpack_pack_raw(np->rpk, state->size);
  msgpack_pack_raw_body(np->rpk, state->data, state->size);
}

dboolean N_UnpackFullState(netpeer_t *np, int *tic, buf_t *buf) {
  int m_tic;

  unpack_and_validate(obj, "game state tic", int);
  m_tic = (int)obj->via.i64;

  unpack_and_validate(obj, "game state data", raw);

  *tic = m_tic;
  M_BufferSetData(buf, (byte *)obj->via.raw.ptr, (size_t)obj->via.raw.size);

  return true;
}

void N_PackStateDelta(netpeer_t *np, int from_tic, int to_tic, buf_t *buf) {
  msgpack_pack_unsigned_char(np->upk, nm_statedelta);
  msgpack_pack_int(np->upk, from_tic);
  msgpack_pack_int(np->upk, to_tic);
  msgpack_pack_raw(np->upk, delta->size);
  msgpack_pack_raw_body(np->upk, delta->data, delta->size);
}

dboolean N_UnpackStateDelta(netpeer_t *np, int *from_tic, int *to_tic,
                                           buf_t *buf) {
  int m_from_tic = -1;
  int m_to_tic = -1;

  unpack_and_validate(obj, "state delta start tic", int);
  m_from_tic = (int)obj->via.i64;

  unpack_and_validate(obj, "state delta end tic", int);
  m_to_tic = (int)obj->via.u64;

  unpack_and_validate(obj, "state delta data", raw);

  *from_tic = m_from_tic;
  *to_tic = m_to_tic;
  M_BufferSetData(buf, (byte *)obj->via.raw.ptr, (size_t)obj->via.raw.size);

  return true;
}

void N_PackAuthResponse(netpeer_t *np, auth_level_e auth_level) {
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

void N_PackServerMessage(netpeer_t *np, rune *message) {
  size_t length = strlen(message) * sizeof(rune);

  msgpack_pack_unsigned_char(np->rpk, nm_servermessage);
  msgpack_pack_raw(np->rpk, length);
  msgpack_pack_raw_body(np->rpk, message, length);
}

dboolean N_UnpackServerMessage(netpeer_t *np, buf_t *buf) {
  unpack_and_validate(obj, "server message content", raw);

  M_BufferSetString(buf, (rune *)obj->via.raw.ptr, (size_t)obj->via.raw.size);

  return true;
}

void N_PackPlayerMessage(netpeer_t *np, unsigned short sender,
                                        size_t recipient_count,
                                        buf_t *recipients,
                                        rune *message) {
  int i = 0;
  size_t length = strlen(message) * sizeof(rune);
  short *ra = (short *)recipients->data;

  msgpack_pack_unsigned_char(np->rpk, nm_playermessage);
  msgpack_pack_unsigned_short(np->rpk, sender);
  msgpack_pack_array(np->rpk, recipient_count);

  for (int i = 0; i < recipient_count; i++)
    msgpack_pack_short(ra[i]);

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

  unpack_and_validate(obj, "player message content", raw);

  M_BufferSetString(buf, (rune *)obj->via.raw.ptr, (size_t)obj->via.raw.size);

  *sender = m_sender;
  *recipient_count = m_recipient_count;

  return true;
}

void N_PackPlayerCommandReceived(netpeer_t *np, int tic) {
  msgpack_pack_unsigned_char(np->rpk, nm_playercommandreceived);
  msgpack_pack_int(tic);
}

dboolean N_UnpackPlayerCommandReceived(netpeer_t *np, int *tic) {
  unpack_and_validate(obj, "last player command tic received", int);

  *tic = (int)obj->via.u64;

  return true;
}

void N_PackPlayerCommands(netpeer_t *np, cbuf_t *commands) {
  int index = -1;
  netticcmd_t *ncmd = NULL;

  msgpack_pack_unsigned_char(np->upk, nm_playercommands);
  msgpack_pack_unsigned_char(np->upk, M_CBufGetObjectCount(commands));

  while (M_CBufIter(commands, &index, (void **)&ncmd)) {
    msgpack_pack_unsigned_int(np->upk, ncmd->tic);
    msgpack_pack_signed_char(np->upk, ncmd->forwardmove);
    msgpack_pack_signed_char(np->upk, ncmd->sidemove);
    msgpack_pack_short(np->upk, ncmd->angleturn);
    msgpack_pack_short(np->upk, ncmd->consistancy);
    msgpack_pack_unsigned_char(np->upk, chatchar);
    msgpack_pack_unsigned_char(np->upk, buttons);
  }
}

dboolean N_UnpackPlayerCommands(netpeer_t *np) {
  byte command_count = 0;

  unpack_and_validate(obj, "command count", uchar);
  M_ObjBufferEnsureSize(np->commands, command_count);

  for (byte i = 0; i < command_count; i++) {
    netticmd_t *ncmd = np->commands->objects[i];

    if (ncmd == NULL)
      ncmd = calloc(1, sizeof(netticcmd_t));
    else
      memset(cmd, 0, sizeof(netticcmd_t));

    unpack_and_validate(obj, "command tic", int);
    cmd->tic = (int)obj->via.i64;

    unpack_and_validate(obj, "command forward value", char);
    cmd->forwardmove = (signed char)obj->via.i64;

    unpack_and_validate(obj, "command side value", char);
    cmd->sidemove = (signed char)obj->via.i64;

    unpack_and_validate(obj, "command angle value", short);
    cmd->angle = (signed short)obj->via.i64;

    unpack_and_validate(obj, "command consistancy value", short);
    cmd->consistancy = (signed short)obj->via.i64;

    unpack_and_validate(obj, "command chatchar value", uchar);
    cmd->chatchar = (byte)obj->via.u64;

    unpack_and_validate(obj, "command buttons value", uchar);
    cmd->buttons = (byte)obj->via.u64;
  }

  return true;
}

void N_PackSaveGameNameChange(netpeer_t *np, rune *new_save_game_name) {
  size_t length = strlen(new_save_game_name) * sizeof(rune);

  msgpack_pack_unsigned_char(np->rpk, nm_savegamenamechange);
  msgpack_pack_raw(np->rpk, length);
  msgpack_pack_raw_body(np->rpk, new_save_game_name, length);
}

dboolean N_UnpackSaveGameNameChange(netpeer_t *np, buf_t *buf) {
  unpack_and_validate(obj, "new save game name", raw);

  M_BufferSetString(buf, (rune *)obj->via.raw.ptr, (size_t)obj->via.raw.size);

  return true;
}

dboolean N_UnpackPlayerPreferenceChange(netpeer_t *np, short *playernum,
                                                       size_t *count) {
  short m_playernum;

  unpack_and_validate_player(obj);
  m_playernum = (short)obj->via.i64;

  unpack_and_validate(obj, "player preference change map", map);

  map = &obj->via.map;

  *playernum = m_playernum;
  *count = map->size;

  return true;
}

dboolean N_UnpackPlayerPreferenceName(netpeer_t *np, size_t pref, buf_t *buf) {
  msgpack_object_kv *pair = NULL;
  
  if (pref >= map->size) {
    doom_printf(
      "N_UnpackPlayerPreferenceName: Attempted to index past preference "
      "count\n"
    );
    return false;
  }

  pair = map->ptr + pref;

  validate_type(&pair->key, "player preference name", raw);

  obj = &pair->val;

  M_BufferSetString(
    buf, (rune *)pair->key.via.raw.ptr, (size_t)pair->key.via.raw.size
  );

  return true;
}

void N_PackNameChange(netpeer_t *np, rune *new_name) {
  size_t length = strlen(new_name) * sizeof(rune);

  msgpack_pack_unsigned_char(np->rpk, nm_playerpreferencechange);
  msgpack_pack_map(np->rpk, 2);
  msgpack_pack_raw(np->rpk, 4);
  msgpack_pack_raw_body(np->rpk, "name");
  msgpack_pack_raw(np->rpk, length)
  msgpack_pack_raw_body(np->rpk, new_name, length);
}

dboolean N_UnpackNameChange(netpeer_t *np, buf_t *buf) {
  unpack_and_validate(obj, "new name", raw);

  M_BufferSetString(buf, (rune *)obj->via.raw.ptr, (size_t)obj->via.raw.size);

  return true;
}

void N_PackTeamChange(netpeer_t *np, byte new_team) {
  msgpack_pack_unsigned_char(np->rpk, nm_playerpreferencechange);
  msgpack_pack_map(np->rpk, 2);
  msgpack_pack_raw(np->rpk, 4);
  msgpack_pack_raw_body(np->rpk, "team");
  msgpack_pack_unsigned_char(np->rpk, new_team);
}

dboolean N_UnpackTeamChange(netpeer_t *np, byte *new_team) {
  /* CG: TODO: teams */
  if (team_count > 0) {
    unpack_and_validate_range(obj, "team index", uchar, 0, team_count - 1);
    *new_team = (byte)obj->via.u64;
  }
  else {
    *new_team = 0;
  }

  return true;
}

void N_PackPWOChange(netpeer_t *np) {
  msgpack_pack_unsigned_char(np->rpk, nm_playerpreferencechange);
  msgpack_pack_map(np->rpk, 2);
  msgpack_pack_raw(np->rpk, 3);
  msgpack_pack_raw_body(np->rpk, "pwo");
  /* CG: TODO */
}

dboolean N_UnpackPWOChange(netpeer_t *np) {
  /* CG: TODO */
  return false;
}

void N_PackWSOPChange(netpeer_t *np, byte new_wsop_flags) {
  msgpack_pack_unsigned_char(np->rpk, nm_playerpreferencechange);
  msgpack_pack_map(np->rpk, 2);
  msgpack_pack_raw(np->rpk, 4);
  msgpack_pack_raw_body(np->rpk, "wsop");
  msgpack_pack_unsigned_char(np->rpk, new_wsop_flags);
}

dboolean N_UnpackWSOPChange(netpeer_t *np, byte *new_wsop_flags) {
  unpack_and_validate_range(
    obj, "new WSOP value", uchar, 0, 2 << (WSOP_MAX - 2)
  );

  *new_wsop_flags = (byte)obj->via.u64;

  return true;
}

void N_PackBobbingChange(netpeer_t *np, double new_bobbing_amount) {
  msgpack_pack_unsigned_char(np->rpk, nm_playerpreferencechange);
  msgpack_pack_map(np->rpk, 2);
  msgpack_pack_raw(np->rpk, 7);
  msgpack_pack_raw_body(np->rpk, "bobbing");
  msgpack_pack_double(np->rpk, new_bobbing_amount);
}

dboolean N_UnpackBobbingchanged(netpeer_t *np, double *new_bobbing_amount) {
  unpack_and_validate_range(obj, "new bobbing amount", double, 0.0, 1.0);

  *new_bobbing_amount = obj->via.double;

  return true;
}

void N_PackAutoaimChange(netpeer_t *np, dboolean new_autoaim_enabled) {
  msgpack_pack_unsigned_char(np->rpk, nm_playerpreferencechange);
  msgpack_pack_map(np->rpk, 2);
  msgpack_pack_raw(np->rpk, 7);
  msgpack_pack_raw_body(np->rpk, "autoaim");
  if (new_autoaim_enabled)
    msgpack_pack_true(np->rpk);
  else
    msgpack_pack_false(np->rpk);
}

dboolean N_UnpackAutoaimChange(netpeer_t *np, dboolean *new_autoaim_enabled) {
  unpack_and_validate(obj, "new Autoaim value", bool);

  *new_autoaim_enabled = obj->via.bool;

  return true;
}

void N_PackWeaponSpeedChange(netpeer_t *np, byte new_weapon_speed) {
  msgpack_pack_unsigned_char(np->rpk, nm_playerpreferencechange);
  msgpack_pack_map(np->rpk, 2);
  msgpack_pack_raw(np->rpk, 12);
  msgpack_pack_raw_body(np->rpk, "weapon_speed");
  msgpack_pack_unsigned_char(np->rpk, new_autoaim_enabled);
}

dboolean N_UnpackWeaponSpeedChange(netpeer_t *np, byte *new_weapon_speed) {

  unpack_and_validate(obj, "new weapon speed value", uchar);

  *new_weapon_speed = (byte)obj->via.u64;

  return true;
}

void N_PackColorChange(netpeer_t *np, byte new_red, byte new_green,
                                      byte new_blue) {
  msgpack_pack_unsigned_char(np->rpk, nm_clientpreferencechange);
  msgpack_pack_map(np->rpk, 2);
  msgpack_pack_raw(np->rpk, 5);
  msgpack_pack_raw_body(np->rpk, "color");
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

void N_PackColormapChange(netpeer_t *np, int new_color) {
  msgpack_pack_unsigned_char(np->rpk, nm_playerpreferencechange);
  if (SERVER)
    msgpack_pack_short(np->rpk, np->playernum);
  else
    msgpack_pack_short(np->rpk, consoleplayer);
  msgpack_pack_map(np->rpk, 2);
  msgpack_pack_raw(np->rpk, 8);
  msgpack_pack_raw_body(np->rpk, "colormap");
  msgpack_pack_int(np->rpk, new_color);
}

dboolean N_UnpackColormapChange(netpeer_t *np, short *playernum,
                                               int *new_color) {
  int   m_tic = -1;
  short m_playernum = -1;
  int   m_new_color = -1;

  unpack_and_validate(obj, "tic", int);
  m_tic = (int)obj->via.i64;

  unpack_and_validate_player(obj);
  m_playernum = (short)obj->via.i64;

  unpack_and_validate(obj, "new color", uchar);

  /* CG: TODO: Validate color value is reasonable */
  m_new_color = (int)obj->via.i64;

  *tic = m_tic;
  *playernum = m_playernum;
  *new_color = m_new_color;

  return true;
}

void N_PackSkinChange(netpeer_t *np) {
  msgpack_pack_unsigned_char(np->rpk, nm_clientpreferencechange);
  msgpack_pack_map(np->rpk, 2);
  msgpack_pack_raw(np->rpk, 9);
  msgpack_pack_raw_body(np->rpk, "skin_name");
  /* CG: TODO */
}

dboolean N_UnpackSkinChange(netpeer_t *np) {
  /* CG: TODO */
  return false;
}

void N_PackStateReceived(netpeer_t *np, int tic) {
  msgpack_pack_unsigned_char(np->rpk, nm_statereceived);
  msgpack_pack_int(tic);
}

dboolean N_UnpackStateReceived(netpeer_t *np, int *tic) {
  unpack_and_validate(obj, "last state tic received", int);

  *tic = (int)obj->via.u64;

  return true;
}

void N_PackAuthRequest(netpeer_t *np, rune *password) {
  size_t length = strlen(password) * sizeof(rune);

  msgpack_pack_unsigned_char(np->rpk, nm_authrequest);
  msgpack_pack_raw(np->rpk, length);
  msgpack_pack_raw_body(np->rpk, password, length);
}

dboolean N_UnpackAuthRequest(netpeer_t *np, buf_t *buf) {
  unpack_and_validate(obj, "authorization request password", raw);

  M_BufferSetString(buf, (rune *)obj->via.raw.ptr, (size_t)obj->via.raw.size);

  return true;
}

void N_PackRCONCommand(netpeer_t *np, rune *command) {
  size_t length = strlen(command) * sizeof(rune);

  msgpack_pack_unsigned_char(np->rpk, nm_rconcommand);
  msgpack_pack_raw(np->rpk, length);
  msgpack_pack_raw_body(np->rpk, command, length);
}

dboolean N_UnpackRCONCommand(netpeer_t *np, buf_t *buf) {
  unpack_and_validate(obj, "RCON command", raw);

  M_BufferSetString(buf, (rune *)obj->via.raw.ptr, (size_t)obj->via.raw.size);

  return true;
}

void N_PackVoteRequest(netpeer_t *np, rune *command) {
  size_t length = strlen(command) * sizeof(rune);

  msgpack_pack_unsigned_char(np->rpk, nm_voterequest);
  msgpack_pack_raw(np->rpk, length);
  msgpack_pack_raw_body(np->rpk, command, length);
}

dboolean N_UnpackVoteRequest(netpeer_t *np, buf_t *buf) {
  unpack_and_validate(obj, "vote request command", raw);

  M_BufferSetString(buf, (rune *)obj->via.raw.ptr, (size_t)obj->via.raw.size);

  return true;
}

/* vi: set et ts=2 sw=2: */

