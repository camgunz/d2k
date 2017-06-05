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

#include "m_misc.h"
#include "d_deh.h"
#include "d_event.h"
#include "d_main.h"
#include "d_res.h"
#include "g_game.h"
#include "g_state.h"
#include "pl_main.h"
#include "s_sound.h"
#include "sounds.h"
#include "sv_main.h"
#include "w_wad.h"

#include "n_main.h"
#include "n_proto.h"
#include "cl_main.h"
#include "cl_net.h"

#define SERVER_ONLY(name)                                                     \
  if (!SERVER) {                                                              \
    D_MsgLocalError(                                                          \
      "%s: Erroneously received message [%s] from the server\n",              \
      __func__, name                                                          \
    );                                                                        \
    continue;                                                                 \
  }

#define CLIENT_ONLY(name)                                                     \
  if (SERVER) {                                                               \
    D_MsgLocalError(                                                          \
      "%s: Erroneously received message [%s] from a client\n",                \
      __func__, name                                                          \
    );                                                                        \
    continue;                                                                 \
  }

#define CHECK_CONNECTION(np)                                                  \
  if (((np) = CL_GetServerPeer()) == NULL) {                                  \
    D_MsgLocalError("%s: Not connected\n", __func__);                         \
    return;                                                                   \
  }

static void handle_setup(netpeer_t *np) {
  netpeer_t *server = CL_GetServerPeer();

  N_MsgNetLocalDebug("Handling setup.\n");

  if (!server) {
    return;
  }

  if (server != np) {
    N_MsgNetLocalWarn("Received setup for non-server peer\n");
    return;
  }

  G_ClearStates();

  if (!N_UnpackSetup(np)) {
    /*
     * [CG] [TODO] Come to think of it, this stuff should probably go in
     *             N_Disconnect itself....
     */
    D_ClearResourceFiles();
    D_ClearDEHFiles();
    G_ClearStates();
    N_Disconnect(DISCONNECT_REASON_MALFORMED_SETUP);
    return;
  }

  W_Init();

  N_PeerSyncSetTIC(np, G_GetLatestState()->tic);
  N_PeerSyncSetHasGameInfo(server);
  N_PeerSyncSetHasGameState(server);
  N_PeerSyncSetOutdated(server);

  if (G_GetGameState() == GS_INTERMISSION) {
    G_LoadLatestState(true);
  }
}

static void handle_setup_request(netpeer_t *np) {
  if (!N_PeerCanRequestSetup(np)) {
    return;
  }

  N_PeerSyncSetNeedsGameInfo(np);
  N_PeerSyncSetNeedsGameState(np);
}

static void handle_full_state(netpeer_t *np) {
  N_UnpackFullState(np);
  N_PeerSyncSetHasGameState(np);
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

static void handle_client_auth_change(netpeer_t *np) {
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
  else if ((!sv_join_password) || (strcmp(password, sv_join_password) == 0)) {
    auth_level = AUTH_LEVEL_PLAYER;
  }
  else if ((!sv_spectate_password) ||
           (strcmp(password, sv_spectate_password) == 0)) {
    auth_level = AUTH_LEVEL_SPECTATOR;
  }

  if (auth_level > N_PeerGetAuthLevel(np)) {
    N_PeerSetAuthLevel(np, auth_level);
  }

  M_BufferFree(&buf);
}

static void handle_client_name_change(netpeer_t *np) {
  static buf_t *new_name = NULL;

  if (!new_name) {
    new_name = M_BufferNew();
  }

  if (!N_UnpackClientNameChange(np, new_name)) {
    return;
  }

  D_MsgLocalInfo("%s is now known as ", N_PeerGetName(np));
  N_PeerSetName(np, new_name->data);
  D_MsgLocalInfo("%s\n", N_PeerGetName(np));
}

static void handle_client_team_change(netpeer_t *np) {
  team_t *team = NULL;

  if (!N_UnpackTeamChange(np, &team)) {
    return;
  }

  if (team) {
    N_PeerSetTeam(team);
    D_MsgLocalInfo("%s has joined %s\n", N_PeerGetName(np), team->name);
  }
  else {
    team = N_PeerGetTeam(np);

    if (team) {
      D_MsgLocalInfo("%s has left %s", N_PeerGetName(np), team->name);
    }

    N_PeerSetTeam(NULL);
  }
}

static void handle_client_wsop_change(netpeer_t *np) {
  unsigned int wsop_flags = 0;

  if (!N_UnpackWSOPChange(np, &wsop_flags)) {
    return;
  }
}

static void handle_client_bobbing_change(netpeer_t *np) {
  double bobbing;

  if (!N_UnpackBobbingChange(np, &bobbing)) {
    return;
  }
}

static void handle_client_autoaim_change(netpeer_t *np) {
  bool autoaim;

  if (!N_UnpackAutoaimChange(np, &autoaim)) {
    return;
  }
}

static void handle_client_weapon_speed_change(netpeer_t *np) {
  unsigned char weapon_speed;

  if (!N_UnpackWeaponSpeedChange(np, &weapon_speed)) {
    return;
  }
}

static void handle_client_color_change(netpeer_t *np) {
  unsigned char red;
  unsigned char green;
  unsigned char blue;

  if (!N_UnpackColorChange(np, &red, &green, &blue)) {
    return;
  }
}

static void handle_client_attribute_change(netpeer_t *np) {
  int tic = 0;
  netpeer_t *src = NULL;
  client_attribute_e attribute_type;

  if (!N_UnpackClientAttributeChange(np, &attribute_type)) {
    return;
  }

  switch (attribute_type) {
    case CLIENT_ATTRIBUTE_AUTH:
      handle_client_auth_change(np);
    break;
    case CLIENT_ATTRIBUTE_NAME:
      handle_client_name_change(np);
    break;
    case CLIENT_ATTRIBUTE_TEAM:
      handle_client_team_change(np);
    break;
    case CLIENT_ATTRIBUTE_WSOP:
      handle_client_wsop_change(np);
    break;
    case CLIENT_ATTRIBUTE_PWO:
    case CLIENT_ATTRIBUTE_BOBBING:
    case CLIENT_ATTRIBUTE_AUTOAIM:
    case CLIENT_ATTRIBUTE_WEAPON_SPEED:
    case CLIENT_ATTRIBUTE_COLOR:
    case CLIENT_ATTRIBUTE_COLORMAP_INDEX:
    case CLIENT_ATTRIBUTE_SKIN:
      D_MsgLocalWarn("Unimplemented preference [%d].\n", attribute_type);
    break;
    default:
      D_MsgLocalWarn("Unsupported preference [%d].\n", attribute_type);
    break;
  }
}

static void handle_game_action_change(netpeer_t *np) {
  gameaction_t new_gameaction = 0;
  int new_gametic = 0;

  if (!N_UnpackGameActionChange(np, &new_gameaction, &new_gametic)) {
    return;
  }

  gametic = new_gametic;

  switch (new_gameaction) {
    case ga_loadlevel:
      puts("got ga_loadlevel");
      // force players to be initialized on level reload
      for (size_t i = 0; i < MAXPLAYERS; i++) {
        players[i].playerstate = PST_REBORN;
      }

      G_DoLoadLevel();
    break;
    case ga_completed:
      puts("got ga_completed");
      G_DoCompleted();
    break;
    case ga_worlddone:
      puts("got ga_worlddone");
      G_DoWorldDone();
    break;
    default:
    break;
  }
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
      case NM_MESSAGE:
        handle_chat_message(np);
      break;
      case NM_SYNC:
        handle_sync(np);
      break;
      case NM_CLIENT_ATTRIBUTE_CHANGE:
        SERVER_ONLY("client attribute change");
        handle_client_attribute_change(np);
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

    N_PeerClearUnreliableChannel(server);
    N_PackSync(server);
    N_PeerSyncSetNotOutdated(server);

    return;
  }

  NETPEER_FOR_EACH(iter) {
    if (N_PeerSyncNeedsGameInfo(iter.np) &&
        N_PeerSyncNeedsGameState(iter.np)) {
      N_PackSetup(iter.np);
      N_PeerSyncSetHasGameInfo(iter.np);
      N_PeerSyncSetHasGameState(iter.np);
      N_PeerUpdateLastSetupRequestTime(iter.np);
    }
    else if (N_PeerSyncNeedsGameState(iter.np)) {
      printf("(%d) Sending full state to %d\n",
        gametic, N_PeerGetPlayernum(iter.np)
      );
      N_PackFullState(iter.np);
      N_PeerSyncSetHasGameState(iter.np);
      N_PeerUpdateLastSetupRequestTime(iter.np);
    }
    else if (N_PeerSyncOutdated(iter.np) && N_PeerGetSyncTIC(iter.np) != 0) {
      N_PeerClearUnreliableChannel(iter.np);
      N_PeerBuildNewSyncStateDelta(iter.np);
      N_PackSync(iter.np);
      N_PeerSyncSetNotOutdated(iter.np);
    }
  }
}

void SV_SendAuthResponse(netpeer_t *np, auth_level_e auth_level) {
  N_PackAuthResponse(np, auth_level);
}

void SV_SendPing(netpeer_t *np) {
  N_PackPing(np, M_GetCurrentTime());
}

void SV_SendMessage(netpeer_t *np, const char *message) {
  if ((!message) || (!(*message))) {
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
  static GString *buf = NULL;

  va_list args;

  if (!buf) {
    buf = g_string_sized_new(256);
  }

  va_start(args, fmt);
  g_string_vprintf(buf, fmt, args);
  va_end(args);

  if (!buf->len) {
    return;
  }

  SV_BroadcastMessage(buf->str);
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
    unsigned int playernum = N_PeerGetPlayernum(iter.np);

    N_PeerResetSync(iter.np);
    N_PeerSyncSetHasGameInfo(iter.np);
    P_ResetPlayerCommands(playernum);
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

