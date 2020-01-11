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

#include <time.h>

#include "m_file.h"
#include "m_idhash.h"
#include "d_msg.h"
#include "pl_main.h"

typedef struct message_handler_s {
  message_handler_f *handle;
  void *data;
} message_handler_t;

typedef struct message_channel_s {
  const char *name;
  bool enabled;
  msg_level_e level;
} message_channel_t;

/* [CG] [FIXME] There's no reason to use idhash here; indices will suffice. */

static id_hash_t message_channels;
static GArray *message_handlers = NULL;

uint32_t MSG_CHANNEL_SYS;

static void handle_new_message(message_t *msg) {
  message_channel_t *mc = M_IDHashLookup(&message_channels, msg->channel_id);

  if (!mc->enabled) {
    return;
  }

  if (msg->level < mc->level) {
    return;
  }

  for (size_t i = 0; i < message_handlers->len; i++) {
    message_handler_t *handler = &g_array_index(
      message_handlers,
      message_handler_t,
      i
    );

    handler->handle(msg, handler->data);
  }
}

static void free_message_channel(gpointer data) {
  message_channel_t *mc = data;

  free(mc);
}

void D_MessagingInit(void) {
  message_handlers = g_array_new(false, false, sizeof(message_handler_t));

  M_IDHashInit(&message_channels, free_message_channel);

  MSG_CHANNEL_SYS = D_MsgRegisterChannel("System message channel");
}

void D_MsgRegisterHandler(message_handler_f func, void *data) {
  message_handler_t handler;

  handler.handle = func;
  handler.data = data;

  g_array_append_val(message_handlers, handler);
}

void D_MsgUnregisterHandler(message_handler_f func) {
  for (size_t i = message_handlers->len; i > 0; i--) {
    message_handler_t *handler = &g_array_index(
      message_handlers, 
      message_handler_t,
      i - 1
    );

    if (handler->handle == func) {
      g_array_remove_index_fast(message_handlers, i - 1);
    }
  }
}

uint32_t D_MsgRegisterChannel(const char *name) {
  message_channel_t *mc = calloc(1, sizeof(message_channel_t));

  mc->name = name;
  mc->enabled = true;
#ifdef DEBUG
  mc->level = MSG_LEVEL_DEBUG;
#else
  mc->level = MSG_LEVEL_INFO;
#endif

  return M_IDHashAdd(&message_channels, (void *)mc);
}

void D_MsgUnregisterChannel(uint32_t channel_id) {
  M_IDHashRemoveID(&message_channels, channel_id);
}

void D_MsgChanSetEnabled(uint32_t channel_id, bool enabled) {
  message_channel_t *mc = M_IDHashLookup(&message_channels, channel_id);

  if (!mc) {
    D_MsgLocalError("D_MsgChanSetEnabled: Unknown channel ID %u\n", channel_id);
    return;
  }

  mc->enabled = enabled;
}

bool D_MsgChanEnabled(uint32_t channel_id) {
  message_channel_t *mc = M_IDHashLookup(&message_channels, channel_id);

  if (!mc) {
    I_Error("D_MsgChanSetEnabled: Unknown channel ID %u\n", channel_id);
    return false;
  }

  return mc->enabled;
}

void D_MsgChanSetLevel(uint32_t channel_id, msg_level_e level) {
  message_channel_t *mc = M_IDHashLookup(&message_channels, channel_id);

  if (!mc) {
    D_MsgLocalError("D_MsgChanSetLevel: Unknown channel ID %u\n",
      channel_id
    );
    return;
  }

  mc->level = level;
}

msg_level_e D_MsgChanGetLevel(uint32_t channel_id) {
  message_channel_t *mc = M_IDHashLookup(&message_channels, channel_id);

  if (!mc) {
    I_Error("D_MsgChanSetLevel: Unknown channel ID %u\n", channel_id);
  }

  return mc->level;
}

const char* D_MsgChanGetName(uint32_t channel_id) {
  message_channel_t *mc = M_IDHashLookup(&message_channels, channel_id);

  if (!mc) {
    I_Error("D_MsgChanSetEnabled: Unknown channel ID %u\n", channel_id);
  }

  return mc->name;
}

void D_MsgChanMsg(uint32_t channel_id, msg_level_e level,
                                       msg_recipient_e recipient,
                                       msg_visibility_e visibility,
                                       uint32_t recipient_id,
                                       bool is_markup,
                                       int sfx,
                                       const char *fmt,
                                       ...) {
  va_list args;

  va_start(args, fmt);
  D_MsgChanVMsg(
    channel_id,
    level,
    recipient,
    visibility,
    recipient_id,
    is_markup,
    sfx,
    fmt,
    args
  );
  va_end(args);
}

void D_MsgChanVMsg(uint32_t channel_id, msg_level_e level,
                                        msg_recipient_e recipient,
                                        msg_visibility_e visibility,
                                        uint32_t recipient_id,
                                        bool is_markup,
                                        int sfx,
                                        const char *fmt,
                                        va_list args) {
  message_t msg;

  if (recipient == MSG_RECIPIENT_PEER) {
    if (!MULTINET) {
      D_MsgLocalWarn(
        "D_MsgChanVMsg: Cannot send messages to peers outside of netgames\n"
      );
      return;
    }

    if (recipient_id == 0) {
      D_MsgLocalWarn("D_MsgChanVMsg: Invalid peer ID 0\n");
      return;
    }
  }

  if (recipient == MSG_RECIPIENT_PLAYER) {
    if (recipient_id == 0) {
      D_MsgLocalWarn("D_MsgChanVMsg: Invalid peer ID 0\n");
    }

    if (!P_PlayersLookup(recipient_id)) {
      D_MsgLocalWarn(
        "Cannot send message to non-existent player %u\n",
        recipient_id
      );
      return;
    }
  }

  msg.channel_id = channel_id;
  msg.level = level;
  msg.recipient = recipient;
  msg.visibility = visibility;
  msg.recipient_id = recipient_id;
  msg.is_markup = is_markup;
  msg.sfx = sfx;
  msg.contents = g_strdup_vprintf(fmt, args);
  msg.processed = false;

  handle_new_message(&msg);
}

void D_MsgLocalDebug(const char *fmt, ...) {
  va_list args;

  va_start(args, fmt);
  D_VMsgLocalDebug(fmt, args);
  va_end(args);
}

void D_VMsgLocalDebug(const char *fmt, va_list args) {
  D_MsgChanVMsg(
    MSG_CHANNEL_SYS,
    MSG_LEVEL_DEBUG,
    MSG_RECIPIENT_LOCAL,
    MSG_VISIBILITY_CONSOLE_ONLY,
    0,
    true,
    -1,
    fmt,
    args
  );
}

void D_MsgLocalInfo(const char *fmt, ...) {
  va_list args;

  va_start(args, fmt);
  D_VMsgLocalInfo(fmt, args);
  va_end(args);
}

void D_VMsgLocalInfo(const char *fmt, va_list args) {
  D_MsgChanVMsg(
    MSG_CHANNEL_SYS,
    MSG_LEVEL_INFO,
    MSG_RECIPIENT_LOCAL,
    MSG_VISIBILITY_NORMAL,
    0,
    true,
    -1,
    fmt,
    args
  );
}

void D_MsgLocalWarn(const char *fmt, ...) {
  va_list args;

  va_start(args, fmt);
  D_VMsgLocalWarn(fmt, args);
  va_end(args);
}

void D_VMsgLocalWarn(const char *fmt, va_list args) {
  D_MsgChanVMsg(
    MSG_CHANNEL_SYS,
    MSG_LEVEL_WARN,
    MSG_RECIPIENT_LOCAL,
    MSG_VISIBILITY_NORMAL,
    0,
    true,
    -1,
    fmt,
    args
  );
}

void D_MsgLocalError(const char *fmt, ...) {
  va_list args;

  va_start(args, fmt);
  D_VMsgLocalError(fmt, args);
  va_end(args);
}

void D_VMsgLocalError(const char *fmt, va_list args) {
  D_MsgChanVMsg(
    MSG_CHANNEL_SYS,
    MSG_LEVEL_ERROR,
    MSG_RECIPIENT_LOCAL,
    MSG_VISIBILITY_NORMAL,
    0,
    true,
    -1,
    fmt,
    args
  );
}

/* vi: set et ts=2 sw=2: */
