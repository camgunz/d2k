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

#include "doomdef.h"
#include "doomstat.h"
#include "d_deh.h"
#include "d_main.h"
#include "d_ticcmd.h"
#include "g_game.h"
#include "i_system.h"
#include "lprintf.h"
#include "m_swap.h"
#include "w_wad.h"

#include "n_net.h"
#include "n_main.h"
#include "n_peer.h"
#include "n_pack.h"
#include "n_proto.h"
#include "n_state.h"

const char *D_dehout(void); /* CG: from d_main.c */

#define MAX_PREF_NAME_SIZE 20

#define COMMAND_SYNC_ONLY(name)                                               \
  if (!CMDSYNC) {                                                             \
    doom_printf(                                                              \
      __func__ ": Erroneously received command-sync packet [" name "] in "    \
      "delta-sync mode\n"                                                     \
    );                                                                        \
    return;                                                                   \
  }

#define DELTA_SYNC_ONLY(name)                                                 \
  if (!DELTASYNC) {                                                           \
    doom_printf(                                                              \
      __func__ ": Erroneously received delta-sync packet [" name "] in "      \
      "command-sync mode\n"                                                   \
    );                                                                        \
    return;                                                                   \
  }

#define SERVER_ONLY(name)                                                     \
  if (!SERVER) {                                                              \
    doom_printf(                                                              \
      __func__ ": Erroneously received packet [" name "] from the server\n"   \
    );                                                                        \
    return;                                                                   \
  }

#define CLIENT_ONLY(name)                                                     \
  if (SERVER) {                                                               \
    doom_printf(                                                              \
      __func__ ": Erroneously received packet [" name "] from a client\n"     \
    );                                                                        \
    return;                                                                   \
  }

#define NOT_DELTA_SERVER(name)                                                \
  if (DELTASERVER) {                                                          \
    doom_printf(                                                              \
      __func__ ": Erroneously received packet [" name "] as a server in "     \
      "delta-sync mode\n"                                                     \
    );                                                                        \
    return;                                                                   \
  }

#define NOT_DELTA_CLIENT(name)                                                \
  if (DELTACLIENT) {                                                          \
    doom_printf(                                                              \
      __func__ ": Erroneously received packet [" name "] as a client in "     \
      "delta-sync mode\n"                                                     \
    );                                                                        \
    return;                                                                   \
  }

#define NOT_COMMAND_SERVER(name)                                              \
  if (COMMANDSERVER) {                                                        \
    doom_printf(                                                              \
      __func__ ": Erroneously received packet [" name "] as a server in "     \
      "command-sync mode\n"                                                   \
    );                                                                        \
    return;                                                                   \
  }

#define NOT_COMMAND_CLIENT(name)                                              \
  if (COMMANDCLIENT) {                                                        \
    doom_printf(                                                              \
      __func__ ": Erroneously received packet [" name "] as a client in "     \
      "command-sync mode\n"                                                   \
    );                                                                        \
    return;                                                                   \
  }

#define DELTA_SERVER_ONLY(name)                                               \
  DELTA_SYNC_ONLY(name);                                                      \
  SERVER_ONLY(name);

#define DELTA_CLIENT_ONLY(name)                                               \
  DELTA_SYNC_ONLY(name);                                                      \
  CLIENT_ONLY(name);

#define COMMAND_SERVER_ONLY(name)                                             \
  COMMAND_SYNC_ONLY(name);                                                    \
  SERVER_ONLY(name);

#define COMMAND_CLIENT_ONLY(name)                                             \
  COMMAND_SYNC_ONLY(name);                                                    \
  CLIENT_ONLY(name);

#define CHECK_VALID_PLAYER(np, playernum)                                     \
  if (playernum < 0 || playernum >= MAXPLAYERS)                               \
    I_Error(__func__ ": Invalid player %d.\n", playernum);                    \
  if (!playeringame[playernum])                                               \
    I_Error(__func__ ": Invalid player %d.\n", playernum);                    \
  if (((np) = N_GetPeerForPlayer(playernum)) == NULL)                         \
    I_Error(__func__ ": Invalid player %d.\n", playernum)

#define CHECK_CONNECTION(np)                                                  \
  if (((np) = N_GetPeer(0)) == NULL) {                                        \
    doom_printf(__func__ ": Not connected\n");                                \
    return;                                                                   \
  }

const byte nm_setup                  = 1;  /* S => C | BOTH    |   reliable */
const byte nm_fullstate              = 2;  /* S => C | BOTH    |   reliable */
const byte nm_statedelta             = 3;  /* S => C | DELTA   | unreliable */
const byte nm_authresponse           = 4;  /* S => C | BOTH    |   reliable */
const byte nm_servermessage          = 5;  /* S => C | BOTH    |   reliable */
const byte nm_playermessage          = 6;  /* BOTH   | BOTH    |   reliable */
const byte nm_playercommandreceived  = 7;  /* BOTH   | BOTH    |   reliable */
const byte nm_playercommands         = 8;  /* NOT DELTA CLIENT | unreliable */
const byte nm_savegamenamechange     = 9;  /* NOT DELTA CLIENT |   reliable */
const byte nm_playerpreferencechange = 10; /* NOT DELTA CLIENT |   reliable */
const byte nm_statereceived          = 11; /* C => S | DELTA   |   reliable */
const byte nm_authrequest            = 12; /* C => S | BOTH    |   reliable */
const byte nm_rconcommand            = 13; /* C => S | BOTH    |   reliable */
const byte nm_voterequest            = 14; /* C => S | BOTH    |   reliable */

static buf_t message_recipients;

static void handle_setup(netpeer_t *np) {
  net_sync_type_e net_sync = NET_SYNC_TYPE_NONE;
  unsigned short player_count = 0;
  unsigned short playernum = 0;
  int i;

  if (!N_UnpackSetup(np, &net_sync, &player_count, &playernum)) {
    M_OBufFreeEntriesAndClear(&resource_files_buf);
    M_OBufFreeEntriesAndClear(&deh_files_buf);
    return;
  }

  netsync = net_sync;

  for (i = 0; i < player_count; i++) {
    playeringame[i] = true;
    M_CBufInitWithCapacity(
      &players[i].commands, sizeof(netticcmd_t), BACKUPTICS
    );
  }

  for (; i < MAXPLAYERS; i++)
    playeringame[i] = false;

  consoleplayer = playernum;

  if (!playeringame[consoleplayer])
    I_Error("consoleplayer not in game");

  OBUF_FOR_EACH(&resource_files_buf, entry) {
    char *name = entry.obj;
    char *fpath = I_FindFile(name, NULL);

    if (fpath == NULL) {
      doom_printf("Unable to find resource \"%s\", disconnecting.\n", name);
      N_Disconnect();
      return;
    }
    else {
      D_AddFile(fpath, source_net);
    }
  }

  OBUF_FOR_EACH(&deh_files_buf, entry) {
    char *name = entry.obj;
    char *fpath = I_FindFile(name, NULL);

    if (fpath == NULL) {
      doom_printf(
        "Unable to find DEH/BEX patch \"%s\", disconnecting.\n", name
      );
      N_Disconnect();
      return;
    }
    else {
      ProcessDehFile(fpath, D_dehout(), 0);
    }
  }
}

static void handle_full_state(netpeer_t *np) {
  static buf_t state_buffer;
  static dboolean initialized_buffer = false;

  int tic;
  dboolean call_init_new = CL_ReceivedSetup();

  if (!initialized_buffer) {
    M_BufferInit(&state_buffer);
    initialized_buffer = true;
  }

  if (N_UnpackFullState(np, &tic, &state_buffer)) {
    N_SaveCurrentState(tic, &state_buffer);
    G_ReadSaveData(N_GetCurrentState(), true, call_init_new);
    CL_SendStateReceived(tic);
    CL_SetReceivedSetup(true);
  }
}

static void handle_state_delta(netpeer_t *np) {
  static buf_t delta_buffer;
  static dboolean initialized_buffer = false;

  int from_tic, to_tic;

  if (!initialized_buffer) {
    M_BufferInit(&delta_buffer);
    initialized_buffer = true;
  }

  if (N_UnpackStateDelta(np, &from_tic, &to_tic, &delta_buffer)) {
    N_ApplyStateDelta(from_tic, to_tic, &delta_buffer);
    G_ReadSaveData(N_GetCurrentState(), true, false);
    CL_SendStateReceived(to_tic);
  }
}

static void handle_auth_response(netpeer_t *np) {
  auth_level_e level;

  if (N_UnpackAuthResponse(np, &level))
    CL_SetAuthorizationLevel(level);
}

static void handle_server_message(netpeer_t *np) {
  static buf_t server_message_buffer;
  static dboolean initialized_buffer = false;

  if (!initialized_buffer) {
    M_BufferInit(&server_message_buffer);
    initialized_buffer = true;
  }

  if (N_UnpackServerMessage(np, &server_message_buffer))
    doom_printf("[SERVER]: %s\n", server_message_buffer.data);
}

static void handle_player_message(netpeer_t *np) {
  static buf_t player_message_buffer;
  static dboolean initialized_buffer = false;

  unsigned short sender = 0;
  size_t recipient_count = 0;
  dboolean unpacked_successfully = false;

  if (!initialized_buffer) {
    M_BufferInit(&player_message_buffer);
    initialized_buffer = true;
  }

  unpacked_successfully = N_UnpackPlayerMessage(
    np, &sender, &recipient_count, &message_recipients, &player_message_buffer
  );

  if (!unpacked_successfully)
    return;

  if (SERVER) {
    for (int i = 0; i < N_GetPeerCount(); i++) {
      netpeer_t *np = N_GetPeer(i);

      if (np != NULL) {
        N_PackPlayerMessage(
          np,
          sender,
          recipient_count,
          &message_recipients,
          player_message_buffer.data
        );
      }
    }
  }
  else {
    if (sender == -1) {
      doom_printf("[SERVER]: %s\n", player_message_buffer.data);
    }
    else {
      doom_printf(
        "%s: %s\n", players[sender].name, player_message_buffer.data
      );
    }
  }
}

static void handle_player_command_received(netpeer_t *np) {
  int tic;

  if (!N_UnpackPlayerCommandReceived(np, &tic))
    return;

  if (CMDSYNC)
    np->last_sync_received_tic = tic;
 
  if (CLIENT)
    CL_RemoveOldCommands(tic);
}

static void handle_player_commands(netpeer_t *np) {
  N_UnpackPlayerCommands(np);
}

static void handle_save_game_name_change(netpeer_t *np) {
  static buf_t save_game_name;
  static dboolean initialized_buffer = false;

  if (!initialized_buffer) {
    M_BufferInit(&save_game_name);
    initialized_buffer = true;
  }

  if (N_UnpackSaveGameNameChange(np, &save_game_name)) {
    if (save_game_name.size < SAVEDESCLEN) {
      memset(savedescription, 0, SAVEDESCLEN);
      memcpy(savedescription, save_game_name.data, save_game_name.size);
    }
  }
}

static void handle_player_preference_change(netpeer_t *np) {
  static buf_t pref_key_name;
  static buf_t pref_key_value;
  static dboolean initialized_buffers = false;

  short playernum = 0;
  int tic = 0;
  size_t pref_count = 0;
  player_t *player = &players[np->playernum];

  if (!initialized_buffers) {
    M_BufferInit(&pref_key_name);
    M_BufferInit(&pref_key_value);
    initialized_buffers = true;
  }

  if (!N_UnpackPlayerPreferenceChange(np, &playernum, &tic, &pref_count))
    return;

  if (SERVER && playernum != np->playernum) {
    doom_printf("Received player preference for player %d from peer %d.\n",
      playernum, np->playernum
    );
    return;
  }

  for (size_t i = 0; i < pref_count; i++) {
    if (!N_UnpackPlayerPreferenceName(np, i, &pref_key_name)) {
      doom_printf(
        "N_HandlePacket: Error unpacking client preference change\n"
      );
      return;
    }

    if (pref_key_name.size > MAX_PREF_NAME_SIZE) {
      doom_printf(
        "N_HandlePacket: Invalid client preference change packet: preference "
        "name exceeds maximum length\n"
      );
      return;
    }

    if (M_BufferEqualsString(&pref_key_name, "name")) {
      N_UnpackNameChange(np, &pref_key_value);

      if (player->name != NULL) {
        doom_printf("%s is now %s.\n", player->name, pref_key_value.data);
        free(player->name);
      }

      player->name = strdup(pref_key_value.data);
    }
    else if (M_BufferEqualsString(&pref_key_name, "team")) {
      byte new_team = 0;

      if (!N_UnpackTeamChange(np, &new_team))
        return;

      player->team = new_team;
    }
    else if (M_BufferEqualsString(&pref_key_name, "pwo")) {
    }
    else if (M_BufferEqualsString(&pref_key_name, "wsop")) {
    }
    else if (M_BufferEqualsString(&pref_key_name, "bobbing")) {
    }
    else if (M_BufferEqualsString(&pref_key_name, "autoaim")) {
    }
    else if (M_BufferEqualsString(&pref_key_name, "weapon_speed")) {
    }
    else if (M_BufferEqualsString(&pref_key_name, "color")) {
    }
    else if (M_BufferEqualsString(&pref_key_name, "colormap")) {
      int new_color;

      if (N_UnpackColormapChange(np, &new_color))
        G_ChangedPlayerColour(np->playernum, new_color);
    }
    else if (M_BufferEqualsString(&pref_key_name, "skin_name")) {
    }
    else {
      doom_printf("Unsupported player preference %s.\n", pref_key_name.data);
    }
  }
}

static void handle_state_received(netpeer_t *np) {
  int tic;

  if (N_UnpackStateReceived(np, &tic))
    np->last_sync_received_tic = tic;
}

static void handle_auth_request(netpeer_t *np) {
  auth_level_e level;

  if (N_UnpackAuthResponse(np, &level))
    CL_SetAuthorizationLevel(level);
}

static void handle_rcon(netpeer_t *np) {
}

static void handle_vote_request(netpeer_t *np) {
}

void N_HandlePacket(int peernum, void *data, size_t data_size) {
  netpeer_t *np = N_GetPeer(peernum);
  byte message_type = 0;

  N_LoadPacketData(data, data_size);

  while (N_LoadNewMessage(np, &message_type)) {
    printf("Handling message %u.\n", message_type);
    switch (message_type) {
      case nm_setup:
        CLIENT_ONLY("setup");
        handle_setup(np);
      break;
      case nm_fullstate:
        DELTA_CLIENT_ONLY("full state");
        handle_full_state(np);
      break;
      case nm_statedelta:
        DELTA_CLIENT_ONLY("state delta");
        handle_state_delta(np);
      break;
      case nm_authresponse:
        CLIENT_ONLY("authorization response");
        handle_auth_response(np);
      break;
      case nm_playercommandreceived:
        handle_player_command_received(np);
      case nm_servermessage:
        CLIENT_ONLY("server message");
        handle_server_message(np);
      break;
      case nm_playermessage:
        handle_player_message(np);
      break;
      case nm_playercommands:
        NOT_DELTA_CLIENT("player commands");
        handle_player_commands(np);
      break;
      case nm_savegamenamechange:
        NOT_DELTA_CLIENT("save game name change");
        handle_save_game_name_change(np);
      break;
      case nm_playerpreferencechange:
        NOT_DELTA_CLIENT("player preference change");
        handle_player_preference_change(np);
      break;
      case nm_statereceived:
        DELTA_SERVER_ONLY("state received");
        handle_state_received(np);
      break;
      case nm_authrequest:
        SERVER_ONLY("authorization request");
        handle_auth_request(np);
      break;
      case nm_rconcommand:
        SERVER_ONLY("RCON command");
        handle_rcon(np);
      break;
      case nm_voterequest:
        SERVER_ONLY("vote request");
        handle_vote_request(np);
      break;
      default:
        doom_printf("Received unknown message type %u from peer %s:%u.\n",
          message_type,
          N_IPToConstString(doom_b32(np->peer->address.host)),
          np->peer->address.port
        );
      break;
    }
  }
}

buf_t* N_GetMessageRecipientBuffer(void) {
  M_BufferEnsureCapacity(
    &message_recipients, MAXPLAYERS * sizeof(unsigned short)
  );
  M_BufferZero(&message_recipients);

  return &message_recipients;
}

void SV_SetupNewPeer(int peernum) {
  short playernum;
  netpeer_t *np = N_GetPeer(peernum);

  if (np == NULL) {
    doom_printf("SV_SetupNewPlayer: invalid peer %d.\n", peernum);
    return;
  }

  for (playernum = 0; playernum < MAXPLAYERS; playernum++) {
    if (!playeringame[playernum]) {
      break;
    }
  }

  if (playernum == MAXPLAYERS) {
    N_DisconnectPeer(peernum);
    return;
  }

  playeringame[playernum] = true;
  P_SpawnPlayer(playernum, &playerstarts[playernum]);
  np->playernum = playernum;

  SV_SendSetup(playernum);

  SV_SendFullState(playernum);
}

void SV_SendSetup(short playernum) {
  netpeer_t *np = NULL;
  CHECK_VALID_PLAYER(np, playernum);

  N_PackSetup(np);
}

void SV_SendFullState(short playernum) {
  netpeer_t *np = NULL;
  CHECK_VALID_PLAYER(np, playernum);

  N_PackFullState(np, N_GetCurrentState());
  np->last_sync_sent_tic = gametic;
}

void SV_SendStateDelta(short playernum) {
  netpeer_t *np = NULL;
  CHECK_VALID_PLAYER(np, playernum);

  N_BuildStateDelta(np);
  N_PackStateDelta(np, np->last_sync_received_tic, gametic, &np->delta);
  np->last_sync_sent_tic = gametic;
}

void SV_SendAuthResponse(short playernum, auth_level_e auth_level) {
  netpeer_t *np = NULL;
  CHECK_VALID_PLAYER(np, playernum);

  N_PackAuthResponse(np, auth_level);
}

void SV_SendPlayerCommandReceived(short playernum, int tic) {
  netpeer_t *np = NULL;
  CHECK_VALID_PLAYER(np, playernum);

  N_PackPlayerCommandReceived(np, tic);
}

void SV_SendMessage(short playernum, char *message) {
  netpeer_t *np = NULL;
  CHECK_VALID_PLAYER(np, playernum);

  N_PackServerMessage(np, message);
}

void SV_BroadcastMessage(char *message) {
  for (int i = 0; i < N_GetPeerCount(); i++) {
    netpeer_t *np = N_GetPeer(i);

    if (np != NULL)
      N_PackServerMessage(np, message);
  }
}

void CL_SendMessageToServer(char *message) {
  CL_SendMessageToPlayer(-1, message);
}

void CL_SendMessageToPlayer(short recipient, char *message) {
  buf_t *recipients = N_GetMessageRecipientBuffer();
  netpeer_t *np = NULL;
  CHECK_CONNECTION(np);

  ((short *)message_recipients.data)[0] = recipient;

  N_PackPlayerMessage(np, consoleplayer, 1, recipients, message);
}

void CL_SendMessageToTeam(byte team, char *message) {
  buf_t *recipients = N_GetMessageRecipientBuffer();
  size_t recipient_count = 0;
  netpeer_t *np = NULL;
  CHECK_CONNECTION(np);

  for (int i = 0; i < MAXPLAYERS; i++) {
    if (players[i].team == team) {
      ((dboolean *)message_recipients.data)[i] = true;
      recipient_count++;
    }
  }

  N_PackPlayerMessage(np, consoleplayer, recipient_count, recipients, message);
}

void CL_SendMessageToCurrentTeam(char *message) {
  CL_SendMessageToTeam(players[consoleplayer].team, message);
}

void CL_SendPlayerCommandReceived(int tic) {
  netpeer_t *np = NULL;
  CHECK_CONNECTION(np);

  N_PackPlayerCommandReceived(np, tic);
}

void CL_SendCommands(void) {
  netpeer_t *np = NULL;
  CHECK_CONNECTION(np);

  N_PackPlayerCommands(np, consoleplayer);
}

void CL_SendSaveGameNameChange(char *new_save_game_name) {
  netpeer_t *np = NULL;
  CHECK_CONNECTION(np);

  N_PackSaveGameNameChange(np, new_save_game_name);
}

void CL_SendNameChange(char *new_name) {
  netpeer_t *np = NULL;
  CHECK_CONNECTION(np);

  N_PackNameChange(np, consoleplayer, new_name);
}

void SV_BroadcastPlayerNameChanged(short playernum, char *new_name) {
  for (int i = 0; i < N_GetPeerCount(); i++) {
    netpeer_t *np = N_GetPeer(i);

    if (np != NULL && np->playernum != playernum)
      N_PackNameChange(np, playernum, new_name);
  }
}

void CL_SendTeamChange(byte new_team) {
  netpeer_t *np = NULL;
  CHECK_CONNECTION(np);

  N_PackTeamChange(np, consoleplayer, new_team);
}

void SV_BroadcastPlayerTeamChanged(short playernum, byte new_team) {
  for (int i = 0; i < N_GetPeerCount(); i++) {
    netpeer_t *np = N_GetPeer(i);

    if (np != NULL && np->playernum != playernum)
      N_PackTeamChange(np, playernum, new_team);
  }
}

void CL_SendPWOChange(void) {
  netpeer_t *np = NULL;
  CHECK_CONNECTION(np);
  /* CG: TODO */
}

void SV_BroadcastPlayerPWOChanged(short playernum) {
  /* CG: TODO */
}

void CL_SendWSOPChange(byte new_wsop_flags) {
  netpeer_t *np = NULL;
  CHECK_CONNECTION(np);

  N_PackWSOPChange(np, consoleplayer, new_wsop_flags);
}

void SV_BroadcastPlayerWSOPChanged(short playernum, byte new_wsop_flags) {
  for (int i = 0; i < N_GetPeerCount(); i++) {
    netpeer_t *np = N_GetPeer(i);

    if (np != NULL && np->playernum != playernum)
      N_PackWSOPChange(np, playernum, new_wsop_flags);
  }
}

void CL_SendBobbingChange(double new_bobbing_amount) {
  netpeer_t *np = NULL;
  CHECK_CONNECTION(np);

  N_PackBobbingChange(np, consoleplayer, new_bobbing_amount);
}

void SV_BroadcastPlayerBobbingChanged(short playernum,
                                      double new_bobbing_amount) {
  for (int i = 0; i < N_GetPeerCount(); i++) {
    netpeer_t *np = N_GetPeer(i);

    if (np != NULL && np->playernum != playernum)
      N_PackBobbingChange(np, playernum, new_bobbing_amount);
  }
}

void CL_SendAutoaimChange(dboolean new_autoaim_enabled) {
  netpeer_t *np = NULL;
  CHECK_CONNECTION(np);

  N_PackAutoaimChange(np, consoleplayer, new_autoaim_enabled);
}

void SV_BroadcastPlayerAutoaimChanged(short playernum,
                                      dboolean new_autoaim_enabled) {
  for (int i = 0; i < N_GetPeerCount(); i++) {
    netpeer_t *np = N_GetPeer(i);

    if (np != NULL && np->playernum != playernum)
      N_PackAutoaimChange(np, playernum, new_autoaim_enabled);
  }
}

void CL_SendWeaponSpeedChange(byte new_weapon_speed) {
  netpeer_t *np = NULL;
  CHECK_CONNECTION(np);

  N_PackWeaponSpeedChange(np, consoleplayer, new_weapon_speed);
}

void SV_BroadcastPlayerWeaponSpeedChanged(short playernum,
                                          byte new_weapon_speed) {
  for (int i = 0; i < N_GetPeerCount(); i++) {
    netpeer_t *np = N_GetPeer(i);

    if (np != NULL && np->playernum != playernum)
      N_PackWeaponSpeedChange(np, playernum, new_weapon_speed);
  }
}

void CL_SendColorChange(byte new_red, byte new_green, byte new_blue) {
  netpeer_t *np = NULL;
  CHECK_CONNECTION(np);

  N_PackColorChange(np, consoleplayer, new_red, new_green, new_blue);
}

void SV_BroadcastPlayerColorChanged(short playernum, byte new_red,
                                                     byte new_green,
                                                     byte new_blue) {
  for (int i = 0; i < N_GetPeerCount(); i++) {
    netpeer_t *np = N_GetPeer(i);

    if (np != NULL && np->playernum != playernum)
      N_PackColorChange(np, playernum, new_red, new_green, new_blue);
  }
}

void CL_SendColormapChange(int new_color) {
  netpeer_t *np = NULL;
  CHECK_CONNECTION(np);

  N_PackColormapChange(np, consoleplayer, new_color);
}

void SV_BroadcastPlayerColormapChanged(short playernum, int new_color) {
  for (int i = 0; i < N_GetPeerCount(); i++) {
    netpeer_t *np = N_GetPeer(i);

    if (np != NULL && np->playernum != playernum)
      N_PackColormapChange(np, consoleplayer, new_color);
  }
}

void CL_SendSkinChange(void) {
  netpeer_t *np = NULL;
  CHECK_CONNECTION(np);
  /* CG: TODO */
}

void SV_BroadcastPlayerSkinChanged(short playernum) {
  /* CG: TODO */
}

void CL_SendStateReceived(int tic) {
  netpeer_t *np = NULL;
  CHECK_CONNECTION(np);

  N_PackStateReceived(np, tic);
}

void CL_SendAuthRequest(char *password) {
  netpeer_t *np = NULL;
  CHECK_CONNECTION(np);

  N_PackAuthRequest(np, password);
}

void CL_SendRCONCommand(char *command) {
  netpeer_t *np = NULL;
  CHECK_CONNECTION(np);

  N_PackRCONCommand(np, command);
}

void CL_SendVoteRequest(char *command) {
  netpeer_t *np = NULL;
  CHECK_CONNECTION(np);

  N_PackVoteRequest(np, command);
}

/* vi: set et ts=2 sw=2: */

