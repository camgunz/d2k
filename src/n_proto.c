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

#include "c_main.h"
#include "d_deh.h"
#include "d_event.h"
#include "d_main.h"
#include "g_game.h"
#include "g_state.h"
#include "m_misc.h"
#include "n_main.h"
#include "p_user.h"
#include "cl_main.h"
#include "cl_net.h"
#include "s_sound.h"
#include "sounds.h"
#include "sv_main.h"
#include "w_wad.h"

const char *D_dehout(void); /* CG: from d_main.c */

#define MAX_PREF_NAME_SIZE 20

#define SERVER_ONLY(name)                                                     \
  if (!SERVER) {                                                              \
    P_Printf(consoleplayer,                                                   \
      "%s: Erroneously received message [%s] from the server\n",              \
      __func__, name                                                          \
    );                                                                        \
    continue;                                                                 \
  }

#define CLIENT_ONLY(name)                                                     \
  if (SERVER) {                                                               \
    P_Printf(consoleplayer,                                                   \
      "%s: Erroneously received message [%s] from a client\n",                \
      __func__, name                                                          \
    );                                                                        \
    continue;                                                                 \
  }

#define CHECK_VALID_PLAYER(np, playernum)                                     \
  if (playernum < 0 || playernum >= MAXPLAYERS)                               \
    I_Error("%s: Invalid player %d.\n", __func__, playernum);                 \
  if (!playeringame[playernum])                                               \
    I_Error("%s: Invalid player %d.\n", __func__, playernum);                 \
  if (((np) = N_PeerForPlayer(playernum)) == NULL)                            \
    I_Error("%s: Invalid player %d.\n", __func__, playernum);                 \
  if (N_PeerGetPlayernum(np) != playernum) {                                  \
    I_Error("%s: Peer playernum mismatch (%d != %d)\n",                       \
      __func__,                                                               \
      N_PeerGetPlayernum(np),                                                 \
      playernum                                                               \
    );                                                                        \
  }

#define CHECK_CONNECTION(np)                                                  \
  if (((np) = CL_GetServerPeer()) == NULL) {                                  \
    P_Printf(consoleplayer, "%s: Not connected\n", __func__);                 \
    return;                                                                   \
  }

static void display_chat_message(chat_channel_e chat_channel,
                                 unsigned short sender,
                                 const char *message) {
  int sfx;
  const char *sender_name;
  const char *sender_opener;
  const char *sender_closer;
  const char *channel;

  if (gamemode == commercial) {
    sfx = sfx_radio;
  }
  else {
    sfx = sfx_tink;
  }

  if (sender < 0) {
    sender_name = "SERVER";
    sender_opener = "[";
    sender_closer = "]";
  }
  else if ((sender >= MAXPLAYERS) || (!playeringame[sender])) {
    D_Msg(MSG_WARN, "Invalid message sender %d\n", sender);
    return;
  }
  else {
    sender_name = players[sender].name;
    sender_opener = "&lt;";
    sender_closer = "&gt;";
  }

  switch (chat_channel)
  {
    case CHAT_CHANNEL_SERVER:
      channel = " (SERVER)";
    break;
    case CHAT_CHANNEL_TEAM:
      channel = " (TEAM)";
    break;
    case CHAT_CHANNEL_PLAYER:
      channel = " (PRIVATE)";
    break;
    case CHAT_CHANNEL_ALL:
      channel = "";
    break;
    default:
      D_Msg(MSG_WARN, "Invalid chat channel %d\n", chat_channel);
      return;
    break;
  }

  /*
   * CG [TODO]: Pretty sure this has to be printed to all recipients' message
   *            buffers for it to work right....
   */

  P_SPrintf(consoleplayer, sfx, "%s%s%s%s: %s\n",
    sender_opener,
    sender_name,
    sender_closer,
    channel,
    message
  );
}

static void handle_setup(netpeer_t *np) {
  netpeer_t *server = CL_GetServerPeer();
  unsigned short player_count = 0;
  unsigned short playernum = 0;
  int i;

  D_Msg(MSG_NET, "Handling setup.\n");

  if (!server) {
    return;
  }

  G_ClearStates();

  if (!N_UnpackSetup(np, &player_count, &playernum)) {
    D_ClearResourceFiles();
    D_ClearDEHFiles();
    G_ClearStates();
    N_Disconnect();
    return;
  }

  W_Init();

  for (i = 0; i < MAXPLAYERS; i++) {
    playeringame[i] = false;
  }

  for (i = 0; i < player_count; i++) {
    playeringame[i] = true;
  }

  consoleplayer = playernum;

  if (!playeringame[consoleplayer]) {
    I_Error("consoleplayer not in game");
  }

  N_PeerSetSyncTIC(np, G_GetLatestState()->tic);

  N_PeerSyncSetHasGameInfo(server);
  N_PeerSyncSetHasGameState(server);
  N_PeerSyncSetOutdated(server);

  if (gamestate == GS_INTERMISSION) {
    G_LoadLatestState(true);
  }
}

static void handle_setup_request(netpeer_t *np) {
  if (!N_PeerCanRequestSetup(np)) {
    return;
  }

  SV_SendSetup(N_PeerGetPlayernum(np));
  N_PeerUpdateLastSetupRequestTime(np);
}

static void handle_full_state(netpeer_t *np) {
  N_UnpackFullState(np);
  N_PeerSyncSetHasGameState(np);
}

static void handle_auth_response(netpeer_t *np) {
  auth_level_e level;

  if (!N_UnpackAuthResponse(np, &level)) {
    return;
  }

  CL_SetAuthorizationLevel(level);
}

static void handle_chat_message(netpeer_t *np) {
  static buf_t message_contents;
  static bool initialized_buffer = false;

  bool unpacked_successfully;
  chat_channel_e chat_channel;
  unsigned short message_sender;
  unsigned short message_recipient;

  if (!initialized_buffer) {
    M_BufferInit(&message_contents);
    initialized_buffer = true;
  }

  unpacked_successfully = N_UnpackChatMessage(np,
    &chat_channel,
    &message_sender,
    &message_recipient,
    &message_contents
  );

  if (!unpacked_successfully)
    return;

  if (SERVER) {
    switch (chat_channel) {
      case CHAT_CHANNEL_ALL:
        NETPEER_FOR_EACH(iter) {
          if (iter.np == np) {
            continue;
          }

          N_PackRelayedChatMessage(
            iter.np, message_sender, message_contents.data
          );
        }
      break;
      case CHAT_CHANNEL_TEAM:
        NETPEER_FOR_EACH(iter) {
          unsigned int playernum;

          if (iter.np == np) {
            continue;
          }

          playernum = N_PeerGetPlayernum(iter.np);

          if (players[playernum].team != players[message_sender].team) {
            continue;
          }

          N_PackRelayedTeamChatMessage(
            iter.np, message_sender, message_contents.data
          );
        }
      break;
      case CHAT_CHANNEL_PLAYER:
        NETPEER_FOR_EACH(iter) {
          unsigned int playernum;

          if (iter.np == np) {
            continue;
          }

          playernum = N_PeerGetPlayernum(iter.np);

          if (playernum != message_recipient) {
            continue;
          }

          N_PackRelayedPlayerChatMessage(
            iter.np, message_sender, message_recipient, message_contents.data
          );
        }
      break;
      default:
      break;
    }
  }

  display_chat_message(chat_channel, message_sender, message_contents.data);
}

static void handle_sync(netpeer_t *np) {
  if (SERVER || CL_ReceivedSetup()) {
    N_UnpackSync(np);
  }
}

static void handle_ping(netpeer_t *np) {
  double now = M_GetCurrentTime();
  double server_time;

  if (!N_UnpackPing(np, &server_time)) {
    return;
  }

  if (CLIENT) {
    CL_SendPing(server_time);
  }

  if (SERVER) {
    double ping = (now - server_time) * 1000;

    players[N_PeerGetPlayernum(np)].ping = (int)ping;
  }
}

static void handle_player_preference_change(netpeer_t *np) {
  static buf_t *pref_key_name = NULL;
  static buf_t *pref_key_value = NULL;

  unsigned short playernum = 0;
  int tic = 0;
  unsigned int pref_count = 0;
  player_t *player = &players[N_PeerGetPlayernum(np)];

  if (!pref_key_name) {
    pref_key_name = M_BufferNew();
  }

  if (!pref_key_value) {
    pref_key_value = M_BufferNew();
  }

  if (!N_UnpackPlayerPreferenceChange(np, &tic, &playernum, &pref_count)) {
    return;
  }

  if (SERVER && playernum != N_PeerGetPlayernum(np)) {
    P_Printf(consoleplayer,
      "Received player preference for player %d from peer %d.\n",
      playernum, N_PeerGetPlayernum(np)
    );
    return;
  }

  for (size_t i = 0; i < pref_count; i++) {
    if (!N_UnpackPlayerPreferenceName(np, pref_key_name)) {
      P_Echo(consoleplayer,
        "N_HandlePacket: Error unpacking client preference change"
      );
      return;
    }

    if (pref_key_name->size > MAX_PREF_NAME_SIZE) {
      P_Echo(consoleplayer,
        "N_HandlePacket: Invalid client preference change message: preference "
        "name exceeds maximum length"
      );
      return;
    }

    M_BufferSeek(pref_key_name, 0);

    if (M_BufferEqualsString(pref_key_name, "name")) {
      N_UnpackNameChange(np, pref_key_value);

      P_SetPlayerName(playernum, pref_key_value->data);
    }
    else if (M_BufferEqualsString(pref_key_name, "team")) {
      unsigned char new_team = 0;

      if (!N_UnpackTeamChange(np, &new_team))
        return;

      player->team = new_team;
    }
    else if (M_BufferEqualsString(pref_key_name, "pwo")) {
    }
    else if (M_BufferEqualsString(pref_key_name, "wsop")) {
    }
    else if (M_BufferEqualsString(pref_key_name, "bobbing")) {
    }
    else if (M_BufferEqualsString(pref_key_name, "autoaim")) {
    }
    else if (M_BufferEqualsString(pref_key_name, "weapon speed")) {
    }
    else if (M_BufferEqualsString(pref_key_name, "color")) {
    }
    else if (M_BufferEqualsString(pref_key_name, "color index")) {
      int new_color;

      if (N_UnpackColorIndexChange(np, &new_color)) {
        G_ChangedPlayerColour(N_PeerGetPlayernum(np), new_color);
      }
    }
    else if (M_BufferEqualsString(pref_key_name, "skin name")) {
    }
    else {
      P_Printf(consoleplayer,
        "Unsupported player preference %s.\n", pref_key_name->data
      );
    }
  }
}

static void handle_auth_request(netpeer_t *np) {
  buf_t buf;
  const char *password;
  auth_level_e auth_level = AUTH_LEVEL_NONE;

  M_BufferInit(&buf);

  if (!N_UnpackAuthRequest(np, &buf)) {
    return;
  }

  password = M_BufferGetData(&buf);

  if (strcmp(password, sv_administrate_password) == 0) {
    auth_level = AUTH_LEVEL_ADMINISTRATOR;
  }
  else if (strcmp(password, sv_moderate_password) == 0) {
    auth_level = AUTH_LEVEL_MODERATOR;
  }
  else if ((!sv_join_password) ||
           (strcmp(password, sv_join_password) == 0)) {
    auth_level = AUTH_LEVEL_PLAYER;
  }
  else if ((!sv_spectate_password) ||
           (strcmp(password, sv_spectate_password) == 0)) {
    auth_level = AUTH_LEVEL_SPECTATOR;
  }

  if (auth_level > N_PeerGetAuthLevel(np)) {
    N_PeerSetAuthLevel(np, auth_level);
    SV_SendAuthResponse(N_PeerGetPlayernum(np), N_PeerGetAuthLevel(np));
  }

  M_BufferFree(&buf);
}

static void handle_game_action_change(netpeer_t *np) {
  gameaction_t new_gameaction = 0;
  int new_gametic = 0;

  if (!N_UnpackGameActionChange(np, &new_gameaction, &new_gametic)) {
    return;
  }

  G_SetGameAction(new_gameaction);
  gametic = new_gametic;
}

static void handle_rcon(netpeer_t *np) {
  buf_t rcon_buf;

  M_BufferInit(&rcon_buf);

  if (!N_UnpackRCONCommand(np, &rcon_buf)) {
    return;
  }

  if (N_PeerGetAuthLevel(np) < AUTH_LEVEL_MODERATOR) {
    SV_SendMessage(N_PeerGetPlayernum(np), "Not authorized for RCON usage");
    return;
  }

  /* [CG] TODO: Handle console output somehow */
  C_HandleInput(M_BufferGetData(&rcon_buf));
}

static void handle_vote_request(netpeer_t *np) {
}

void N_HandlePacket(netpeer_t *np, void *data, size_t data_size) {
  net_message_e message_type = NM_NONE;

  if (!N_PeerSetIncoming(np, data, data_size)) {
    D_Msg(MSG_NET, "N_HandlePacket: Ignoring malformed packet.\n");
    return;
  }

  while (N_PeerLoadNextMessage(np, &message_type)) {
    if (message_type > NM_NONE &&
        message_type != NM_SYNC &&
        message_type < NM_MAX) {
      D_Msg(MSG_NET, "Handling [%s] message.\n",
        network_message_info[message_type].name
      );
    }

    switch (message_type) {
      case NM_SETUP:
        if (CLIENT) {
          handle_setup(np);
        }
        else if (SERVER) {
          handle_setup_request(np);
        }
      break;
      case NM_FULL_STATE:
        CLIENT_ONLY("full state");
        handle_full_state(np);
      break;
      case NM_AUTH:
        if (CLIENT) {
          handle_auth_response(np);
        }
        else if (SERVER) {
          handle_auth_request(np);
        }
      break;
      case NM_CHAT_MESSAGE:
        handle_chat_message(np);
      break;
      case NM_SYNC:
        handle_sync(np);
      break;
      case NM_PLAYER_PREFERENCE_CHANGE:
        SERVER_ONLY("player preference change");
        handle_player_preference_change(np);
      break;
      case NM_GAME_ACTION:
        CLIENT_ONLY("game action change");
        handle_game_action_change(np);
      break;
      case NM_RCON_COMMAND:
        SERVER_ONLY("RCON command");
        handle_rcon(np);
      break;
      case NM_VOTE_REQUEST:
        SERVER_ONLY("vote request");
        handle_vote_request(np);
      break;
      case NM_PING:
        handle_ping(np);
      break;
      default:
        P_Printf(consoleplayer,
          "Received unknown message type %d from peer %s:%u.\n",
          message_type,
          N_PeerGetIPAddressConstString(np),
          N_PeerGetPort(np)
        );
      break;
    }
  }
}

void N_UpdateSync(void) {
  if (CLIENT) {
    netpeer_t *server = CL_GetServerPeer();

    if (!server) {
      return;
    }

    if (!N_PeerSyncOutdated(server)) {
      return;
    }

    N_PeerClearChannel(server, network_message_info[NM_SYNC].channel);
    N_PackSync(server);
    N_PeerSyncSetNotOutdated(server);

    return;
  }

  NETPEER_FOR_EACH(iter) {
    if (!N_PeerSynchronized(iter.np)) {
      continue;
    }

    if (N_PeerSyncOutdated(iter.np) && N_PeerGetSyncTIC(iter.np) != 0) {
      N_PeerClearChannel(iter.np, network_message_info[NM_SYNC].channel);
      N_PeerBuildNewSyncStateDelta(iter.np);
      N_PackSync(iter.np);
      N_PeerSyncSetNotOutdated(iter.np);
    }
  }
}

void SV_SendSetup(unsigned short playernum) {
  netpeer_t *np = NULL;
  CHECK_VALID_PLAYER(np, playernum);

  N_PackSetup(np);
  N_PeerSyncSetHasGameInfo(np);
  N_PeerSyncSetHasGameState(np);
}

void SV_SendFullState(unsigned short playernum) {
  netpeer_t *np = NULL;
  CHECK_VALID_PLAYER(np, playernum);

  N_PackFullState(np);
  N_PeerSyncSetHasGameState(np);
}

void SV_SendAuthResponse(unsigned short playernum, auth_level_e auth_level) {
  netpeer_t *np = NULL;
  CHECK_VALID_PLAYER(np, playernum);

  N_PackAuthResponse(np, auth_level);
}

void SV_SendPing(unsigned short playernum) {
  double now = M_GetCurrentTime();
  netpeer_t *np = NULL;
  CHECK_VALID_PLAYER(np, playernum);

  N_PackPing(np, now);
}

void SV_SendMessage(unsigned short playernum, const char *message) {
  netpeer_t *np = NULL;
  CHECK_VALID_PLAYER(np, playernum);

  if (strlen(message) <= 0) {
    return;
  }

  N_PackServerChatMessage(np, message);
}

void SV_BroadcastMessage(const char *message) {
  NETPEER_FOR_EACH(iter) {
    N_PackServerChatMessage(iter.np, message);
  }
}

void SV_BroadcastPrintf(const char *fmt, ...) {
  gchar *gmessage;
  va_list args;

  va_start(args, fmt);
  gmessage = g_strdup_vprintf(fmt, args);
  va_end(args);

  if (strlen(gmessage) <= 0) {
    return;
  }

  SV_BroadcastMessage(gmessage);
  g_free(gmessage);
}

void CL_SendMessageToServer(const char *message) {
  netpeer_t *np = CL_GetServerPeer();

  if (strlen(message) <= 0) {
    return;
  }

  display_chat_message(CHAT_CHANNEL_SERVER, consoleplayer, message);

  if (!np) {
    return;
  }

  N_PackServerChatMessage(np, message);
}

void CL_SendMessageToPlayer(unsigned short recipient, const char *message) {
  netpeer_t *np = CL_GetServerPeer();

  if (strlen(message) <= 0) {
    return;
  }

  display_chat_message(CHAT_CHANNEL_PLAYER, consoleplayer, message);

  if (!np) {
    return;
  }

  if (!playeringame[recipient]) {
    return;
  }

  N_PackPlayerChatMessage(np, recipient, message);
}

void CL_SendMessageToTeam(const char *message) {
  netpeer_t *np = CL_GetServerPeer();

  if (strlen(message) <= 0) {
    return;
  }

  display_chat_message(CHAT_CHANNEL_TEAM, consoleplayer, message);

  if (!np) {
    return;
  }

  N_PackTeamChatMessage(np, message);
}

void CL_SendMessage(const char *message) {
  netpeer_t *np = CL_GetServerPeer();

  if (strlen(message) <= 0) {
    return;
  }

  display_chat_message(CHAT_CHANNEL_ALL, consoleplayer, message);

  if (!np) {
    return;
  }

  N_PackChatMessage(np, message);
}

void CL_SendNameChange(const char *new_name) {
  netpeer_t *np = NULL;
  CHECK_CONNECTION(np);

  N_PackNameChange(np, consoleplayer, new_name);
}

void SV_BroadcastPlayerNameChanged(unsigned short playernum,
                                   const char *new_name) {
  NETPEER_FOR_EACH(iter) {
    unsigned int peer_playernum = N_PeerGetPlayernum(iter.np);

    if (peer_playernum != playernum) {
      N_PackNameChange(iter.np, playernum, new_name);
    }
  }
}

void CL_SendTeamChange(unsigned char new_team) {
  netpeer_t *np = NULL;
  CHECK_CONNECTION(np);

  N_PackTeamChange(np, consoleplayer, new_team);
}

void CL_SendSetupRequest(void) {
  netpeer_t *np = NULL;
  CHECK_CONNECTION(np);

  N_PackSetupRequest(np);
}

void CL_SendPing(double server_time) {
  netpeer_t *np = NULL;
  CHECK_CONNECTION(np);

  N_PackPing(np, server_time);
}

void SV_BroadcastPlayerTeamChanged(unsigned short playernum,
                                   unsigned char new_team) {
  NETPEER_FOR_EACH(iter) {
    unsigned int peer_playernum = N_PeerGetPlayernum(iter.np);

    if (peer_playernum != playernum) {
      N_PackTeamChange(iter.np, playernum, new_team);
    }
  }
}

void CL_SendPWOChange(void) {
  netpeer_t *np = NULL;
  CHECK_CONNECTION(np);
  /* CG: TODO */
}

void SV_BroadcastPlayerPWOChanged(unsigned short playernum) {
  /* CG: TODO */
}

void CL_SendWSOPChange(unsigned char new_wsop_flags) {
  netpeer_t *np = NULL;
  CHECK_CONNECTION(np);

  N_PackWSOPChange(np, consoleplayer, new_wsop_flags);
}

void SV_BroadcastPlayerWSOPChanged(unsigned short playernum,
                                   unsigned char new_wsop_flags) {
  NETPEER_FOR_EACH(iter) {
    unsigned int peer_playernum = N_PeerGetPlayernum(iter.np);

    if (peer_playernum != playernum) {
      N_PackWSOPChange(iter.np, playernum, new_wsop_flags);
    }
  }
}

void CL_SendBobbingChange(double new_bobbing_amount) {
  netpeer_t *np = NULL;
  CHECK_CONNECTION(np);

  N_PackBobbingChange(np, consoleplayer, new_bobbing_amount);
}

void SV_BroadcastPlayerBobbingChanged(unsigned short playernum,
                                      double new_bobbing_amount) {
  NETPEER_FOR_EACH(iter) {
    unsigned int peer_playernum = N_PeerGetPlayernum(iter.np);

    if (peer_playernum != playernum) {
      N_PackBobbingChange(iter.np, playernum, new_bobbing_amount);
    }
  }
}

void CL_SendAutoaimChange(bool new_autoaim_enabled) {
  netpeer_t *np = NULL;
  CHECK_CONNECTION(np);

  N_PackAutoaimChange(np, consoleplayer, new_autoaim_enabled);
}

void SV_BroadcastPlayerAutoaimChanged(unsigned short playernum,
                                      bool new_autoaim_enabled) {
  NETPEER_FOR_EACH(iter) {
    unsigned int peer_playernum = N_PeerGetPlayernum(iter.np);

    if (peer_playernum != playernum) {
      N_PackAutoaimChange(iter.np, playernum, new_autoaim_enabled);
    }
  }
}

void CL_SendWeaponSpeedChange(unsigned char new_weapon_speed) {
  netpeer_t *np = NULL;
  CHECK_CONNECTION(np);

  N_PackWeaponSpeedChange(np, consoleplayer, new_weapon_speed);
}

void SV_BroadcastPlayerWeaponSpeedChanged(unsigned short playernum,
                                          unsigned char new_weapon_speed) {
  NETPEER_FOR_EACH(iter) {
    unsigned int peer_playernum = N_PeerGetPlayernum(iter.np);

    if (peer_playernum != playernum) {
      N_PackWeaponSpeedChange(iter.np, playernum, new_weapon_speed);
    }
  }
}

void CL_SendColorChange(unsigned char new_red,
                        unsigned char new_green,
                        unsigned char new_blue) {
  netpeer_t *np = NULL;
  CHECK_CONNECTION(np);

  N_PackColorChange(np, consoleplayer, new_red, new_green, new_blue);
}

void SV_BroadcastPlayerColorChanged(unsigned short playernum,
                                    unsigned char new_red,
                                    unsigned char new_green,
                                    unsigned char new_blue) {
  NETPEER_FOR_EACH(iter) {
    unsigned int peer_playernum = N_PeerGetPlayernum(iter.np);

    if (peer_playernum != playernum) {
      N_PackColorChange(iter.np, playernum, new_red, new_green, new_blue);
    }
  }
}

void CL_SendColorIndexChange(int new_color) {
  netpeer_t *np = NULL;
  CHECK_CONNECTION(np);

  N_PackColorIndexChange(np, consoleplayer, new_color);
}

void SV_BroadcastPlayerColorIndexChanged(unsigned short playernum,
                                         int new_color) {
  NETPEER_FOR_EACH(iter) {
    unsigned int peer_playernum = N_PeerGetPlayernum(iter.np);

    if (peer_playernum != playernum) {
      N_PackColorIndexChange(iter.np, consoleplayer, new_color);
    }
  }
}

void CL_SendSkinChange(void) {
  netpeer_t *np = NULL;
  CHECK_CONNECTION(np);
  /* CG: TODO */
}

void SV_BroadcastPlayerSkinChanged(unsigned short playernum) {
  /* CG: TODO */
}

void SV_ResyncPeers(void) {
  NETPEER_FOR_EACH(iter) {
    N_PeerResetSync(iter.np);
  }
}

void SV_BroadcastGameActionChange(void) {
  NETPEER_FOR_EACH(iter) {
    N_PackGameActionChange(iter.np);
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

