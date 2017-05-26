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


#ifndef PL_MSG_H__
#define PL_MSG_H__

struct player_message_s;
typedef struct player_message_s player_message_t;

struct player_s;
typedef struct player_s player_t;

player_t* P_GetConsolePlayer(void);
player_t* P_GetDisplayPlayer(void);

void PL_InitMessages(player_t *player);
void PL_AddMessage(player_t *player, player_message_t *message);
void PL_ClearMessagesUpdated(player_t *player);

void PL_Printf(player_t *player, const char *fmt, ...) PRINTF_DECL(2, 3);
void PL_VPrintf(player_t *player, const char *fmt, va_list args);
void PL_Echo(player_t *player, const char *message);
void PL_Write(player_t *player, const char *message);
void PL_MPrintf(player_t *player, const char *fmt, ...) PRINTF_DECL(2, 3);
void PL_MVPrintf(player_t *player, const char *fmt, va_list args);
void PL_MEcho(player_t *player, const char *message);
void PL_MWrite(player_t *player, const char *message);
void PL_SPrintf(player_t *player, int sfx, const char *fmt, ...) PRINTF_DECL(3, 4);
void PL_SVPrintf(player_t *player, int sfx, const char *fmt, va_list args);
void PL_SEcho(player_t *player, int sfx, const char *message);
void PL_SWrite(player_t *player, int sfx, const char *message);
void PL_MSPrintf(player_t *player, int sfx, const char *fmt, ...) PRINTF_DECL(3, 4);
void PL_MSVPrintf(player_t *player, int sfx, const char *fmt, va_list args);
void PL_MSEcho(player_t *player, int sfx, const char *message);
void PL_MSWrite(player_t *player, int sfx, const char *message);
void PL_CenterPrintf(player_t *player, const char *fmt, ...) PRINTF_DECL(2, 3);
void PL_CenterVPrintf(player_t *player, const char *fmt, va_list args);
void PL_CenterEcho(player_t *player, const char *message);
void PL_CenterWrite(player_t *player, const char *message);
void PL_CenterMPrintf(player_t *player, const char *fmt, ...) PRINTF_DECL(2, 3);
void PL_CenterMVPrintf(player_t *player, const char *fmt, va_list args);
void PL_CenterMEcho(player_t *player, const char *message);
void PL_CenterMWrite(player_t *player, const char *message);
void PL_CenterSPrintf(player_t *player, int sfx, const char *fmt, ...) PRINTF_DECL(3, 4);
void PL_CenterSVPrintf(player_t *player, int sfx, const char *fmt, va_list args);
void PL_CenterSEcho(player_t *player, int sfx, const char *message);
void PL_CenterSWrite(player_t *player, int sfx, const char *message);
void PL_CenterMSPrintf(player_t *player, int sfx, const char *fmt, ...) PRINTF_DECL(3, 4);
void PL_CenterMSVPrintf(player_t *player, int sfx, const char *fmt, va_list args);
void PL_CenterMSEcho(player_t *player, int sfx, const char *message);
void PL_CenterMSWrite(player_t *player, int sfx, const char *message);
void PL_DestroyMessage(gpointer data);


#endif

/* vi: set et ts=2 sw=2: */
