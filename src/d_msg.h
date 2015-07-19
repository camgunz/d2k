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

#define MSG_MIN MSG_DEBUG

typedef enum {
  MSG_DEBUG,
  MSG_INFO,
  MSG_WARN,
  MSG_ERROR,
  MSG_DEH,
  MSG_GAME,
  MSG_NET,
  MSG_SYNC,
  MSG_STATE,
  MSG_MEM,
  MSG_SAVE,
  MSG_SOUND,
  MSG_MAX
} msg_channel_e;

void D_InitMessaging(void);
bool D_MsgActive(msg_channel_e chan);
void D_MsgActivate(msg_channel_e chan);
bool D_MsgActivateWithFile(msg_channel_e chan, const char *file_path);
void D_MsgDeactivate(msg_channel_e chan);
void D_VMsg(msg_channel_e chan, const char *fmt, va_list args);
void D_Msg(msg_channel_e chan, const char *fmt, ...) PRINTF_DECL(2, 3);
bool D_LogToFile(msg_channel_e chan, const char *file_path);

#endif

/* vi: set et ts=2 sw=2: */

