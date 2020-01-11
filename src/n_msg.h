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


#ifndef N_MSG_H__
#define N_MSG_H__

extern uint32_t MSG_CHANNEL_CMD;
extern uint32_t MSG_CHANNEL_NET;
extern uint32_t MSG_CHANNEL_SYNC;
extern uint32_t MSG_CHANNEL_SAVE;

void N_MsgInit(void);

void N_MsgCmdLocalDebug(const char *fmt, ...) PRINTF_DECL(1, 2);
void N_MsgNetLocalDebug(const char *fmt, ...) PRINTF_DECL(1, 2);
void N_MsgSyncLocalDebug(const char *fmt, ...) PRINTF_DECL(1, 2);
void N_MsgSaveLocalDebug(const char *fmt, ...) PRINTF_DECL(1, 2);

void N_MsgCmdLocalInfo(const char *fmt, ...) PRINTF_DECL(1, 2);
void N_MsgNetLocalInfo(const char *fmt, ...) PRINTF_DECL(1, 2);
void N_MsgSyncLocalInfo(const char *fmt, ...) PRINTF_DECL(1, 2);
void N_MsgSaveLocalInfo(const char *fmt, ...) PRINTF_DECL(1, 2);

void N_MsgCmdLocalWarn(const char *fmt, ...) PRINTF_DECL(1, 2);
void N_MsgNetLocalWarn(const char *fmt, ...) PRINTF_DECL(1, 2);
void N_MsgSyncLocalWarn(const char *fmt, ...) PRINTF_DECL(1, 2);
void N_MsgSaveLocalWarn(const char *fmt, ...) PRINTF_DECL(1, 2);

void N_MsgCmdLocalError(const char *fmt, ...) PRINTF_DECL(1, 2);
void N_MsgNetLocalError(const char *fmt, ...) PRINTF_DECL(1, 2);
void N_MsgSyncLocalError(const char *fmt, ...) PRINTF_DECL(1, 2);
void N_MsgSaveLocalError(const char *fmt, ...) PRINTF_DECL(1, 2);

#endif

/* vi: set et ts=2 sw=2: */
