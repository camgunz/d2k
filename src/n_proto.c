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

#include <enet/enet.h>

#include "doomdef.h"
#include "doomstat.h"
#include "d_deh.h"
#include "d_main.h"
#include "d_ticcmd.h"
#include "g_game.h"
#include "lprintf.h"
#include "p_user.h"
#include "s_sound.h"
#include "sounds.h"
#include "w_wad.h"

#include "n_net.h"
#include "n_main.h"
#include "n_state.h"
#include "n_peer.h"
#include "n_pack.h"
#include "n_proto.h"

const char *D_dehout(void); /* CG: from d_main.c */

#define MAX_PREF_NAME_SIZE 20

#define COMMAND_SYNC_ONLY(name)                                               \
  if (!CMDSYNC) {                                                             \
    P_Printf(consoleplayer,                                                   \
      "%s: Erroneously received command-sync message [%s] in delta-sync "     \
      "mode\n",                                                               \
      __func__, name                                                          \
    );                                                                        \
    return;                                                                   \
  }

#define DELTA_SYNC_ONLY(name)                                                 \
  if (!DELTASYNC) {                                                           \
    P_Printf(consoleplayer,                                                   \
      "%s: Erroneously received delta-sync message [%s] in command-sync "     \
      "mode\n",                                                               \
      __func__, name                                                          \
    );                                                                        \
    return;                                                                   \
  }

#define SERVER_ONLY(name)                                                     \
  if (!SERVER) {                                                              \
    P_Printf(consoleplayer,                                                   \
      "%s: Erroneously received message [%s] from the server\n",              \
      __func__, name                                                          \
    );                                                                        \
    return;                                                                   \
  }

#define CLIENT_ONLY(name)                                                     \
  if (SERVER) {                                                               \
    P_Printf(consoleplayer,                                                   \
      "%s: Erroneously received message [%s] from a client\n",                \
      __func__, name                                                          \
    );                                                                        \
    return;                                                                   \
  }

#define NOT_DELTA_SERVER(name)                                                \
  if (DELTASERVER) {                                                          \
    P_Printf(consoleplayer,                                                   \
      "%s: Erroneously received message [%s] as a delta-sync server\n",       \
      __func__, name                                                          \
    );                                                                        \
    return;                                                                   \
  }

#define NOT_DELTA_CLIENT(name)                                                \
  if (DELTACLIENT) {                                                          \
    P_Printf(consoleplayer,                                                   \
      "%s: Erroneously received message [%s] as a delta-sync client\n",       \
      __func__, name                                                          \
    );                                                                        \
    return;                                                                   \
  }

#define NOT_COMMAND_SERVER(name)                                              \
  if (CMDSERVER) {                                                            \
    P_Printf(consoleplayer,                                                   \
      "%s: Erroneously received message [%s] as a command-sync server "       \
      "mode\n",                                                               \
      __func__, name                                                          \
    );                                                                        \
    return;                                                                   \
  }

#define NOT_COMMAND_CLIENT(name)                                              \
  if (CMDCLIENT) {                                                            \
    P_Printf(consoleplayer,                                                   \
      "%s: Erroneously received message [%s] as a command-sync client"        \
      "mode\n",                                                               \
      __func__, name                                                          \
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
  if (((np) = N_PeerForPlayer(playernum)) == NULL)                            \
    I_Error("%s: Invalid player %d.\n", __func__, playernum);                 \
  if (np->playernum < 0 || np->playernum >= MAXPLAYERS)                       \
    I_Error("%s: Invalid player %d.\n", __func__, np->playernum);             \
  if (!playeringame[np->playernum])                                           \
    I_Error("%s: Invalid player %d.\n", __func__, np->playernum);

#define CHECK_CONNECTION(np)                                                  \
  if (((np) = N_PeerGet(0)) == NULL) {                                        \
    P_Printf(consoleplayer, "%s: Not connected\n", __func__);                 \
    return;                                                                   \
  }

const char *nm_names[9] = {
  "setup",
  "auth response",
  "server message",
  "sync",
  "player message",
  "player preference change",
  "auth request",
  "RCON command",
  "vote request"
};

static buf_t* get_message_recipient_buffer(void) {
  static buf_t *recipients = NULL;

  if (!recipients)
    recipients = M_BufferNew();

  M_BufferEnsureCapacity(recipients, MAXPLAYERS * sizeof(unsigned short));
  M_BufferClear(recipients);

  return recipients;
}

static void handle_setup(netpeer_t *np) {
  netpeer_t *server = N_PeerGet(0);
  net_sync_type_e net_sync = NET_SYNC_TYPE_NONE;
  unsigned short player_count = 0;
  unsigned short playernum = 0;
  int i;

  D_Log(LOG_NET, "Handling setup.\n");

  if (server == NULL)
    return;

  N_ClearStates();
  // N_ResetLocalCommandIndex();

  if (!N_UnpackSetup(np, &net_sync, &player_count, &playernum)) {
    M_CBufFree(&resource_files_buf);
    M_CBufFree(&deh_files_buf);
    N_ClearStates();
    N_Disconnect();
    return;
  }

  netsync = net_sync;

  for (i = 0; i < MAXPLAYERS; i++)
    playeringame[i] = false;

  for (i = 0; i < player_count; i++)
    playeringame[i] = true;

  consoleplayer = playernum;

  if (!playeringame[consoleplayer])
    I_Error("consoleplayer not in game");

  np->sync.tic = N_GetLatestState()->tic;

  CL_SetReceivedSetup(true);
  server->sync.outdated = true;
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
    P_Printf(consoleplayer, "[SERVER]: %s\n", server_message_buffer.data);
}

static void handle_sync(netpeer_t *np) {
  dboolean update_sync = false;

  if (DELTACLIENT) {
    N_UnpackDeltaSync(np);
    return;
  }

  if ((!DELTACLIENT) && (!N_UnpackSync(np, &update_sync)))
    return;

  if (update_sync) {
    if (SERVER) {
      for (int i = 0; i < N_PeerGetCount(); i++) {
        netpeer_t *client = N_PeerGet(i);

        if (client != NULL)
          client->sync.outdated = true;
      }
    }
    else {
      np->sync.outdated = true;
    }
  }
}

static void handle_player_message(netpeer_t *np) {
  static buf_t *player_message_buffer = NULL;

  short sender = 0;
  dboolean unpacked_successfully = false;
  buf_t *message_recipients = get_message_recipient_buffer();
  int sfx;

  if (!player_message_buffer)
    player_message_buffer = M_BufferNew();

  unpacked_successfully = N_UnpackPlayerMessage(
    np, &sender, message_recipients, player_message_buffer
  );

  if (!unpacked_successfully)
    return;

  if (gamemode == commercial)
    sfx = sfx_radio;
  else
    sfx = sfx_tink;


  if (SERVER) {
    size_t recipient_count =
      M_BufferGetSize(message_recipients) / sizeof(unsigned short);

    M_BufferSeek(message_recipients, 0);

    while (recipient_count--) {
      netpeer_t *np;
      unsigned short recipient;

      M_BufferReadUShort(message_recipients, &recipient);

      np = N_PeerForPlayer(recipient);

      if (np == NULL)
        continue;

      N_PackRelayedPlayerMessage(
        np, sender, recipient, M_BufferGetData(player_message_buffer)
      );
    }
  }

  if (sender != consoleplayer) {
    P_SPrintf(displayplayer, sfx, "<%s>: %s\n",
      players[sender].name,
      M_BufferGetData(player_message_buffer)
    );
  }
}

static void handle_player_preference_change(netpeer_t *np) {
  static buf_t pref_key_name;
  static buf_t pref_key_value;
  static dboolean initialized_buffers = false;

  short playernum = 0;
  int tic = 0;
  unsigned int pref_count = 0;
  player_t *player = &players[np->playernum];

  if (!initialized_buffers) {
    M_BufferInit(&pref_key_name);
    M_BufferInit(&pref_key_value);
    initialized_buffers = true;
  }

  if (!N_UnpackPlayerPreferenceChange(np, &tic, &playernum, &pref_count))
    return;

  if (SERVER && playernum != np->playernum) {
    P_Printf(consoleplayer,
      "Received player preference for player %d from peer %d.\n",
      playernum, np->playernum
    );
    return;
  }

  for (size_t i = 0; i < pref_count; i++) {
    if (!N_UnpackPlayerPreferenceName(np, &pref_key_name)) {
      P_Echo(consoleplayer,
        "N_HandlePacket: Error unpacking client preference change"
      );
      return;
    }

    if (pref_key_name.size > MAX_PREF_NAME_SIZE) {
      P_Echo(consoleplayer,
        "N_HandlePacket: Invalid client preference change message: preference "
        "name exceeds maximum length"
      );
      return;
    }

    M_BufferSeek(&pref_key_name, 0);

    if (M_BufferEqualsString(&pref_key_name, "name")) {
      N_UnpackNameChange(np, &pref_key_value);

      if (player->name != NULL) {
        P_Printf(consoleplayer,
          "%s is now %s.\n",
          player->name, pref_key_value.data
        );
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
    else if (M_BufferEqualsString(&pref_key_name, "weapon speed")) {
    }
    else if (M_BufferEqualsString(&pref_key_name, "color")) {
    }
    else if (M_BufferEqualsString(&pref_key_name, "color index")) {
      int new_color;

      if (N_UnpackColorIndexChange(np, &new_color))
        G_ChangedPlayerColour(np->playernum, new_color);
    }
    else if (M_BufferEqualsString(&pref_key_name, "skin name")) {
    }
    else {
      P_Printf(consoleplayer,
        "Unsupported player preference %s.\n", pref_key_name.data
      );
    }
  }
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
  netpeer_t *np = N_PeerGet(peernum);
  unsigned char message_type = 0;

  if (!N_PeerLoadIncoming(peernum, data, data_size)) {
    D_Log(LOG_NET, "N_HandlePacket: Ignoring packet with malformed TOC.\n");
    return;
  }

  while (N_PeerLoadNextMessage(peernum, &message_type)) {

    if (message_type >= 1 &&
        message_type != nm_sync &&
        message_type <= sizeof(nm_names)) {
      D_Log(LOG_NET, "Handling [%s] message.\n", nm_names[message_type - 1]);
    }

    switch (message_type) {
      case nm_setup:
        CLIENT_ONLY("setup");
        handle_setup(np);
      break;
      case nm_authresponse:
        CLIENT_ONLY("authorization response");
        handle_auth_response(np);
      break;
      case nm_servermessage:
        CLIENT_ONLY("server message");
        handle_server_message(np);
      break;
      case nm_sync:
        handle_sync(np);
      break;
      case nm_playermessage:
        handle_player_message(np);
      break;
      case nm_playerpreferencechange:
        NOT_DELTA_CLIENT("player preference change");
        handle_player_preference_change(np);
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
        P_Printf(consoleplayer,
          "Received unknown message type %u from peer %s:%u.\n",
          message_type,
          N_IPToConstString(doom_b32(np->peer->address.host)),
          np->peer->address.port
        );
      break;
    }
  }
}

void N_UpdateSync(void) {
  netpeer_t *np = NULL;

  if (CLIENT) {
    np = N_PeerGet(0);

    if (np != NULL && np->sync.outdated) {
      N_PeerClearUnreliable(np->peernum);
      N_PackSync(np);
      np->sync.outdated = false;
    }
  }
  else {
    for (int i = 0; i < N_PeerGetCount(); i++) {
      np = N_PeerGet(i);

      if (np != NULL && np->sync.outdated && np->sync.tic != 0) {
        N_PeerClearUnreliable(np->peernum);

        if (DELTASERVER) {
          if (np->sync.tic < N_GetLatestState()->tic)
            N_BuildStateDelta(np->sync.tic, &np->sync.delta);

          N_PackDeltaSync(np);
        }
        else {
          N_PackSync(np);
        }

        np->sync.outdated = false;
      }
    }
  }
}

void SV_SetupNewPeer(int peernum) {
  short playernum;
  netpeer_t *np = N_PeerGet(peernum);

  if (np == NULL)
    I_Error("SV_SetupNewPlayer: invalid peer %d.\n", peernum);

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
  players[playernum].playerstate = PST_REBORN;
  np->playernum = playernum;
  np->sync.tic = gametic;
}

void SV_SendSetup(short playernum) {
  netpeer_t *np = NULL;
  CHECK_VALID_PLAYER(np, playernum);

  np->sync.initialized = true;
  N_PackSetup(np);
}

void SV_SendAuthResponse(short playernum, auth_level_e auth_level) {
  netpeer_t *np = NULL;
  CHECK_VALID_PLAYER(np, playernum);

  N_PackAuthResponse(np, auth_level);
}

void SV_SendMessage(short playernum, const char *message) {
  netpeer_t *np = NULL;
  CHECK_VALID_PLAYER(np, playernum);

  N_PackServerMessage(np, message);
}

void SV_BroadcastMessage(const char *message) {
  for (int i = 0; i < N_PeerGetCount(); i++) {
    netpeer_t *np = N_PeerGet(i);

    if (np != NULL)
      N_PackServerMessage(np, message);
  }
}

void CL_SendMessageToServer(const char *message) {
  CL_SendMessageToPlayer(-1, message);
}

void CL_SendMessageToPlayer(short recipient, const char *message) {
  buf_t *recipients = get_message_recipient_buffer();
  netpeer_t *np = NULL;
  CHECK_CONNECTION(np);

  if (recipient != -1 && !playeringame[recipient])
    return;

  M_BufferWriteShort(recipients, recipient);

  N_PackPlayerMessage(np, consoleplayer, recipients, message);
}

void CL_SendMessageToTeam(byte team, const char *message) {
  buf_t *recipients = get_message_recipient_buffer();
  netpeer_t *np = NULL;
  CHECK_CONNECTION(np);

  for (short i = 0; i < MAXPLAYERS; i++) {
    if (playeringame[i] && players[i].team == team) {
      M_BufferWriteShort(recipients, i);
    }
  }

  N_PackPlayerMessage(np, consoleplayer, recipients, message);
}

void CL_SendMessageToCurrentTeam(const char *message) {
  CL_SendMessageToTeam(players[consoleplayer].team, message);
}

void CL_SendMessage(const char *message) {
  buf_t *recipients = get_message_recipient_buffer();
  netpeer_t *np = NULL;
  CHECK_CONNECTION(np);

  for (short i = 0; i < MAXPLAYERS; i++) {
    if (playeringame[i])
      M_BufferWriteShort(recipients, i);
  }

  N_PackPlayerMessage(np, consoleplayer, recipients, message);
}

void CL_SendNameChange(const char *new_name) {
  netpeer_t *np = NULL;
  CHECK_CONNECTION(np);

  N_PackNameChange(np, consoleplayer, new_name);
}

void SV_BroadcastPlayerNameChanged(short playernum, const char *new_name) {
  for (int i = 0; i < N_PeerGetCount(); i++) {
    netpeer_t *np = N_PeerGet(i);

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
  for (int i = 0; i < N_PeerGetCount(); i++) {
    netpeer_t *np = N_PeerGet(i);

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
  for (int i = 0; i < N_PeerGetCount(); i++) {
    netpeer_t *np = N_PeerGet(i);

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
  for (int i = 0; i < N_PeerGetCount(); i++) {
    netpeer_t *np = N_PeerGet(i);

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
  for (int i = 0; i < N_PeerGetCount(); i++) {
    netpeer_t *np = N_PeerGet(i);

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
  for (int i = 0; i < N_PeerGetCount(); i++) {
    netpeer_t *np = N_PeerGet(i);

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
  for (int i = 0; i < N_PeerGetCount(); i++) {
    netpeer_t *np = N_PeerGet(i);

    if (np != NULL && np->playernum != playernum)
      N_PackColorChange(np, playernum, new_red, new_green, new_blue);
  }
}

void CL_SendColorIndexChange(int new_color) {
  netpeer_t *np = NULL;
  CHECK_CONNECTION(np);

  N_PackColorIndexChange(np, consoleplayer, new_color);
}

void SV_BroadcastPlayerColorIndexChanged(short playernum, int new_color) {
  for (int i = 0; i < N_PeerGetCount(); i++) {
    netpeer_t *np = N_PeerGet(i);

    if (np != NULL && np->playernum != playernum)
      N_PackColorIndexChange(np, consoleplayer, new_color);
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

void SV_ResyncPeers(void) {
  for (int i = 1; i < MAXPLAYERS; i++) {
    netpeer_t *np = N_PeerForPlayer(i);

    if (np == NULL)
      continue;

    N_PeerResetSync(np->peernum);
  }
}

void CL_SendAuthRequest(const char *password) {
  netpeer_t *np = NULL;
  CHECK_CONNECTION(np);

  N_PackAuthRequest(np, password);
}

void CL_SendRCONCommand(const char *command) {
  netpeer_t *np = NULL;
  CHECK_CONNECTION(np);

  N_PackRCONCommand(np, command);
}

void CL_SendVoteRequest(const char *command) {
  netpeer_t *np = NULL;
  CHECK_CONNECTION(np);

  N_PackVoteRequest(np, command);
}

/* vi: set et ts=2 sw=2: */

