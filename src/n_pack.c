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

#define mp_uint_name "an unsigned integer"
static dboolean mp_uint(msgpack_object *o) {
  return o->type == MSGPACK_OBJECT_POSITIVE_INTEGER;
}

#define mp_sint_name "a signed integer"
static dboolean mp_sint(msgpack_object *o) {
  return ((o->type == MSGPACK_OBJECT_POSITIVE_INTEGER) ||
          (o->type == MSGPACK_OBJECT_NEGATIVE_INTEGER));
}

#define mp_nint_name "a negative integer"
static dboolean mp_nint(msgpack_object *o) {
  return o->type == MSGPACK_OBJECT_NEGATIVE_INTEGER;
}

#define mp_null_name "a NULL value"
static dboolean mp_null(msgpack_object *o) {
  return o->type == MSGPACK_OBJECT_NIL;
}

#define mp_bool_name "a boolean"
static dboolean mp_bool(msgpack_object *o) {
  return o->type == MSGPACK_OBJECT_BOOLEAN;
}

#define mp_double_name "a double"
static dboolean mp_double(msgpack_object *o) {
  return o->type == MSGPACK_OBJECT_DOUBLE;
}

#define mp_raw_name "raw bytes"
static dboolean mp_raw(msgpack_object *o) {
  return o->type == MSGPACK_OBJECT_RAW;
}

#define mp_array_name "an array"
static dboolean mp_array(msgpack_object *o) {
  return o->type == MSGPACK_OBJECT_ARRAY;
}

#define mp_map_name "a map"
static dboolean mp_map(msgpack_object *o) {
  return o->type == MSGPACK_OBJECT_MAP;
}

#define mp_byte_size_name "0xFF"
static dboolean mp_byte_size(msgpack_object *o) {
  return o->via.u64 <= 0xFF;
}

#define mp_byte_size_name "0xFFFF"
static dboolean mp_short_size(msgpack_object *o) {
  return o->via.u64 <= 0xFFFF;
}

#define mp_byte_size_name "0xFFFFFFFF"
static dboolean mp_int_size(msgpack_object *o) {
  return o->via.u64 <= 0xFFFFFFFF;
}

static dboolean mp_int_range(msgpack_object *o, int min, int max) {
  return obj->via.i64 <= min || obj->via.i64 >= max;
}

static dboolean mp_uint_range(msgpack_object *o, unsigned int min,
                                                 unsigned int max) {
  return obj->via.u64 <= min || obj->via.u64 >= max;
}

static dboolean mp_double_range(msgpack_object *o, double min, double max) {
  return obj->via.double <= min || obj->via.double >= max;
}

#define unpack(s) \
  if (!msgpack_unpacker_next(&pac, &result)) {                                \
    doom_printf(__func__ ": Error unpacking " s "\n");                        \
    return false;                                                             \
  }                                                                           \

#define validate(s, o, t)                                                     \
  if (!(t((o)->type))) {                                                      \
    doom_printf(__func__ ": Invalid packet: " s " is not " t ## _name "\n");  \
    return false;                                                             \
  }

#define validate_size(s, o, t)                                                \
  if (!(t((o)))) {                                                            \
    doom_printf(__func__ ": " s " out of range (> " t ## _name ")\n");        \
    return false;                                                             \
  }

#define unpack_and_validate(s, t)                                             \
  unpack("last player command tic");                                          \
  validate("last player command tic", &result.data, mp_uint);                 \
  obj = &result.data

#define unpack_and_validate_array(as, s, mt, t, ob)                           \
  unpack_and_validate(as, mp_array)                                           \
  M_ObjBufferClear((ob));                                                     \
  M_ObjBufferEnsureSize(&(ob), obj->via.array.size);                          \
  for (int i = 0; i < obj->via.array.size; i++) {                             \
    t *buf_entry = NULL;                                                      \
    msgpack_object *array_entry = obj->via.array.ptr + i;                     \
    if (!mt(array_entry)) {                                                   \
      doom_printf(                                                            \
        __func__ ": Invalid packet: " s " is not " mt ## _name "\n"           \
      );                                                                      \
      M_ObjBufferFreeEntriesAndClear((ob));                                   \
      return false;                                                           \
    }                                                                         \
    buf_entry = malloc(array_entry->via.raw.size * sizeof(t));                \
    memcpy(buf_entry, array_entry->via.raw.data, array_entry->via.raw.size);  \
    M_ObjBufferAppend((ob), buf_entry);                                       \
  }

#define unpack_and_validate_string_array(as, s, ob)                           \
  unpack_and_validate(as, mp_array)                                           \
  M_ObjBufferClear((ob));                                                     \
  M_ObjBufferEnsureSize(&(ob), obj->via.array.size);                          \
  for (int i = 0; i < obj->via.array.size; i++) {                             \
    msgpack_object *array_entry = obj->via.array.ptr + i;                     \
    if (!mp_raw(array_entry)) {                                               \
      doom_printf(                                                            \
        __func__ ": Invalid packet: " s " is not " mp_raw_name "\n"           \
      );                                                                      \
      M_ObjBufferFreeEntriesAndClear((ob));                                   \
      return false;                                                           \
    }                                                                         \
    rune *buf_entry = calloc(array_entry->via.raw.size + 1, sizeof(rune));    \
    memcpy(buf_entry, array_entry->via.raw.data, array_entry->via.raw.size);  \
    M_ObjBufferAppend((ob), buf_entry);                                       \
  }

#define validate_raw_size(s, sz)                                              \
  if (obj->via.raw.size != sz) {                                              \
    doom_printf(__func__ ": Invalid packet, " s " size is not %u", sz);       \
    return false;                                                             \
  }

#define validate_is_byte(s)                                                   \
  if (!mp_byte_size(obj)) {                                                   \
    doom_printf(__func__ ": " s " out of range (> 0xFF)\n");                  \
    return false;                                                             \
  }

#define validate_is_short(s)                                                  \
  if (!mp_short_size(obj)) {                                                  \
    doom_printf(__func__ ": " s " out of range (> 0xFFFF)\n");                \
    return false;                                                             \
  }

#define validate_is_int(s)                                                    \
  if (!mp_int_size(obj)) {                                                    \
    doom_printf(__func__ ": " s " out of range (> 0xFFFFFFFF)\n");            \
    return false;                                                             \
  }

#define validate_range_unsigned(s, min, max, s)                               \
  if (!mp_uint_range(obj->via.u64, min, max)) {                               \
    doom_printf(__func__ ": %s out of range (%u, %u)\n", s, min, max);        \
    return false;                                                             \
  }

#define validate_range_signed(min, max, s)                                    \
  if (!mp_int_range(obj->via.u64, min, max)) {                                \
    doom_printf(__func__ ": %s out of range (%d, %d)\n", s, min, max);        \
    return false;                                                             \
  }

#define validate_range_double(min, max, s)                                    \
  if (obj->via.dec < min || obj->via.dec > max) {                             \
    doom_printf(__func__ ": %s out of range (%f, %f)\n", s, min, max);        \
    return false;                                                             \
  }

#define validate_player(pn)                                                   \
  if (!M_ObjBufferIsIndexValid(pn)) {                                         \
    doom_printf(__func__ ": Invalid player number\n");                        \
    return false;                                                             \
  }                                                                           \
  if (!mp_short_size(pn)) {                                                   \
    doom_printf(__func__ ": player number out of range (> 0xFFFF)\n");        \
    return false;                                                             \
  }

#define SERVER_ONLY(s)                                                        \
  if (!server) {                                                              \
    doom_printf(                                                              \
      __func__ ": Erroneously received packet [" s "] from the server\n"     \
    );                                                                        \
    return;                                                                   \
  }

#define CLIENT_ONLY(s)                                                        \
  if (server) {                                                               \
    doom_printf(                                                              \
      __func__ ": Erroneously received packet [" s "] from a client\n"       \
    );                                                                        \
    return;                                                                   \
  }

static msgpack_unpacker    pac;
static msgpack_unpacked    result;
static msgpack_object     *obj;
static msgpack_object_map *map = NULL;

void N_InitPacker(void) {
  msgpack_unpacker_init(&pac, MSGPACK_UNPACKER_INIT_BUFFER_SIZE);
  msgpack_unpacked_init(&result);
}

dboolean N_LoadNewMessage(netpeer_t *np, byte *message_type) {
  unpack_and_validate("message type", mp_uint);
  validate_is_byte("message type");

  *message_type = (byte)obj->via.u64;

  return true;
}

void N_PackSetup(netpeer_t *np, setup_packet_t *sinfo, buf_t *wad_names) {
  int offset = 0;

  msgpack_pack_unsigned_char(np->rpk, nm_setup);
  msgpack_pack_short(np->rpk, sinfo->players);
  msgpack_pack_short(np->rpk, sinfo->yourplayer);
  msgpack_pack_unsigned_char(np->rpk, sinfo->skill);
  msgpack_pack_unsigned_char(np->rpk, sinfo->episode);
  msgpack_pack_unsigned_char(np->rpk, sinfo->level);
  msgpack_pack_unsigned_char(np->rpk, sinfo->deathmatch);
  msgpack_pack_unsigned_char(np->rpk, sinfo->complevel);
  msgpack_pack_unsigned_char(np->rpk, sinfo->ticdup);
  msgpack_pack_unsigned_char(np->rpk, sinfo->extratic);
  msgpack_pack_raw(np->rpk, GAME_OPTIONS_SIZE);
  msgpack_pack_raw_body(np->rpk, sinfo->game_options);
  msgpack_pack_array(np->rpk, sinfo->numwads);
  for (int i = 0; i < sinfo->numwads; i++) {
    size_t length = strlen(wad_names->data + offset);

    msgpack_pack_raw(np->rpk, length);
    msgpack_raw_body(np->rpk, wad_names->data + offset);

    offset += length + 2;
  }
}

dboolean N_UnpackSetup(netpeer_t *np, setup_packet_t *sinfo,
                                      objbuf_t *wad_names) {
  short players;
  short yourplayer;
  byte  skill;
  byte  episode;
  byte  level;
  byte  deathmatch;
  byte  complevel;
  byte  ticdup;
  byte  extratic;
  byte  game_options[GAME_OPTIONS_SIZE];
  byte  numwads;

  unpack_and_validate("players", mp_uint);
  validate_is_short("players");
  players = (short)obj->via.i64;

  unpack_and_validate("yourplayer", mp_uint);
  validate_is_short("yourplayer");
  yourplayer = (short)obj->via.i64;

  unpack_and_validate("skill", mp_uint);
  validate_is_byte("skill");
  skill = (byte)obj->via.u64;

  unpack_and_validate("episode", mp_uint);
  validate_is_byte("episode");
  episode = (byte)obj->via.u64;

  unpack_and_validate("level", mp_uint);
  validate_is_byte("level");
  level = (byte)obj->via.u64;

  unpack_and_validate("deathmatch", mp_uint);
  validate_is_byte("deathmatch");
  deathmatch = (byte)obj->via.u64;

  unpack_and_validate("complevel", mp_uint);
  validate_is_byte("complevel");
  complevel = (byte)obj->via.u64;

  unpack_and_validate("ticdup", mp_uint);
  validate_is_byte("ticdup");
  ticdup = (byte)obj->via.u64;

  unpack_and_validate("extratic", mp_uint);
  validate_is_byte("extratic");
  extratic = (byte)obj->via.u64;

  unpack_and_validate("game options", mp_raw);
  validate_raw_size("game options", GAME_OPTIONS_SIZE);
  memcpy(game_options, (byte)obj->via.raw.ptr, GAME_OPTIONS_SIZE);

  unpack_and_validate_string_array("WAD names", "WAD name", wad_names);

  sinfo->players      = players;
  sinfo->yourplayer   = yourplayer;
  sinfo->skill        = skill;
  sinfo->episode      = episode;
  sinfo->level        = level;
  sinfo->deathmatch   = deathmatch;
  sinfo->complevel    = complevel;
  sinfo->ticdup       = ticdup;
  sinfo->extratic     = extratic;
  sinfo->numwads      = wad_names->size;

  memcpy(sinfo->game_options, game_options, GAME_OPTIONS_SIZE);

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

  unpack_and_validate("state delta start tic", mp_uint);
  validate_is_int("state delta start tic");
  m_from_tic = (int)obj->via.u64;

  unpack_and_validate("state delta end tic", mp_uint);
  validate_is_int("state delta end tic");
  m_to_tic = (int)obj->via.u64;

  unpack_and_validate("state delta data", mp_raw);

  *from_tic = m_from_tic;
  *tic_ti = m_to_tic;
  M_BufferSetData(buf, (byte *)obj->via.raw.ptr, (size_t)obj->via.raw.size);

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

  unpack_and_validate("game state tic", mp_uint);
  m_tic = (int)obj->via.u64;

  unpack_and_validate("game state data", mp_raw);

  *tic = m_tic;
  M_BufferSetData(buf, (byte *)obj->via.raw.ptr, (size_t)obj->via.raw.size);

  return true;
}

void N_PackTicMarker(netpeer_t *np, int tic) {
  msgpack_pack_unsigned_char(np->rpk, nm_ticmarker);
  msgpack_pack_int(np->rpk, tic);
}

dboolean N_UnpackTicMarker(netpeer_t *np, int *tic) {
  unpack_and_validate("tic", mp_uint);
  validate_is_int("tic");

  *tic = (int)obj->via.u64;

  return true;
}

void N_PackPlayerCommandReceived(netpeer_t *np, int tic) {
  msgpack_pack_unsigned_char(np->rpk, nm_playercommandreceived);
  msgpack_pack_int(tic);
}

dboolean N_UnpackPlayerCommandReceived(netpeer_t *np, int *tic) {
  unpack_and_validate("last player command tic received", mp_uint);
  validate_is_int("last player command tic received");

  *tic = (int)obj->via.u64;

  return true;
}

void N_PackAuthResponse(netpeer_t *np, auth_level_e auth_level) {
  msgpack_pack_unsigned_char(np->rpk, nm_authresponse);
  msgpack_pack_unsigned_char(np->rpk, auth_level);
}

dboolean N_UnpackAuthResponse(netpeer_t *np, auth_level_e *auth_level) {
  unpack_and_validate("authorization level", mp_uint);
  validate_range_signed(0, AUTH_LEVEL_MAX - 1);

  *auth_level = (auth_level_e)obj->via.u64;

  return true;
}

void N_PackSaveGameName(netpeer_t *np, rune *new_save_game_name) {
  size_t length = strlen(new_save_game_name);

  msgpack_pack_unsigned_char(np->rpk, PKT_SAVEG);
  msgpack_pack_int(np->rpk, gametic);
  msgpack_pack_raw(np->rpk, length);
  msgpack_pack_raw_body(np->rpk, new_save_game_name, length);
}

dboolean N_UnpackSaveGameName(netpeer_t *np, int *tic, buf_t *buf) {
  int m_tic = 0;

  unpack_and_validate("tic", mp_uint);
  validate_is_int("tic");
  m_tic = (int)obj->via.i64;

  unpack_and_validate("save game name", mp_raw);

  *tic = m_tic;
  M_BufferSetString(buf, (rune *)obj->via.raw.ptr, (size_t)obj->via.raw.size);

  return true;
}

void N_PackServerMessage(netpeer_t *np, rune *message) {
  size_t length = strlen(message) * sizeof(rune);

  msgpack_pack_unsigned_char(np->rpk, nm_servermessage);
  msgpack_pack_raw(np->rpk, length);
  msgpack_pack_raw_body(np->rpk, message, length);
}

dboolean N_UnpackServerMessage(netpeer_t *np, buf_t *buf) {
  unpack_and_validate("server message content", mp_raw);

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

  /*
   * CG: TODO: Add player validation.  All recipients should be valid players
   *           as should the sender.
   */

  unpack_and_validate("player message sender", mp_uint);
  validate_is_short("player message sender");
  m_sender = (unsigned short)obj->via.u64;

  unpack_and_validate("player message recipients", mp_array);
  m_recipient_count = obj->via.array.size;

  if (recipient_count > 0xFF) {
    doom_printf(
      "N_UnpackPlayerMessage: player message recipient count out of range
      (> 0xFF)\n"
    );
    return false;
  }

  M_BufferEnsureTotalCapacity(recipients, recipient_count * sizeof(short));
  M_BufferZero(recipients);

  for (int i = 0; i < recipient_count; i++) {
    short recipient = 0;

    if (!mp_sint(obj->via.array.ptr[i])) {
      doom_printf(
        "N_UnpackPlayerMessage: Invalid packet: player message recipient is "
        "not a signed integer\n"
      );
      M_BufferZero(recipients);
      return false;
    }

    if (obj->via.array.ptr[i]->via.u64 > 0xFF) {
      doom_printf(
        "N_UnpackPlayerMessage: player message recipient out of range "
        "(> 0xFF)\n"
      );
      M_BufferZero(recipients);
      return false;
    }

    ((short *)recipients->data)[i] = obj->via.array.ptr[i]->via.i64;
  }

  unpack_and_validate("player message content", mp_raw);

  M_BufferSetString(buf, (rune *)obj->via.raw.ptr, (size_t)obj->via.raw.size);

  *sender = m_sender;
  *recipient_count = m_recipient_count;

  return true;
}

void N_PackPlayerCommands(netpeer_t *np) {
  msgpack_pack_unsigned_char(np->upk, nm_playercommands);
  msgpack_pack_unsigned_char(np->upk, M_ObjBufferGetObjectCount(np->commands));

  for (int i = 0; i < commands->size; i++) {
    netticcmd_t *cmd = commands->objects[i];

    if (cmd == NULL)
      continue;

    msgpack_pack_unsigned_int(np->upk, cmd->tic);
    msgpack_pack_signed_char(np->upk, cmd->forwardmove);
    msgpack_pack_signed_char(np->upk, cmd->sidemove);
    msgpack_pack_short(np->upk, cmd->angleturn);
    msgpack_pack_short(np->upk, cmd->consistancy);
    msgpack_pack_unsigned_char(np->upk, chatchar);
    msgpack_pack_unsigned_char(np->upk, buttons);
  }
}

dboolean N_UnpackPlayerCommands(netpeer_t *np) {
  byte command_count = 0;

  unpack_and_validate("command count", mp_uint);
  validate_is_byte("command_count");
  M_ObjBufferEnsureSize(np->commands, command_count);

  for (byte i = 0; i < command_count; i++) {
    netticmd_t *cmd = np->commands->objects[i];

    if (cmd == NULL)
      cmd = calloc(1, sizeof(netticcmd_t));
    else
      memset(cmd, 0, sizeof(netticcmd_t));

    unpack_and_validate("command tic", mp_uint);
    validate_is_int("command tic");
    cmd->tic = (int)obj->via.u64;

    unpack_and_validate("command forward value", mp_sint);
    validate_is_byte("command forward value");
    cmd->forwardmove = (signed char)obj->via.i64;

    unpack_and_validate("command side value", mp_sint);
    validate_is_byte("command side value");
    cmd->sidemove = (signed char)obj->via.i64;

    unpack_and_validate("command angle value", mp_sint);
    validate_is_short("command angle value");
    cmd->angle = (signed short)obj->via.i64;

    unpack_and_validate("command consistancy value", mp_sint);
    validate_is_short("command consistancy value");
    cmd->consistancy = (signed short)obj->via.i64;

    unpack_and_validate("command chatchar value", mp_uint);
    validate_is_byte("command chatchar value");
    cmd->chatchar = (byte)obj->via.u64;

    unpack_and_validate("command buttons value", mp_uint);
    validate_is_byte("command buttons value");
    cmd->buttons = (byte)obj->via.u64;
  }

  return true;
}

dboolean N_UnpackClientPreferenceChange(netpeer_t *np, short *playernum,
                                                       size_t *count) {
  short m_playernum;

  unpack_and_validate("client preference change player", mp_uint);
  validate_is_short("client preference change player");
  m_playernum = (short)obj->via.u64;

  unpack_and_validate("client preference change map", mp_map);

  map = &obj->via.map;

  *playernum = m_playernum;
  *count = map->size;

  return true;
}

dboolean N_UnpackClientPreferenceName(netpeer_t *np, size_t pref, buf_t *buf) {
  msgpack_object_kv *pair = NULL;
  
  if (pref >= map->size) {
    doom_printf(
      "N_UnpackClientPreferenceName: Attempted to index past preference "
      "count\n"
    );
    return false;
  }

  pair = map->ptr + pref;

  validate("client preference name", &pair->key, mp_raw);

  M_BufferSetString(
    buf, (rune *)pair->key.via.raw.ptr, (size_t)pair->key.via.raw.size
  );

  obj = &pair->val;

  return true;
}

void N_PackNameChange(netpeer_t *np, rune *new_name) {
  size_t length = strlen(new_name) * sizeof(rune);

  msgpack_pack_unsigned_char(np->rpk, nm_clientpreferencechange);
  msgpack_pack_map(np->rpk, 2);
  msgpack_pack_raw(np->rpk, 4);
  msgpack_pack_raw_body(np->rpk, "name");
  msgpack_pack_raw(np->rpk, length)
  msgpack_pack_raw_body(np->rpk, new_name, length);
}

dboolean N_UnpackNameChange(netpeer_t *np, buf_t *buf) {
  unpack_and_validate("new name", mp_raw);

  M_BufferSetString(buf, (rune *)obj->via.raw.ptr, (size_t)obj->via.raw.size);

  return true;
}

void N_PackTeamChange(netpeer_t *np, byte new_team) {
  msgpack_pack_unsigned_char(np->rpk, nm_clientpreferencechange);
  msgpack_pack_map(np->rpk, 2);
  msgpack_pack_raw(np->rpk, 4);
  msgpack_pack_raw_body(np->rpk, "team");
  msgpack_pack_unsigned_char(np->rpk, new_team);
}

dboolean N_UnpackTeamChange(netpeer_t *np, byte *new_team) {
  unpack_and_validate("team index", mp_uint);
  /* CG: TODO: teams */
  if (team_count > 0)
    validate_range_unsigned(0, team_count - 1);
  else
    validate_range_unsigned(0, 0);

  *new_team = (byte)obj->via.u64;

  return true;
}

void N_PackPWOChange(netpeer_t *np) {
  msgpack_pack_unsigned_char(np->rpk, nm_clientpreferencechange);
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
  msgpack_pack_unsigned_char(np->rpk, nm_clientpreferencechange);
  msgpack_pack_map(np->rpk, 2);
  msgpack_pack_raw(np->rpk, 4);
  msgpack_pack_raw_body(np->rpk, "wsop");
  msgpack_pack_unsigned_char(np->rpk, new_wsop_flags);
}

dboolean N_UnpackWSOPChange(netpeer_t *np, byte *new_wsop_flags) {
  unpack_and_validate("new WSOP value", mp_uint);
  validate_range_unsigned(0, 2 << (WSOP_MAX - 2));

  *new_wsop_flags = (byte)obj->via.u64;

  return true;
}

void N_PackBobbingChange(netpeer_t *np, double new_bobbing_amount) {
  msgpack_pack_unsigned_char(np->rpk, nm_clientpreferencechange);
  msgpack_pack_map(np->rpk, 2);
  msgpack_pack_raw(np->rpk, 7);
  msgpack_pack_raw_body(np->rpk, "bobbing");
  msgpack_pack_double(np->rpk, new_bobbing_amount);
}

dboolean N_UnpackBobbingchanged(netpeer_t *np, double *new_bobbing_amount) {
  unpack_and_validate("new bobbing amount", DOUBLE);
  validate_range_double(0.0, 1.0);

  *new_bobbing_amount = obj->via.double;

  return true;
}

void N_PackAutoaimChange(netpeer_t *np, dboolean new_autoaim_enabled) {
  msgpack_pack_unsigned_char(np->rpk, nm_clientpreferencechange);
  msgpack_pack_map(np->rpk, 2);
  msgpack_pack_raw(np->rpk, 7);
  msgpack_pack_raw_body(np->rpk, "autoaim");
  if (new_autoaim_enabled)
    msgpack_pack_true(np->rpk);
  else
    msgpack_pack_false(np->rpk);
}

dboolean N_UnpackAutoaimChange(netpeer_t *np, dboolean *new_autoaim_enabled) {
  unpack_and_validate("new Autoaim value", mp_bool);

  *new_autoaim_enabled = obj->via.bool;

  return true;
}

void N_PackWeaponSpeedChange(netpeer_t *np, byte new_weapon_speed) {
  msgpack_pack_unsigned_char(np->rpk, nm_clientpreferencechange);
  msgpack_pack_map(np->rpk, 2);
  msgpack_pack_raw(np->rpk, 12);
  msgpack_pack_raw_body(np->rpk, "weapon_speed");
  msgpack_pack_unsigned_char(np->rpk, new_autoaim_enabled);
}

dboolean N_UnpackWeaponSpeedChange(netpeer_t *np, byte *new_weapon_speed) {

  unpack_and_validate("new weapon speed value", mp_uint);
  validate_is_byte("new weapon speed value");

  *new_weapon_speed = (byte)obj->via.u64;

  return true;
}

void N_PackColormapChange(netpeer_t *np, mapcolor_me new_color) {
  msgpack_pack_unsigned_char(np->rpk, PKT_COLOR);
  if (server)
    msgpack_pack_short(np->rpk, np->playernum);
  else
    msgpack_pack_short(np->rpk, consoleplayer);
  msgpack_pack_int(np->rpk, new_color);
}

dboolean N_UnpackColormapChange(netpeer_t *np, short *playernum,
                                               mapcolor_me *new_color) {
  int m_tic = -1;
  short m_playernum = -1;
  mapcolor_me m_new_color;

  unpack_and_validate("tic", mp_uint);
  validate_is_int("tic");
  m_tic = (int)obj->via.u64;

  unpack_and_validate("player", mp_uint);
  validate_is_short("player");
  m_playernum = (short)obj->via.i64;

  unpack_and_validate("new color", mp_uint);
  validate_is_int("new color");

  /* CG: TODO: Validate color value is within the mapcolor_me enum */
  m_new_color = (mapcolor_me)obj->via.i64;

  *tic = m_tic;
  *playernum = m_playernum;
  *new_color = m_new_color;

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
                                     (new_blue  << 8));
}

dboolean N_UnpackColorChange(netpeer_t *np, byte *new_red, byte *new_green,
                                            byte *new_blue) {
  unpack_and_validate("new color", mp_uint);
  validate_is_int("new color");

  *new_red   = obj->via.u64 >> 24;
  *new_green = obj->via.u64 >> 16;
  *new_blue  = obj->via.u64 >> 8;

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

void N_PackAuthRequest(netpeer_t *np, rune *password) {
  size_t length = strlen(password) * sizeof(rune);

  msgpack_pack_unsigned_char(np->rpk, nm_authrequest);
  msgpack_pack_raw(np->rpk, length);
  msgpack_pack_raw_body(np->rpk, password, length);
}

dboolean N_UnpackAuthRequest(netpeer_t *np, buf_t *buf) {
  unpack_and_validate("authorization request password", mp_raw);

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
  unpack_and_validate("RCON command", mp_raw);

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
  unpack_and_validate("vote request command", mp_raw);

  M_BufferSetString(buf, (rune *)obj->via.raw.ptr, (size_t)obj->via.raw.size);

  return true;
}

void N_PackClientTic(netpeer_t *np, int tic, objbuf_t *commands) {
  int command_count = M_ObjBufferGetObjectCount(commands);

  msgpack_pack_unsigned_char(np->rpk, PKT_TICC);
  msgpack_pack_unsigned_char(np->rpk, tic);
  msgpack_pack_int(np->rpk, command_count);
  for (int i = 0; i < command_count; i++) {
    ticcmd_t *cmd = commands->objects[i];

    msgpack_pack_signed_char(np->rpk, cmd->forwardmove, sizeof(signed char));
    msgpack_pack_signed_char(np->rpk, cmd->sidemove, sizeof(signed char));
    msgpack_pack_short(np->rpk, cmd->angleturn, sizeof(signed short));
    msgpack_pack_short(np->rpk, cmd->consistancy, sizeof(short));
    msgpack_pack_byte(np->rpk, cmd->chatchar, sizeof(byte));
    msgpack_pack_byte(np->rpk, cmd->buttons, sizeof(byte));
  }
}

dboolean N_UnpackClientTic(netpeer_t *np, int *tic, objbuf_t *commands) {
  int m_tic = 0;
  short command_count = 0;

  unpack_and_validate("tic", mp_uint);
  m_tic = (int)obj->via.u64;

  unpack_and_validate("command count", mp_uint);
  command_count = (short)obj->via.i64;

  /*
   * CG: FIXME: Put a limit on command_count to avoid pegging the CPU.  The
   *            The 'unpack_and_validate' check probably already prevents
   *            reading past the edge of the commands array, but CPU usage is a
   *            valid concern and it's good to be explicit nonetheless.
   */

  M_ObjBufferClear(commands);

  /*
   * CG: FIXME: There's no way to make sure the sparse commands object buffer
   *            is large enough without knowing how many total players there
   *            are.
   *
   *  M_ObjBufferEnsureSize(commands, command_count);
   *
   */

  for (int i = 0; i < command_count; i++) {
    short playernum = 0;
    ticcmd_t cmd;

    unpack_and_validate("player", mp_uint);
    validate_is_short("player");
    playernum = (short)obj->via.u64;

    M_ObjBufferEnsureSize(playernum + 1);

    unpack_and_validate("forwardmove", mp_sint);
    validate_is_byte("forwardmove");
    cmd.forwardmove = (signed char)obj->via.i64;

    unpack_and_validate("sidemove", mp_sint);
    validate_is_byte("sidemove");
    cmd.sidemove = (signed char)obj->via.i64;

    unpack_and_validate("angleturn", mp_sint);
    validate_is_byte("angleturn");
    cmd.angleturn = (short)obj->via.i64;

    unpack_and_validate("consistancy", mp_sint);
    validate_is_byte("consistancy");
    cmd.consistancy = (short)obj->via.i64;

    unpack_and_validate("chatchar", mp_uint);
    validate_is_byte("chatchar");
    cmd.chatchar = (byte)obj->via.u64;

    unpack_and_validate("buttons", mp_uint);
    validate_is_byte("buttons");
    cmd.buttons = (byte)obj->via.u64;

    memcpy(commands->objects[playernum], &cmd, sizeof(ticcmd_t));
  }

  *tic = m_tic;

  return true;
}

void N_PackServerTic(netpeer_t *np, int tic, objbuf_t *commands) {
  int players_this_tic = M_ObjBufferGetObjectCount(commands);

  msgpack_pack_unsigned_char(np->rpk, PKT_TICS);
  msgpack_pack_int(np->rpk, tic);
  msgpack_pack_int(np->rpk, M_ObjBufferGetObjectCount(commands));

  for (int i = 0; i < commands->size; i++) {
    ticcmd_t *cmd = commands->objects[i];

    if (cmd == NULL)
      continue;

    msgpack_pack_unsigned_short(np->rpk, i);
    msgpack_pack_signed_char(np->rpk, cmd->forwardmove, sizeof(signed char));
    msgpack_pack_signed_char(np->rpk, cmd->sidemove, sizeof(signed char));
    msgpack_pack_short(np->rpk, cmd->angleturn, sizeof(signed short));
    msgpack_pack_short(np->rpk, cmd->consistancy, sizeof(short));
    msgpack_pack_byte(np->rpk, cmd->chatchar, sizeof(byte));
    msgpack_pack_byte(np->rpk, cmd->buttons, sizeof(byte));
  }
}

dboolean N_UnpackServerTic(netpeer_t *np, int *tic, objbuf_t *commands) {
  int m_tic = 0;
  int players_this_tic = 0;

  unpack_and_validate("tic", mp_uint);
  m_tic = (int)obj->via.u64;

  unpack_and_validate("player command count", mp_uint);
  players_this_tic = (int)obj->via.u64;

  for (int i = 0; i < players_this_tic; i++) {
    if (commands->objects[i] == NULL)
      commands->objects[i] = malloc(sizeof(ticcmd_t));

    memcpy(commands->objects[i]
  }

  return true;
}

void N_PackQuit(netpeer_t *np) {
  msgpack_pack_unsigned_char(np->rpk, PKT_QUIT);
  msgpack_pack_int(np->rpk, gametic);
  if (server)
    msgpack_pack_short(np->rpk, np->playernum);
  else
    msgpack_pack_short(np->rpk, consoleplayer);
}

dboolean N_UnpackQuit(netpeer_t *np, int *tic, short *playernum) {
  int m_tic = -1;
  int m_playernum = -1;

  unpack_and_validate("tic", mp_uint);
  validate_is_int("tic");
  m_tic = (int)obj->via.u64;

  unpack_and_validate("player", mp_uint);
  validate_is_int("player");
  m_playernum = (int)obj->via.u64;

  *tic = m_tic;
  *playernum = m_playernum;

  return true;
}

void N_PackDown(netpeer_t *np) {
  msgpack_pack_unsigned_char(np->rpk, PKT_DOWN);
}

/*
 * CG: In PKT_WAD messages, the client sends the name of a missing WAD to the
 *     server, and the server sends back a URL where the WAD can be downloaded,
 *     or a zero-length string.
 */

void N_PackWad(netpeer_t *np, rune *wad_name_or_url) {
  size_t length = strlen(wad_name_or_url);

  msgpack_pack_unsigned_char(np->rpk, PKT_WAD);
  msgpack_pack_raw(np->rpk, length);
  msgpack_pack_raw_body(np->rpk, wad_name_or_url, length);
}

dboolean N_UnpackWad(netpeer_t *np, buf_t *wad_name_or_url) {
  unpack_and_validate("wad name", mp_raw);

  M_BufferSetString(buf, (rune *)obj->via.raw.ptr, (size_t)obj->via.raw.size);

  return true;
}

void N_PackBackoff(netpeer_t *np, int tic) {
  msgpack_pack_unsigned_char(np->rpk, PKT_BACKOFF);
  msgpack_pack_int(np->rpk, tic);
}

dboolean N_UnpackBackoff(netpeer_t *np, int *tic) {
  unpack_and_validate("tic", mp_uint);
  validate_is_int("tic");

  *tic = (int)obj->via.u64;

  return true;
}

/* vi: set et ts=2 sw=2: */

