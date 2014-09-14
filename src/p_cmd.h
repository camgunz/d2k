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


#ifndef P_CMD_H__
#define P_CMD_H__

void         P_InitLocalCommands(void);
void         P_InitPlayerCommands(player_t *player);
unsigned int P_GetLocalCommandCount(void);
unsigned int P_GetPlayerCommandCount(player_t *player);
void         P_UpdateConsoleplayerCommands(void);
void         P_ClearPlayerCommands(player_t *player);
void         P_ClearLocalCommands(void);
void         P_RemoveSyncedCommands(void);
void         P_PrintPlayerCommands(GArray *commands);
void         P_BuildCommand(void);
void         P_RunAllPlayerCommands(player_t *player);
void         P_RunBufferedCommands(player_t *player);
void         P_RunNextCommand(player_t *player);
void         P_PredictPlayerPosition(player_t *player);
void         P_RunPlayerCommand(player_t *player);
void         P_RunPlayerCommands(player_t *player);
bool         P_LoadCommandForTic(player_t *player, int tic);
bool         P_LoadLocalCommandForTic(int tic);
void         P_RemoveOldCommands(player_t *player, int tic);
GArray*      P_GetLocalCommands(void);
netticcmd_t *P_GetNewBlankCommand(player_t *player);
void         P_EnsurePlayerCommandsSize(player_t *player, unsigned int min_sz);

#endif

/* vi: set et ts=2 sw=2: */

