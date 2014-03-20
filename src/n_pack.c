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
#include "m_obuf.h"

#define unpack_and_validate(s, t) \
  if (!msgpack_unpacker_next(&pac, &result)) { \
    doom_printf("N_HandlePacket: Error unpacking " #s "\n"); \
    return false; \
  } \
  else { \
    if (result.data.type != MSGPACK_OBJECT_ ## t) { \
      doom_printf(\
        "N_HandlePacket: Invalid packet: " #s "is not " \
        mp_type_names[MSGPACK_OBJECT_ ## t] "\n" \
      ); \
      return false; \
    } \
  } \
  obj = result.data

#define validate_is_byte(s) \
  if (obj.via.u64 > 0xFF) {\
    doom_printf("N_HandlePacket: " #s " out of range (> 0xFF\n"); \
    return false; \
  }

#define validate_is_short(s) \
  if (obj.via.u64 > 0xFFFF) {\
    doom_printf("N_HandlePacket: " #s " out of range (> 0xFFFF\n"); \
    return false; \
  }

#define validate_is_int(s) \
  if (obj.via.u64 > 0xFFFFFFFF) {\
    doom_printf("N_HandlePacket: " #s " out of range (> 0xFFFFFFFF\n"); \
    return false; \
  }

#define validate_range_unsigned(min, max, s) \
  if (obj.via.u64 < min || obj.via.u64 > max) { \
    doom_printf(\
      "N_HandlePacket: " #s " out of range (" #min ", " #max ")\n" \
    ); \
    return false; \
  }

#define validate_range_signed(min, max, s) \
  if (obj.via.i64 < min || obj.via.i64 > max) { \
    doom_printf(\
      "N_HandlePacket: " #s " out of range (" #min ", " #max ")\n" \
    ); \
    return false; \
  }

#define validate_range_double(min, max, s) \
  if (obj.via.dec < min || obj.via.dec > max) { \
    doom_printf(\
      "N_HandlePacket: " #s " out of range (" #min ", " #max ")\n" \
    ); \
    return false; \
  }

#define SERVER_ONLY(s) \
  if (!server) { \
    doom_printf( \
      "N_HandlePacket: Erroneously received packet [" #s "] from the server\n" \
    ); \
    return; \
  }

#define CLIENT_ONLY(s) \
  if (server) { \
    doom_printf( \
      "N_HandlePacket: Erroneously received packet [" #s "] from a client\n" \
    ); \
    return; \
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

static msgpack_object   obj;
static msgpack_unpacker pac;
static msgpack_unpacked result;

void N_InitPacker(void) {
  msgpack_unpacker_init(&pac, MSGPACK_UNPACKER_INIT_BUFFER_SIZE);
  msgpack_unpacked_init(&result);
}

dboolean N_UnpackMessageType(netpeer_t *np, byte *message_type) {
  unpack_and_validate("message type", POSITIVE_INTEGER);
  validate_is_byte("message type");

  *message_type = (byte)obj.via.u64;

  return true;
}

void N_PackGameState(netpeer_t *np, void *state_data, size_t state_size) {
  msgpack_pack_unsigned_char(np->pk, nm_gamestate);
  msgpack_pack_raw(np->pk, state_size);
  msgpack_pack_raw_body(np->pk, state_data);
}

dboolean N_UnpackGameState(netpeer_t *np, buf_t *buf) {
  byte *state_data = NULL;
  size_t state_size = 0;

  unpack_and_validate("game state data", RAW);
  state_data = (void *)obj.via.raw.ptr;

  unpack_and_validate("game state size", POSITIVE_INTEGER);
  state_size = (size_t)obj.via.u64;

  M_BufferSetData(buf, state_data, state_size);

  return true;
}

void N_PackServerMessage(netpeer_t *np, rune *message) {
  msgpack_pack_unsigned_char(np->pk, nm_servermessage);
  msgpack_pack_raw(np->pk, strlen(message) * sizeof(rune));
  msgpack_pack_raw_body(np->pk, message);
}

dboolean N_UnpackServerMessage(netpeer_t *np, buf_t *buf) {
  unpack_and_validate("server message", RAW);

  M_BufferSetString(buf, (rune *)obj.via.raw.ptr, (size_t)obj.via.raw.size);

  return true;
}

void N_PackAuthResponse(netpeer_t *np, auth_level_e auth_level) {
  msgpack_pack_unsigned_char(np->pk, nm_authresponse);
  msgpack_pack_unsigned_char(np->pk, auth_level);
}

dboolean N_UnpackAuthResponse(netpeer_t *np, auth_level_e *auth_level) {
  unpack_and_validate("authorization response", POSITIVE_INTEGER);
  validate_range_signed(0, AUTH_LEVEL_MAX - 1); /* CG: TODO: auth levels */

  *auth_level = (auth_level_e)obj.via.u64;

  return true;
}

void N_PackPlayerCommand(netpeer_t *np, netticcmd_t *cmd) {
  msgpack_pack_unsigned_char(np->pk, nm_playercommand);
  msgpack_pack_unsigned_int(np->pk, cmd->index);
  msgpack_pack_unsigned_int(np->pk, cmd->world_index);
  msgpack_pack_signed_char(np->pk, cmd->forward);
  msgpack_pack_signed_char(np->pk, cmd->side);
  msgpack_pack_short(np->pk, cmd->angle);
  msgpack_pack_unsigned_char(np->pk, cmd->buttons);
}

dboolean N_UnpackPlayerCommand(netpeer_t *np, netticcmd_t *cmd) {
  unsigned int index = 0;
  unsigned int world_index = 0;
  signed char  forward = 0;
  signed char  side = 0;
  short        angle = 0;
  byte         buttons = 0;

  unpack_and_validate("command index", POSITIVE_INTEGER);
  validate_is_int("command index");
  index = (unsigned int)obj.via.u64;

  unpack_and_validate("command world index", POSITIVE_INTEGER);
  validate_is_int("command world index");
  world_index = (unsigned int)obj.via.u64;

  unpack_and_validate("command forward value", NEGATIVE_INTEGER);
  validate_is_byte("command forward value");
  forward = (signed char)obj.via.i64;

  unpack_and_validate("command side value", NEGATIVE_INTEGER);
  validate_is_byte("command side value");
  side = (signed char)obj.via.i64;

  unpack_and_validate("command angle value", NEGATIVE_INTEGER);
  validate_is_short("command angle value");
  angle = (short)obj.via.i64;

  unpack_and_validate("command buttons", POSITIVE_INTEGER);
  validate_is_byte("command buttons");
  buttons = (byte)obj.via.u64;

  cmd->index = index;
  cmd->world_index = world_index;
  cmd->forward = forward;
  cmd->side = side;
  cmd->angle = angle;
  cmd->buttons = buttons;

  return true;
}


void N_PackPlayerMessage(netpeer_t *np, short recipient, rune *message) {
  msgpack_pack_unsigned_char(np->pk, nm_playermessage);
  msgpack_pack_short(np->pk, recipient);
  msgpack_pack_raw(np->pk, strlen(message));
  msgpack_pack_raw_body(np->pk, message);
}

dboolean N_UnpackPlayerMessage(netpeer_t *np, short *recipient, buf_t *buf) {
  short m_recipient = 0;

  unpack_and_validate("player message recipient");
  validate_is_short("player message recipient");
  /* CG: TODO: Player count */
  validate_range_signed("player message recipient", player_count);
  m_recipient = (short)obj.via.i64;

  unpack_and_validate("player message content", RAW);

  *recipient = m_recipient;
  M_BufferSetString(buf, (rune *)obj.via.raw.ptr, (size_t)obj.via.raw.size);

  return true;
}

void N_PackAuthRequest(netpeer_t *np, rune *password) {
  msgpack_pack_unsigned_char(np->pk, nm_authrequest);
  msgpack_pack_raw(np->pk, strlen(password) * sizeof(rune));
  msgpack_pack_raw_body(np->pk, password);
}

dboolean N_UnpackAuthRequest(netpeer_t *np, buf_t *buf) {
  unpack_and_validate("authorization request password", RAW);

  M_BufferSetString(buf, (rune *)obj.via.raw.ptr, (size_t)obj.via.raw.size);

  return true;
}

void N_PackNameChanged(netpeer_t *np, rune *new_name) {
  msgpack_pack_unsigned_char(np->pk, nm_namechange);
  msgpack_pack_raw(np->pk, strlen(new_name) * sizeof(rune));
  msgpack_pack_raw_body(np->pk, new_name);
}

dboolean N_UnpackNameChanged(netpeer_t *np, buf_t *buf) {
  unpack_and_validate("new name", RAW);

  M_BufferSetString(buf, (rune *)obj.via.raw.ptr, (size_t)obj.via.raw.size);

  return true;
}

void N_PackTeamChanged(netpeer_t *np, byte new_team) {
  msgpack_pack_unsigned_char(np->pk, nm_teamchange);
  msgpack_pack_unsigned_char(np->pk, new_team);
}

dboolean N_UnpackTeamChanged(netpeer_t *np, byte *new_team) {
  unpack_and_validate("team index", POSITIVE_INTEGER);
  /* CG: TODO: teams */
  if (team_count > 0)
    validate_range_unsigned(0, team_count - 1);
  else
    validate_range_unsigned(0, 0);

  *new_team = (byte)obj.via.u64;

  return true;
}

void N_PackPWOChanged(netpeer_t *np) {
  msgpack_pack_unsigned_char(np->pk, nm_pwochange);
  /* CG: TODO */
}

dboolean N_UnpackPWOChanged(netpeer_t *np) {
  /* CG: TODO */
  return false;
}

void N_PackWSOPChanged(netpeer_t *np, byte new_wsop_flags) {
  msgpack_pack_unsigned_char(np->pk, nm_wsopchange);
  msgpack_pack_unsigned_char(np->pk, new_wsop_flags);
}

dboolean N_UnpackWSOPChanged(netpeer_t *np, byte *new_wsop_flags) {
  unpack_and_validate("new WSOP value", POSITIVE_INTEGER);
  validate_range_unsigned(0, 2 << (WSOP_MAX - 2)); /* CG: TODO: WSOP */

  *new_wsop_flags = (byte)obj.via.u64;

  return true;
}

void N_PackBobbingChanged(netpeer_t *np, double new_bobbing_amount) {
  msgpack_pack_unsigned_char(np->pk, nm_bobbingchange);
  msgpack_pack_double(np->pk, new_bobbing_amount);
}

dboolean N_UnpackBobbingchanged(netpeer_t *np, double *new_bobbing_amount) {
  unpack_and_validate("new bobbing amount", DOUBLE);
  validate_range_double(0.0, 1.0);

  *new_bobbing_amount = obj.via.double;

  return true;
}

void N_PackAutoaimChanged(netpeer_t *np, dboolean new_autoaim_enabled) {
  msgpack_pack_unsigned_char(np->pk, nm_autoaimchange);
  if (new_autoaim_enabled)
    msgpack_pack_true(np->pk);
  else
    msgpack_pack_false(np->pk);
}

dboolean N_UnpackAutoaimChanged(netpeer_t *np, dboolean *new_autoaim_enabled) {
  unpack_and_validate("new Autoaim value", BOOLEAN);

  *new_autoaim_enabled = obj.via.bool;

  return true;
}

void N_PackWeaponSpeedChanged(netpeer_t *np, byte new_weapon_speed) {
  msgpack_pack_unsigned_char(np->pk, nm_weaponspeedchange);
  msgpack_pack_unsigned_char(np->pk, new_autoaim_enabled);
}

dboolean N_UnpackWeaponSpeedChanged(netpeer_t *np, byte *new_weapon_speed) {

  unpack_and_validate("new weapon speed value", POSITIVE_INTEGER);
  validate_is_byte("new weapon speed value");

  *new_weapon_speed = (byte)obj.via.u64;

  return true;
}

void N_PackColorChanged(netpeer_t *np, byte new_red, byte new_green,
                                       byte new_blue) {
  msgpack_pack_unsigned_char(np->pk, nm_colorchange);
  msgpack_pack_unsigned_char(np->pk, new_red);
  msgpack_pack_unsigned_char(np->pk, new_green);
  msgpack_pack_unsigned_char(np->pk, new_blue);
}

dboolean N_UnpackColorChanged(netpeer_t *np, byte *new_red,
                                             byte *new_green,
                                             byte *new_blue) {
  byte m_new_red = 0;
  byte m_new_green = 0;
  byte m_new_blue = 0;

  unpack_and_validate("new red value", POSITIVE_INTEGER);
  validate_is_byte("new red value");
  m_new_red = (byte)obj.via.u64;

  unpack_and_validate("new green value", POSITIVE_INTEGER);
  validate_is_byte("new green value");
  m_new_green = (byte)obj.via.u64;

  unpack_and_validate("new blue value", POSITIVE_INTEGER);
  validate_is_byte("new blue value");
  m_new_blue = (byte)obj.via.u64;

  *new_red = m_new_red;
  *new_green = m_new_green;
  *new_blue = m_new_blue;

  return true;
}

void N_PackSkinChanged(netpeer_t *np) {
  msgpack_pack_unsigned_char(np->pk, nm_skinchange);
  /* CG: TODO */
}

dboolean N_UnpackSkinChanged(netpeer_t *np) {

  return false;
}

void N_PackRCONCommand(netpeer_t *np, rune *command) {
  msgpack_pack_unsigned_char(np->pk, nm_rconcommand);
  msgpack_pack_raw(np->pk, strlen(command) * sizeof(rune));
  msgpack_pack_raw_body(np->pk, command);
}

dboolean N_UnpackRCONCommand(netpeer_t *np, buf_t *buf) {
  unpack_and_validate("RCON command", RAW);

  M_BufferSetString(buf, (rune *)obj.via.raw.ptr, (size_t)obj.via.raw.size);

  return true;
}

void N_PackVoteRequest(netpeer_t *np, rune *command) {
  msgpack_pack_unsigned_char(np->pk, nm_voterequest);
  msgpack_pack_raw(np->pk, strlen(command) * sizeof(rune));
  msgpack_pack_raw_body(np->pk, command);
}

dboolean N_UnpackVoteRequest(netpeer_t *np, buf_t *buf) {
  unpack_and_validate("vote request command", RAW);

  M_BufferSetString(buf, (rune *)obj.via.raw.ptr, (size_t)obj.via.raw.size);

  return true;
}

/* vi: set cindent et ts=2 sw=2: */

