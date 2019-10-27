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

#include "d_msg.h"
#include "g_team.h"
#include "pl_main.h"
#include "n_main.h"
#include "n_msg.h"
#include "n_peer.h"
#include "n_proto.h"

#define DEBUG_NET 1
#define DEBUG_SYNC 1
#define DEBUG_SAVE 0
#define DEBUG_CMD 0

uint32_t MSG_CHANNEL_NET;
uint32_t MSG_CHANNEL_SYNC;
uint32_t MSG_CHANNEL_SAVE;
uint32_t MSG_CHANNEL_CMD;
uint32_t MSG_CHANNEL_STATE;
uint32_t MSG_CHANNEL_CHAT_INCOMING;
uint32_t MSG_CHANNEL_CHAT_OUTGOING;

static void sv_handle_incoming_chat_message(message_t *message, void *data) {
  /*
   * This is for chat messages received over the network.  If they're not meant
   * specifically for the server, the server needs to forward them to their
   * destinations.
   */
  if (message->channel_id != MSG_CHANNEL_CHAT_INCOMING) {
    return;
  }

  switch (message->recipient) {
    case MSG_RECIPIENT_EVERYONE:
      break;
    case MSG_RECIPIENT_TEAM:
      break;
    case MSG_RECIPIENT_PLAYER:
      break;
    case MSG_RECIPIENT_PEER:
      break;
    default:
      break;
  }
}

static void sv_handle_chat_message(message_t *message, void *data) {
  netpeer_t *np = NULL;
  player_t *player = NULL;

  if (!((message->channel_id == MSG_CHANNEL_CHAT_OUTGOING) ||
        (message->channel_id == MSG_CHANNEL_CHAT_INCOMING))) {
    return;
  }

  switch (message->recipient) {
    case MSG_RECIPIENT_EVERYONE:
      NETPEER_FOR_EACH(iter) {
        SV_SendMessage(np, message);
      }
      break;
    case MSG_RECIPIENT_TEAM:
      NETPEER_FOR_EACH(iter) {
        team_t *team = N_PeerGetTeam(iter.np);

        if ((!team) || (team->id != message->recipient_id)) {
          continue;
        }

        SV_SendMessage(iter.np, message);
      }
      break;
    case MSG_RECIPIENT_PLAYER:
      player = P_PlayersLookup(message->recipient_id);

      if (!player) {
        D_MsgLocalWarn(
          "sv_handle_outgoing_chat_message: No player with ID %u\n",
          message->recipient_id
        );
        return;
      }

      np = N_PeersLookupByPlayer(player);

      if (!np) {
        D_MsgLocalWarn(
          "sv_handle_outgoing_chat_message: No peer for player %u\n",
          player->id
        );
        return;
      }

      SV_SendMessage(np, message);
      break;
    case MSG_RECIPIENT_PEER:
      message->recipient = MSG_RECIPIENT_LOCAL;
      np = N_PeersLookup(message->recipient_id);

      if (!np) {
        D_MsgLocalWarn(
          "sv_handle_outgoing_chat_message: No peer with ID %u\n",
          message->recipient_id
        );
        return;
      }

      SV_SendMessage(np, message);
      break;
    default:
      break;
  }
}

static void cl_handle_outgoing_chat_message(message_t *message, void *data) {
  if (message->recipient == MSG_RECIPIENT_LOCAL) {
    return;
  }

  CL_SendMessage(message);
}

static void server_message_handler(message_t *message, void *data) {
  if (message->channel_id == MSG_CHANNEL_CHAT_INCOMING) {
    sv_handle_incoming_chat_message(message, data);
  }
  else if (message->channel_id == MSG_CHANNEL_CHAT_OUTGOING) {
    sv_handle_outgoing_chat_message(message, data);
  }
}

static void client_message_handler(message_t *message, void *data) {
  if (message->channel_id != MSG_CHANNEL_CHAT_OUTGOING) {
    return;
  }
  cl_handle_outgoing_chat_message(message, data);
}

void N_MsgInit(void) {
  MSG_CHANNEL_NET           = D_MsgRegisterChannel("Network messages");
  MSG_CHANNEL_SYNC          = D_MsgRegisterChannel("Sync messages");
  MSG_CHANNEL_SAVE          = D_MsgRegisterChannel("Save messages");
  MSG_CHANNEL_CMD           = D_MsgRegisterChannel("Player command messages");
  MSG_CHANNEL_STATE         = D_MsgRegisterChannel("Game state messages");
  MSG_CHANNEL_CHAT_INCOMING = D_MsgRegisterChannel("Chat messages (incoming)");
  MSG_CHANNEL_CHAT_OUTGOING = D_MsgRegisterChannel("Chat messages (outgoing)");

#ifdef DEBUG

#if DEBUG_NET == 0
  D_MsgChanSetLevel(MSG_CHANNEL_NET, MSG_LEVEL_INFO);
#endif

#if DEBUG_SYNC == 0
  D_MsgChanSetLevel(MSG_CHANNEL_SYNC, MSG_LEVEL_INFO);
#endif

#if DEBUG_SAVE == 0
  D_MsgChanSetLevel(MSG_CHANNEL_SAVE, MSG_LEVEL_INFO);
#endif

#if DEBUG_CMD == 0
  D_MsgChanSetLevel(MSG_CHANNEL_CMD, MSG_LEVEL_INFO);
#endif

#endif

  if (SERVER) {
    D_MsgRegisterHandler(server_message_handler, NULL);
  }
  else if (CLIENT) {
    D_MsgRegisterHandler(client_message_handler, NULL);
  }
}

void N_MsgCmdLocalDebug(const char *fmt, ...) {
  va_list args;

  va_start(args, fmt);
  D_MsgChanVMsg(
    MSG_CHANNEL_CMD,
    MSG_LEVEL_DEBUG,
    MSG_RECIPIENT_LOCAL,
    MSG_VISIBILITY_CONSOLE_ONLY,
    0,
    false,
    -1,
    fmt,
    args
  );
  va_end(args);
}

void N_MsgNetLocalDebug(const char *fmt, ...) {
  va_list args;

  va_start(args, fmt);
  D_MsgChanVMsg(
    MSG_CHANNEL_NET,
    MSG_LEVEL_DEBUG,
    MSG_RECIPIENT_LOCAL,
    MSG_VISIBILITY_CONSOLE_ONLY,
    0,
    false,
    -1,
    fmt,
    args
  );
  va_end(args);
}

void N_MsgSyncLocalDebug(const char *fmt, ...) {
  va_list args;

  va_start(args, fmt);
  D_MsgChanVMsg(
    MSG_CHANNEL_SYNC,
    MSG_LEVEL_DEBUG,
    MSG_RECIPIENT_LOCAL,
    MSG_VISIBILITY_CONSOLE_ONLY,
    0,
    false,
    -1,
    fmt,
    args
  );
  va_end(args);
}

void N_MsgSaveLocalDebug(const char *fmt, ...) {
  va_list args;

  va_start(args, fmt);
  D_MsgChanVMsg(
    MSG_CHANNEL_SAVE,
    MSG_LEVEL_DEBUG,
    MSG_RECIPIENT_LOCAL,
    MSG_VISIBILITY_CONSOLE_ONLY,
    0,
    false,
    -1,
    fmt,
    args
  );
  va_end(args);
}

void N_MsgCmdLocalInfo(const char *fmt, ...) {
  va_list args;

  va_start(args, fmt);
  D_MsgChanVMsg(
    MSG_CHANNEL_CMD,
    MSG_LEVEL_INFO,
    MSG_RECIPIENT_LOCAL,
    MSG_VISIBILITY_NORMAL,
    0,
    false,
    -1,
    fmt,
    args
  );
  va_end(args);
}

void N_MsgNetLocalInfo(const char *fmt, ...) {
  va_list args;

  va_start(args, fmt);
  D_MsgChanVMsg(
    MSG_CHANNEL_NET,
    MSG_LEVEL_INFO,
    MSG_RECIPIENT_LOCAL,
    MSG_VISIBILITY_NORMAL,
    0,
    false,
    -1,
    fmt,
    args
  );
  va_end(args);
}

void N_MsgSyncLocalInfo(const char *fmt, ...) {
  va_list args;

  va_start(args, fmt);
  D_MsgChanVMsg(
    MSG_CHANNEL_SYNC,
    MSG_LEVEL_INFO,
    MSG_RECIPIENT_LOCAL,
    MSG_VISIBILITY_NORMAL,
    0,
    false,
    -1,
    fmt,
    args
  );
  va_end(args);
}

void N_MsgSaveLocalInfo(const char *fmt, ...) {
  va_list args;

  va_start(args, fmt);
  D_MsgChanVMsg(
    MSG_CHANNEL_SAVE,
    MSG_LEVEL_INFO,
    MSG_RECIPIENT_LOCAL,
    MSG_VISIBILITY_NORMAL,
    0,
    false,
    -1,
    fmt,
    args
  );
  va_end(args);
}

void N_MsgCmdLocalWarn(const char *fmt, ...) {
  va_list args;

  va_start(args, fmt);
  D_MsgChanVMsg(
    MSG_CHANNEL_CMD,
    MSG_LEVEL_WARN,
    MSG_RECIPIENT_LOCAL,
    MSG_VISIBILITY_NORMAL,
    0,
    false,
    -1,
    fmt,
    args
  );
  va_end(args);
}

void N_MsgNetLocalWarn(const char *fmt, ...) {
  va_list args;

  va_start(args, fmt);
  D_MsgChanVMsg(
    MSG_CHANNEL_NET,
    MSG_LEVEL_WARN,
    MSG_RECIPIENT_LOCAL,
    MSG_VISIBILITY_NORMAL,
    0,
    false,
    -1,
    fmt,
    args
  );
  va_end(args);
}

void N_MsgSyncLocalWarn(const char *fmt, ...) {
  va_list args;

  va_start(args, fmt);
  D_MsgChanVMsg(
    MSG_CHANNEL_SYNC,
    MSG_LEVEL_WARN,
    MSG_RECIPIENT_LOCAL,
    MSG_VISIBILITY_NORMAL,
    0,
    false,
    -1,
    fmt,
    args
  );
  va_end(args);
}

void N_MsgSaveLocalWarn(const char *fmt, ...) {
  va_list args;

  va_start(args, fmt);
  D_MsgChanVMsg(
    MSG_CHANNEL_SAVE,
    MSG_LEVEL_WARN,
    MSG_RECIPIENT_LOCAL,
    MSG_VISIBILITY_NORMAL,
    0,
    false,
    -1,
    fmt,
    args
  );
  va_end(args);
}

void N_MsgCmdLocalError(const char *fmt, ...) {
  va_list args;

  va_start(args, fmt);
  D_MsgChanVMsg(
    MSG_CHANNEL_CMD,
    MSG_LEVEL_ERROR,
    MSG_RECIPIENT_LOCAL,
    MSG_VISIBILITY_NORMAL,
    0,
    false,
    -1,
    fmt,
    args
  );
  va_end(args);
}

void N_MsgNetLocalError(const char *fmt, ...) {
  va_list args;

  va_start(args, fmt);
  D_MsgChanVMsg(
    MSG_CHANNEL_NET,
    MSG_LEVEL_ERROR,
    MSG_RECIPIENT_LOCAL,
    MSG_VISIBILITY_NORMAL,
    0,
    false,
    -1,
    fmt,
    args
  );
  va_end(args);
}

void N_MsgSyncLocalError(const char *fmt, ...) {
  va_list args;

  va_start(args, fmt);
  D_MsgChanVMsg(
    MSG_CHANNEL_SYNC,
    MSG_LEVEL_ERROR,
    MSG_RECIPIENT_LOCAL,
    MSG_VISIBILITY_NORMAL,
    0,
    false,
    -1,
    fmt,
    args
  );
  va_end(args);
}

void N_MsgSaveLocalError(const char *fmt, ...) {
  va_list args;

  va_start(args, fmt);
  D_MsgChanVMsg(
    MSG_CHANNEL_SAVE,
    MSG_LEVEL_ERROR,
    MSG_RECIPIENT_LOCAL,
    MSG_VISIBILITY_NORMAL,
    0,
    false,
    -1,
    fmt,
    args
  );
  va_end(args);
}

/* vi: set et ts=2 sw=2: */
