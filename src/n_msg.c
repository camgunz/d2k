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
#include "n_msg.h"

#define DEBUG_NET 1
#define DEBUG_SYNC 1
#define DEBUG_SAVE 0
#define DEBUG_CMD 0

uint32_t MSG_CHANNEL_NET;
uint32_t MSG_CHANNEL_SYNC;
uint32_t MSG_CHANNEL_SAVE;
uint32_t MSG_CHANNEL_CMD;
uint32_t MSG_CHANNEL_STATE;

void N_MsgInit(void) {
  MSG_CHANNEL_NET =   D_MsgRegisterChannel("Network messages");
  MSG_CHANNEL_SYNC =  D_MsgRegisterChannel("Sync messages");
  MSG_CHANNEL_SAVE =  D_MsgRegisterChannel("Save messages");
  MSG_CHANNEL_CMD =   D_MsgRegisterChannel("Player command messages");
  MSG_CHANNEL_STATE = D_MsgRegisterChannel("Game states");

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
}

void N_MsgCmdLocalDebug(const char *fmt, ...) {
  va_list args;

  va_start(args, fmt);
  D_MsgChanVMsg(
    MSG_CHANNEL_CMD,
    MSG_LEVEL_DEBUG,
    MSG_RECIPIENT_LOCAL,
    MSG_SINK_NONE,
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
    MSG_SINK_NONE,
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
    MSG_SINK_NONE,
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
    MSG_SINK_NONE,
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
    MSG_SINK_NONE,
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
    MSG_SINK_NONE,
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
    MSG_SINK_NONE,
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
    MSG_SINK_NONE,
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
    MSG_SINK_NONE,
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
    MSG_SINK_NONE,
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
    MSG_SINK_NONE,
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
    MSG_SINK_NONE,
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
    MSG_SINK_NONE,
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
    MSG_SINK_NONE,
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
    MSG_SINK_NONE,
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
    MSG_SINK_NONE,
    0,
    false,
    -1,
    fmt,
    args
  );
  va_end(args);
}

