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


#ifndef P_PLAYER_H__
#define P_PLAYER_H__

void      P_PlayersInit(void);
uint32_t  P_PlayersGetCount(void);
bool      P_PlayersIter(size_t *index, player_t **start);
player_t* P_PlayersGetNew(void);
player_t* P_PlayersLookup(uint32_t id);
void      P_PlayerRemove(player_t *player);
bool      P_PlayerIsConsoleplayer(player_t *player);
bool      P_PlayerIsDisplayplayer(player_t *player);
void      P_PlayerSetConsoleplayerID(uint32_t new_consoleplayer_id);
void      P_PlayerSetDisplayplayerID(uint32_t new_displayplayer_id);

#endif

/* vi: set et ts=2 sw=2: */
