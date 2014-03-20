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

#include "n_peer.h"

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

static void handle_game_state_message(netpeer_t *np) {
}

static void handle_server_message_message(netpeer_t *np) {
}

static void handle_authorization_response_message(netpeer_t *np) {
}

static void handle_player_message_message(netpeer_t *np) {
  if (server) {
    /* CG: TODO */
  }
  else {
    /* CG: TODO */
  }
}

static void handle_player_command_message(netpeer_t *np) {
}

static void handle_authorization_request_message(netpeer_t *np) {
}

static void handle_name_change_message(netpeer_t *np) {
}

static void handle_team_change_message(netpeer_t *np) {
}

static void handle_pwo_change_message(netpeer_t *np) {
}

static void handle_wsop_change_message(netpeer_t *np) {
}

static void handle_bobbing_change_message(netpeer_t *np) {
}

static void handle_autoaim_change_message(netpeer_t *np) {
}

static void handle_weapon_speed_change_message(netpeer_t *np) {
}

static void handle_color_change_message(netpeer_t *np) {
}

static void handle_skin_change_message(netpeer_t *np) {
}

static void handle_rcon_message(netpeer_t *np) {
}

static void handle_vote_request_message(netpeer_t *np) {
}

void N_InitProtocol(void) {
  msgpack_unpacker_init(&pac, MSGPACK_UNPACKER_INIT_BUFFER_SIZE);
  msgpack_unpacked_init(&result);
}

void N_HandlePacket(int peernum, void *data, size_t data_size) {
  netpeer_t *np = N_GetPeer(peernum);
  byte message_type = 0;

  msgpack_unpacker_reserve_buffer(pac, data_size);
  memcpy(msgpack_unpacker_buffer(pac), data, data_size);
  msgpack_unpacker_buffer_consumed(pac, buf->size);

  if (!N_UnpackMessageType(np, &message_type))
    return;

  switch (message_type) {
    case nm_gamestate:
      CLIENT_ONLY("game state");
      handle_game_state_message(np);
    break;
    case nm_servermessage:
      CLIENT_ONLY("server message");
      handle_server_message_message(np);
    break;
    case nm_authresponse:
      CLIENT_ONLY("authorization response");
      handle_authorization_response_message(np);
    break;
    case nm_playermessage:
      /* CG: Both servers and clients receive player message messages */
      handle_player_message_message(np);
    break;
    case nm_playercommand:
      SERVER_ONLY("player command");
      handle_player_command_message(np);
    break;
    case nm_authrequest:
      SERVER_ONLY("authorization request");
      handle_authorization_request_message(np);
    break;
    case nm_namechange:
      SERVER_ONLY("name change");
      handle_name_change_message(np);
    break;
    case nm_teamchange:
      SERVER_ONLY("team change");
      handle_team_change_message(np);
    break;
    case nm_pwochange:
      SERVER_ONLY("PWO change");
      handle_pwo_change_message(np);
    break;
    case nm_wsopchange:
      SERVER_ONLY("WSOP change");
      handle_wsop_change_message(np);
    break;
    case nm_bobbingchange:
      SERVER_ONLY("bobbing change");
      handle_bobbing_change_message(np);
    break;
    case nm_autoaimchange:
      SERVER_ONLY("autoaim change");
      handle_autoaim_change_message(np);
    break;
    case nm_weaponspeedchange:
      SERVER_ONLY("weapon speed change");
      handle_weapon_speed_change_message(np);
    break;
    case nm_colorchange:
      SERVER_ONLY("color change");
      handle_color_change_message(np);
    break;
    case nm_skinchange:
      SERVER_ONLY("skin change");
      handle_skin_change_message(np);
    break;
    case nm_rconcommand:
      SERVER_ONLY("RCON command");
      handle_rcon_message(np);
    break;
    case nm_voterequest:
      SERVER_ONLY("vote request");
      handle_vote_request_message(np);
    break;
  }
}

/* vi: set cindent et ts=2 sw=2: */
