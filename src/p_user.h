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


#ifndef P_USER__
#define P_USER__

#include "d_player.h"

typedef struct player_message_s {
  char *content;
  int64_t tics;
  dboolean centered;
  dboolean processed;
  int sfx;
} player_message_t;

void P_PlayerThink(player_t *player);
void P_CalcHeight(player_t *player);
void P_DeathThink(player_t *player);
void P_MovePlayer(player_t *player);
void P_Thrust(player_t *player, angle_t angle, fixed_t move);
void P_SetPitch(player_t *player);
void P_RunPlayerCommands(player_t *player);
void P_SetName(int playernum, char *name);
void P_Printf(int playernum, const char *fmt, ...) PRINTF_DECL(2, 3);
void P_Echo(int playernum, const char *message);
void P_Write(int playernum, const char *message);
void P_CenterPrintf(int playernum, int sfx, const char *fmt, ...)
    PRINTF_DECL(3, 4);
void P_CenterEcho(int playernum, int sfx,const char *message);
void P_CenterWrite(int playernum, int sfx, const char *message);
void P_CenterQPrintf(int playernum, int sfx, const char *fmt, ...)
    PRINTF_DECL(3, 4);
void P_CenterQEcho(int playernum, int sfx,const char *message);
void P_CenterQWrite(int playernum, int sfx, const char *message);
void P_AddMessage(int playernum, player_message_t *message);
void P_SendMessage(const char *message);
void P_ClearMessages(int playernum);

#endif  /* P_USER__ */

/* vi: set et ts=2 sw=2: */

