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


#include "z_zone.h"

#include "doomdef.h"
#include "doomstat.h"
#include "m_idlist.h"
#include "p_user.h"
#include "p_player.h"

static id_list_t new_players;
static uint32_t console_player_id;
static uint32_t display_player_id;

static void free_player(gpointer data) {
  free(data);
}

void P_PlayersInit(void) {
  M_IDListInit(&new_players, free_player);
}

uint32_t P_PlayersGetCount(void) {
  return M_IDListGetSize(&new_players);
}

bool P_PlayersIter(size_t *index, player_t **start) {
  return M_IDListIterate(&new_players, index, (void **)start);
}

player_t* P_PlayersGetNew(void) {
  player_t *new_player = malloc(sizeof(player_t));

  new_player->id = M_IDListGetNewID(&new_players, new_player);

  return new_player;
}

player_t* P_PlayersLookup(uint32_t id) {
  return M_IDListLookupObj(&new_players, id);
}

void P_PlayerRemove(player_t *player) {
  M_IDListReleaseID(&new_players, player->id);
}

void P_PlayersSetConsolePlayerID(uint32_t new_console_player_id) {
  console_player_id = new_console_player_id;
}

void P_PlayersSetDisplayPlayerID(uint32_t new_display_player_id) {
  display_player_id = new_display_player_id;
}

player_t* P_PlayersGetConsolePlayer(void) {
  return P_PlayersLookup(console_player_id);
}

player_t* P_PlayersGetDisplayPlayer(void) {
  return P_PlayersLookup(display_player_id);
}

bool PL_IsConsolePlayer(player_t *player) {
  return player->id == console_player_id;
}

bool PL_IsDisplayPlayer(player_t *player) {
  return player->id == display_player_id;
}

/* vi: set et ts=2 sw=2: */
