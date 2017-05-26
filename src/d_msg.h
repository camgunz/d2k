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


#ifndef D_MSG_H__
#define D_MSG_H__

typedef enum {
  MSG_LEVEL_DEBUG,
  MSG_LEVEL_INFO,
  MSG_LEVEL_WARN,
  MSG_LEVEL_ERROR,
} msg_level_e;

typedef enum {
  MSG_RECIPIENT_EVERYONE,
  MSG_RECIPIENT_LOCAL,
  MSG_RECIPIENT_PLAYER,
  MSG_RECIPIENT_PEER
} msg_recipient_e;

#define MSG_SINK_NONE     (0)
#define MSG_SINK_CONSOLE  (1 << 0)
#define MSG_SINK_MESSAGES (1 << 1)
#define MSG_SINK_HIGH     (1 << 2)

typedef struct message_s {
  uint32_t channel_id;
  msg_level_e level;
  msg_recipient_e recipient;
  unsigned int sinks;
  uint32_t recipient_id;
  bool is_markup;
  int sfx;
  char *contents;
  bool processed;
} message_t;

extern uint32_t MSG_CHANNEL_SYS;

typedef void (message_handler_f)(message_t *message, void *data);

void D_MsgInit(void);

void D_MsgRegisterHandler(message_handler_f func, void *data);
void D_MsgUnregisterHandler(message_handler_f func);

uint32_t D_MsgRegisterChannel(const char *name);
void     D_MsgUnregisterChannel(uint32_t channel_id);

void        D_MsgChanSetEnabled(uint32_t channel_id, bool enabled);
bool        D_MsgChanEnabled(uint32_t channel_id);
void        D_MsgChanSetLevel(uint32_t channel_id, msg_level_e level);
msg_level_e D_MsgChanGetLevel(uint32_t channel_id);
const char* D_MsgChanGetName(uint32_t channel_id);

void D_MsgChanVMsg(uint32_t channel_id, msg_level_e level,
                                        msg_recipient_e recipient,
                                        unsigned int sinks,
                                        uint32_t recipient_id,
                                        bool is_markup,
                                        int sfx,
                                        const char *fmt,
                                        va_list args);
void D_MsgChanMsg(uint32_t channel_id, msg_level_e level,
                                       msg_recipient_e recipient,
                                       unsigned int sinks,
                                       uint32_t recipient_id,
                                       bool is_markup,
                                       int sfx,
                                       const char *fmt,
                                       ...) PRINTF_DECL(8, 9);

void D_MsgLocalDebug(const char *fmt, ...);
void D_MsgLocalInfo(const char *fmt, ...);
void D_MsgLocalWarn(const char *fmt, ...);
void D_MsgLocalError(const char *fmt, ...);

#endif

/* vi: set et ts=2 sw=2: */
