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


#ifndef CL_MAIN_H__
#define CL_MAIN_H__

extern int cl_extrapolate_player_positions;

bool CL_Predicting(void);
bool CL_RunningConsoleplayerCommands(void);
bool CL_RunningNonConsoleplayerCommands(void);
void CL_SetRunningThinkers(bool running);
bool CL_RunningThinkers(void);
void CL_SetupCommandState(int playernum, unsigned int command_index);
void CL_ShutdownCommandState(void);
int  CL_GetCurrentCommandIndex(void);
int  CL_GetNextCommandIndex(void);
bool CL_ReceivedSetup(void);
void CL_Init(void);
void CL_Reset(void);

#endif

/* vi: set et ts=2 sw=2: */

