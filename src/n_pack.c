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

#define unpack_and_validate(s, t)                                             \
  if (!msgpack_unpacker_next(&pac, &result)) {                                \
    doom_printf(__func__ ": Error unpacking " #s "\n");                       \
    return false;                                                             \
  }                                                                           \
  else {                                                                      \
    if (result.data.type != MSGPACK_OBJECT_ ##t) {                            \
      doom_printf(                                                            \
        __func__ ": Invalid packet: " #s " is not "                           \
        mp_type_names[MSGPACK_OBJECT_ ##t] "\n"                               \
      );                                                                      \
      return false;                                                           \
    }                                                                         \
  }                                                                           \
  obj = &result.data

#define unpack_and_validate_array(as, s, mt, t, ob)                           \
  unpack_and_validate(as, ARRAY)                                              \
  M_ObjBufferClear((ob));                                                     \
  M_ObjBufferEnsureSize(&(ob), obj->via.array.size);                          \
  for (int i = 0; i < obj->via.array.size; i++) {                             \
    t *buf_entry = NULL;                                                      \
    msgpack_object *array_entry = obj->via.array.ptr + i;                     \
    if (array_entry->type != MSGPACK_OBJECT_ ##mt) {                          \
      doom_printf(                                                            \
        __func__ ": Invalid packet: " #s "is not "                            \
        mp_type_names[MSGPACK_OBJECT_ ##t] "\n"                               \
      );                                                                      \
      M_ObjBufferFreeEntriesAndClear((ob));                                   \
      return false;                                                           \
    }                                                                         \
    buf_entry = malloc(array_entry->via.raw.size * sizeof(t));                \
    memcpy(buf_entry, array_entry->via.raw.data, array_entry->via.raw.size);  \
    M_ObjBufferAppend((ob), buf_entry);                                       \
  }

#define unpack_and_validate_string_array(as, s, ob)                           \
  unpack_and_validate(as, ARRAY)                                              \
  M_ObjBufferClear((ob));                                                     \
  M_ObjBufferEnsureSize(&(ob), obj->via.array.size);                          \
  for (int i = 0; i < obj->via.array.size; i++) {                             \
    msgpack_object *array_entry = obj->via.array.ptr + i;                     \
    if (array_entry->type != MSGPACK_OBJECT_RAW) {                            \
      doom_printf(                                                            \
        __func__ ": Invalid packet: " #s "is not "                            \
        mp_type_names[MSGPACK_OBJECT_RAW] "\n"                                \
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
    doom_printf(__func__ ": Invalid packet, " #s " size is not %u", sz);      \
    return false;                                                             \
  }

#define validate_is_byte(s)                                                   \
  if (obj->via.u64 > 0xFF) {                                                  \
    doom_printf(__func__ ": " #s " out of range (> 0xFF)\n");                 \
    return false;                                                             \
  }

#define validate_is_short(s)                                                  \
  if (obj->via.u64 > 0xFFFF) {                                                \
    doom_printf(__func__ ": " #s " out of range (> 0xFFFF)\n");               \
    return false;                                                             \
  }

#define validate_is_int(s)                                                    \
  if (obj->via.u64 > 0xFFFFFFFF) {                                            \
    doom_printf(__func__ ": " #s " out of range (> 0xFFFFFFFF)\n");           \
    return false;                                                             \
  }

#define validate_range_unsigned(min, max, s)                                  \
  if (obj->via.u64 < min || obj->via.u64 > max) {                               \
    doom_printf(                                                              \
      __func__ ": " #s " out of range (" #min ", " #max ")\n"                 \
    );                                                                        \
    return false;                                                             \
  }

#define validate_range_signed(min, max, s)                                    \
  if (obj->via.i64 < min || obj->via.i64 > max) {                               \
    doom_printf(                                                              \
      __func__ ": " #s " out of range (" #min ", " #max ")\n"                 \
    );                                                                        \
    return false;                                                             \
  }

#define validate_range_double(min, max, s)                                    \
  if (obj->via.dec < min || obj->via.dec > max) {                               \
    doom_printf(                                                              \
      __func__ ": " #s " out of range (" #min ", " #max ")\n"                 \
    );                                                                        \
    return false;                                                             \
  }

#define SERVER_ONLY(s)                                                        \
  if (!server) {                                                              \
    doom_printf(                                                              \
      __func__ ": Erroneously received packet [" #s "] from the server\n"     \
    );                                                                        \
    return;                                                                   \
  }

#define CLIENT_ONLY(s)                                                        \
  if (server) {                                                               \
    doom_printf(                                                              \
      __func__ ": Erroneously received packet [" #s "] from a client\n"       \
    );                                                                        \
    return;                                                                   \
  }

static const char **mp_type_names = {
  "a NULL",              /* MSGPACK_OBJECT_NIL */
  "a boolean",           /* MSGPACK_OBJECT_BOOLEAN */
  "an unsigned integer", /* MSGPACK_OBJECT_POSITIVE_INTEGER */
  "a signed integer",    /* MSGPACK_OBJECT_NEGATIVE_INTEGER */
  "a double",            /* MSGPACK_OBJECT_DOUBLE */
  "raw bytes",           /* MSGPACK_OBJECT_RAW */
  "an array",            /* MSGPACK_OBJECT_ARRAY */
  "a map"                /* MSGPACK_OBJECT_MAP */
};

static msgpack_unpacker    pac;
static msgpack_unpacked    result;
static msgpack_object     *obj;
static msgpack_object_map *map = NULL;

void N_InitPacker(void) {
  msgpack_unpacker_init(&pac, MSGPACK_UNPACKER_INIT_BUFFER_SIZE);
  msgpack_unpacked_init(&result);
}

dboolean N_LoadNewMessage(netpeer_t *np, byte *message_type) {
  unpack_and_validate("message type", POSITIVE_INTEGER);
  validate_is_byte("message type");

  *message_type = (byte)obj->via.u64;

  return true;
}

/* CG: C/S message packing/unpacking here */

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

  unpack_and_validate("state delta start tic", POSITIVE_INTEGER);
  validate_is_int("state delta start tic");
  m_from_tic = (int)obj->via.u64;

  unpack_and_validate("state delta end tic", POSITIVE_INTEGER);
  validate_is_int("state delta end tic");
  m_to_tic = (int)obj->via.u64;

  unpack_and_validate("state delta data", RAW);

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

  unpack_and_validate("game state tic", POSITIVE_INTEGER);
  m_tic = (int)obj->via.u64;

  unpack_and_validate("game state data", RAW);

  *tic = m_tic;
  M_BufferSetData(buf, (byte *)obj->via.raw.ptr, (size_t)obj->via.raw.size);

  return true;
}

void N_PackPlayerCommandReceived(netpeer_t *np, int tic) {
  msgpack_pack_unsigned_char(np->rpk, nm_playercommandreceived);
  msgpack_pack_int(tic);
}

void N_UnpackPlayerCommandReceived(netpeer_t *np, int *tic) {
  unpack_and_validate("last player command tic received", POSITIVE_INTEGER);
  validate_is_int("last player command tic received");

  *tic = (int)obj.via.u64;
}

void N_PackServerMessage(netpeer_t *np, rune *message) {
  size_t length = strlen(message) * sizeof(rune);

  msgpack_pack_unsigned_char(np->rpk, nm_servermessage);
  msgpack_pack_raw(np->rpk, length);
  msgpack_pack_raw_body(np->rpk, message, length);
}

dboolean N_UnpackServerMessage(netpeer_t *np, buf_t *buf) {
  unpack_and_validate("server message content", RAW);

  M_BufferSetString(buf, (rune *)obj->via.raw.ptr, (size_t)obj->via.raw.size);

  return true;
}

void N_PackAuthResponse(netpeer_t *np, auth_level_e auth_level) {
  msgpack_pack_unsigned_char(np->rpk, nm_authresponse);
  msgpack_pack_unsigned_char(np->rpk, auth_level);
}

dboolean N_UnpackAuthResponse(netpeer_t *np, auth_level_e *auth_level) {
  unpack_and_validate("authorization level", POSITIVE_INTEGER);
  validate_range_signed(0, AUTH_LEVEL_MAX - 1); /* CG: TODO: auth levels */

  *auth_level = (auth_level_e)obj->via.u64;

  return true;
}

void N_PackPlayerCommandReceived(netpeer_t *np, int tic) {
  msgpack_pack_unsigned_char(np->rpk, nm_playercommandreceived);
  msgpack_pack_int(tic);
}

void N_UnpackPlayerCommandReceived(netpeer_t *np, int *tic) {
  unpack_and_validate("last command tic received", POSITIVE_INTEGER);
  validate_is_int("last command tic received");

  *tic = (int)obj->via.u64;
}

void N_PackPlayerMessage(netpeer_t *np, short recipient, rune *message) {
  size_t length = strlen(message) * sizeof(rune);

  msgpack_pack_unsigned_char(np->rpk, nm_playermessage);
  msgpack_pack_short(np->rpk, recipient);
  msgpack_pack_raw(np->rpk, length);
  msgpack_pack_raw_body(np->rpk, message, length);
}

dboolean N_UnpackPlayerMessage(netpeer_t *np, short *recipient, buf_t *buf) {
  short m_recipient = 0;

  unpack_and_validate("player message recipient");
  validate_is_short("player message recipient");
  /* CG: TODO: Player count */
  validate_range_signed("player message recipient", player_count);
  m_recipient = (short)obj->via.i64;

  unpack_and_validate("player message content", RAW);

  *recipient = m_recipient;
  M_BufferSetString(buf, (rune *)obj->via.raw.ptr, (size_t)obj->via.raw.size);

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

  unpack_and_validate("command count", POSITIVE_INTEGER);
  validate_is_byte("command_count");
  M_ObjBufferEnsureSize(np->commands, command_count);

  for (byte i = 0; i < command_count; i++) {
    netticmd_t *cmd = np->commands->objects[i];

    if (cmd == NULL)
      cmd = calloc(1, sizeof(netticcmd_t));
    else
      memset(cmd, 0, sizeof(netticcmd_t));

    unpack_and_validate("command tic", POSITIVE_INTEGER);
    validate_is_int("command tic");
    cmd->tic = (int)obj->via.u64;

    unpack_and_validate("command forward value", NEGATIVE_INTEGER);
    validate_is_byte("command forward value");
    cmd->forwardmove = (signed char)obj->via.i64;

    unpack_and_validate("command side value", NEGATIVE_INTEGER);
    validate_is_byte("command side value");
    cmd->sidemove = (signed char)obj->via.i64;

    unpack_and_validate("command angle value", NEGATIVE_INTEGER);
    validate_is_short("command angle value");
    cmd->angle = (signed short)obj->via.i64;

    unpack_and_validate("command consistancy value", NEGATIVE_INTEGER);
    validate_is_short("command consistancy value");
    cmd->consistancy = (signed short)obj->via.i64;

    unpack_and_validate("command chatchar value", POSITIVE_INTEGER);
    validate_is_byte("command chatchar value");
    cmd->chatchar = (byte)obj->via.u64;

    unpack_and_validate("command buttons value", POSITIVE_INTEGER);
    validate_is_byte("command buttons value");
    cmd->buttons = (byte)obj->via.u64;
  }

  return true;
}


void N_PackAuthRequest(netpeer_t *np, rune *password) {
  size_t length = strlen(password) * sizeof(rune);

  msgpack_pack_unsigned_char(np->rpk, nm_authrequest);
  msgpack_pack_raw(np->rpk, length);
  msgpack_pack_raw_body(np->rpk, password, length);
}

dboolean N_UnpackAuthRequest(netpeer_t *np, buf_t *buf) {
  unpack_and_validate("authorization request password", RAW);

  M_BufferSetString(buf, (rune *)obj->via.raw.ptr, (size_t)obj->via.raw.size);

  return true;
}

dboolean N_UnpackClientPreferenceChange(netpeer_t *np, size_t *count) {
  unpack_and_validate("client preference change", MAP);

  map = &obj->via.map;
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

  if (pair->key.type != MSGPACK_OBJECT_RAW) {
    doom_printf(
      "N_UnpackClientPreferenceName: Invalid packet: client preference name "
      "is not raw bytes\n"
    );
    return false;
  }

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
  unpack_and_validate("new name", RAW);

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
  unpack_and_validate("team index", POSITIVE_INTEGER);
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
  unpack_and_validate("new WSOP value", POSITIVE_INTEGER);
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
  unpack_and_validate("new Autoaim value", BOOLEAN);

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

  unpack_and_validate("new weapon speed value", POSITIVE_INTEGER);
  validate_is_byte("new weapon speed value");

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
                                     (new_blue  << 8));
}

dboolean N_UnpackColorChange(netpeer_t *np, byte *new_red, byte *new_green,
                                            byte *new_blue) {
  unpack_and_validate("new color", POSITIVE_INTEGER);
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

void N_PackRCONCommand(netpeer_t *np, rune *command) {
  size_t length = strlen(command) * sizeof(rune);

  msgpack_pack_unsigned_char(np->rpk, nm_rconcommand);
  msgpack_pack_raw(np->rpk, length);
  msgpack_pack_raw_body(np->rpk, command, length);
}

dboolean N_UnpackRCONCommand(netpeer_t *np, buf_t *buf) {
  unpack_and_validate("RCON command", RAW);

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
  unpack_and_validate("vote request command", RAW);

  M_BufferSetString(buf, (rune *)obj->via.raw.ptr, (size_t)obj->via.raw.size);

  return true;
}

/* CG: P2P message packing/unpacking here */

void N_PackInit(netpeer_t *np, short wanted_player_number) {
  msgpack_pack_unsigned_char(np->rpk, PKT_INIT);
  msgpack_pack_short(np->rpk, wanted_player_number);
}

dboolean N_UnpackInit(netpeer_t *np, short *wanted_player_number) {
  unpack_and_validate("wanted player number", POSITIVE_INTEGER);
  validate_is_short("wanted player number");

  *wanted_player_number = (short)obj->via.i64;

  return true;
}

void N_PackSetup(netpeer_t *np, setup_packet_t *sinfo, buf_t *wad_names) {
  int offset = 0;

  msgpack_pack_unsigned_char(np->rpk, PKT_SETUP);
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

  unpack_and_validate("players", POSITIVE_INTEGER);
  validate_is_short("players");
  players = (short)obj->via.i64;

  unpack_and_validate("yourplayer", POSITIVE_INTEGER);
  validate_is_short("yourplayer");
  yourplayer = (short)obj->via.i64;

  unpack_and_validate("skill", POSITIVE_INTEGER);
  validate_is_byte("skill");
  skill = (byte)obj->via.u64;

  unpack_and_validate("episode", POSITIVE_INTEGER);
  validate_is_byte("episode");
  episode = (byte)obj->via.u64;

  unpack_and_validate("level", POSITIVE_INTEGER);
  validate_is_byte("level");
  level = (byte)obj->via.u64;

  unpack_and_validate("deathmatch", POSITIVE_INTEGER);
  validate_is_byte("deathmatch");
  deathmatch = (byte)obj->via.u64;

  unpack_and_validate("complevel", POSITIVE_INTEGER);
  validate_is_byte("complevel");
  complevel = (byte)obj->via.u64;

  unpack_and_validate("ticdup", POSITIVE_INTEGER);
  validate_is_byte("ticdup");
  ticdup = (byte)obj->via.u64;

  unpack_and_validate("extratic", POSITIVE_INTEGER);
  validate_is_byte("extratic");
  extratic = (byte)obj->via.u64;

  unpack_and_validate("game options", RAW);
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

void N_PackGo(netpeer_t *np) {
  msgpack_pack_unsigned_char(np->rpk, PKT_GO);
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

  unpack_and_validate("tic", POSITIVE_INTEGER);
  m_tic = (int)obj->via.u64;

  unpack_and_validate("command count", POSITIVE_INTEGER);
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

    unpack_and_validate("player", POSITIVE_INTEGER);
    validate_is_short("player");
    playernum = (short)obj->via.u64;

    M_ObjBufferEnsureSize(playernum + 1);

    unpack_and_validate("forwardmove", NEGATIVE_INTEGER);
    validate_is_byte("forwardmove");
    cmd.forwardmove = (signed char)obj->via.i64;

    unpack_and_validate("sidemove", NEGATIVE_INTEGER);
    validate_is_byte("sidemove");
    cmd.sidemove = (signed char)obj->via.i64;

    unpack_and_validate("angleturn", NEGATIVE_INTEGER);
    validate_is_byte("angleturn");
    cmd.angleturn = (short)obj->via.i64;

    unpack_and_validate("consistancy", NEGATIVE_INTEGER);
    validate_is_byte("consistancy");
    cmd.consistancy = (short)obj->via.i64;

    unpack_and_validate("chatchar", POSITIVE_INTEGER);
    validate_is_byte("chatchar");
    cmd.chatchar = (byte)obj->via.u64;

    unpack_and_validate("buttons", POSITIVE_INTEGER);
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

  unpack_and_validate("tic", POSITIVE_INTEGER);
  m_tic = (int)obj->via.u64;

  unpack_and_validate("player command count", POSITIVE_INTEGER);
  players_this_tic = (int)obj->via.u64;

  for (int i = 0; i < players_this_tic; i++) {
    if (commands->objects[i] == NULL)
      commands->objects[i] = malloc(sizeof(ticcmd_t));

    memcpy(commands->objects[i]
  }

  return true;
}

void N_PackRetransmissionRequest(netpeer_t *np, int tic) {
  msgpack_pack_unsigned_char(np->rpk, PKT_RETRANS);
  msgpack_pack_int(np->rpk, tic);
}

dboolean N_UnpackRetransmissionRequest(netpeer_t *np, int *tic) {
  unpack_and_validate("tic", POSITIVE_INTEGER);
  validate_is_int("tic");

  *tic = (int)obj->via.u64;

  return true;
}

void N_PackColor(netpeer_t *np, mapcolor_me new_color) {
  msgpack_pack_unsigned_char(np->rpk, PKT_COLOR);
  msgpack_pack_int(np->rpk, gametic);
  if (server)
    msgpack_pack_short(np->rpk, np->playernum);
  else
    msgpack_pack_short(np->rpk, consoleplayer);
  msgpack_pack_int(np->rpk, new_color);
}

dboolean N_UnpackColor(netpeer_t *np, int *tic, short *playernum,
                                      mapcolor_me *new_color) {
  int m_tic = -1;
  short m_playernum = -1;
  mapcolor_me m_new_color;

  unpack_and_validate("tic", POSITIVE_INTEGER);
  validate_is_int("tic");
  m_tic = (int)obj->via.u64;

  unpack_and_validate("player", POSITIVE_INTEGER);
  validate_is_short("player");
  m_playernum = (short)obj->via.i64;

  unpack_and_validate("new color", POSITIVE_INTEGER);
  validate_is_int("new color");

  /* CG: TODO: Validate color value is within the mapcolor_me enum */
  m_new_color = (mapcolor_me)obj->via.i64;

  *tic = m_tic;
  *playernum = m_playernum;
  *new_color = m_new_color;

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

  unpack_and_validate("tic", POSITIVE_INTEGER);
  validate_is_int("tic");
  m_tic = (int)obj->via.i64;

  unpack_and_validate("save game name", RAW);

  *tic = m_tic;
  M_BufferSetString(buf, (rune *)obj->via.raw.ptr, (size_t)obj->via.raw.size);

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

  unpack_and_validate("tic", POSITIVE_INTEGER);
  validate_is_int("tic");
  m_tic = (int)obj->via.u64;

  unpack_and_validate("player", POSITIVE_INTEGER);
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
  unpack_and_validate("wad name", RAW);

  M_BufferSetString(buf, (rune *)obj->via.raw.ptr, (size_t)obj->via.raw.size);

  return true;
}

void N_PackBackoff(netpeer_t *np, int tic) {
  msgpack_pack_unsigned_char(np->rpk, PKT_BACKOFF);
  msgpack_pack_int(np->rpk, tic);
}

dboolean N_UnpackBackoff(netpeer_t *np, int *tic) {
  unpack_and_validate("tic", POSITIVE_INTEGER);
  validate_is_int("tic");

  *tic = (int)obj->via.u64;

  return true;
}

/* vi: set et ts=2 sw=2: */

