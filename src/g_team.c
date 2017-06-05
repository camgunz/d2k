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

#include "m_idhash.h"
#include "g_team.h"

static id_hash_t teams;

static void free_team(team_t *team) {
  if (team->name) {
    free(team->name);
  }

  if (team->message_name) {
    free(team->message_name);
  }

  free(team);
}

static void team_destroy_func(gpointer data) {
  free_team((team_t *)data);
}

void G_TeamsInit(void) {
  M_IDHashInit(&teams, team_destroy_func);
}

/* vi: set et ts=2 sw=2: */
