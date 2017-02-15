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


#ifndef CL_NET_H__
#define CL_NET_H__

netpeer_t* CL_GetServerPeer(void);
void       CL_CheckForStateUpdates(void);
void       CL_MarkServerOutdated(void);
void       CL_UpdateReceivedCommandIndex(unsigned int command_index);
int        CL_StateTIC(void);
bool       CL_ReceivedSetup(void);
void       CL_SetAuthorizationLevel(auth_level_e level);
void       CL_SetNewGameState(gamestate_t new_gamestate);
void       CL_RePredict(int saved_gametic);
bool       CL_OccurredDuringRePrediction(int tic);
bool       CL_LoadingState(void);
bool       CL_Synchronizing(void);
bool       CL_RePredicting(void);
void       CL_SetNeedsInitNew(void);
void       CL_ResetSync(void);

#endif

/* vi: set et ts=2 sw=2: */
