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

void         P_InitCommands(void);
netticcmd_t* P_GetNewBlankCommand(void);
void         P_BuildCommand(void);
void         P_RunPlayerCommands(int playernum);
void         P_ClearPlayerCommands(int playernum);
void         P_RemoveOldCommands(int playernum, int command_index);
void         P_RemoveOldCommandsByTic(int playernum, int tic);
void         P_RecycleCommand(netticcmd_t *ncmd);
int          P_GetCurrentCommandIndex(void);
void         P_SetCurrentCommandIndex(int new_current_command_index);
void         P_PrintCommands(GQueue *commands);

#endif

/* vi: set et ts=2 sw=2: */

