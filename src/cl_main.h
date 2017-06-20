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

struct player_s;
typedef struct player_s player_t;

struct net_peer_s;
typedef struct net_peer_s net_peer_t;

extern int cl_extrapolate_player_positions;

void        CL_Init(void);
void        CL_Reset(void);
void        CL_Connect(const char *address);
void        CL_SetConnected(void);
void        CL_Disconnect(void);
void        CL_TrimSynchronizedCommands(void);
size_t      CL_GetUnsynchronizedCommandCount(player_t *player);
bool        CL_RunningConsoleplayerCommands(void);
bool        CL_RunningNonConsoleplayerCommands(void);
bool        CL_Predicting(void);
void        CL_SetRunningThinkers(bool running);
bool        CL_RunningThinkers(void);
void        CL_SetupCommandState(player_t *player, uint32_t command_index);
void        CL_ShutdownCommandState(void);
uint32_t    CL_GetCurrentCommandIndex(void);
uint32_t    CL_GetNextCommandIndex(void);
net_peer_t* CL_GetServerPeer(void);
void        CL_CheckForStateUpdates(void);
void        CL_MarkServerOutdated(void);
bool        CL_OccurredDuringRePrediction(int tic);
void        CL_UpdateReceivedCommandIndex(uint32_t command_index);
int         CL_GetStateTIC(void);
bool        CL_ReceivedSetup(void);
void        CL_SetAuthorizationLevel(auth_level_e level);
void        CL_SetNewGameState(gamestate_e new_gamestate);
void        CL_RePredict(int saved_gametic);
bool        CL_LoadingState(void);
bool        CL_Synchronizing(void);
bool        CL_RePredicting(void);
void        CL_SetNeedsInitNew(void);
void        CL_ResetSync(void);

#endif

/* vi: set et ts=2 sw=2: */

