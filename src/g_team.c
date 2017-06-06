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

team_t* G_TeamsAdd(const char *name, const char *message_name) {
  team_t *team = calloc(1, sizeof(team_t));

  if (!team) {
    I_Error("G_TeamsAdd: Error allocating memory for new team\n");
  }

  team->name = strdup(name);
  team->message_name = strdup(message_name);
  team->id = M_IDHashAdd(&teams, team);

  return team;
}

team_t* G_TeamsAddRaw(uint32_t id, const char *name,
                                   const char *message_name) {
  team_t *team = calloc(1, sizeof(team_t));

  if (!team) {
    I_Error("G_TeamsAddRaw: Error allocating memory for new team\n");
  }

  team->id = id;
  team->name = strdup(name);
  team->message_name = strdup(message_name);

  M_IDHashAssign(&teams, (void *)team, team->id);

  return team;
}

team_t* G_TeamsLookup(uint32_t id) {
  return M_IDHashLookup(&teams, id);
}

size_t G_TeamsGetCount(void) {
  return M_IDHashGetCount(&teams);
}

bool G_TeamsTeamExists(uint32_t id) {
  if (G_TeamsLookup(id)) {
    return true;
  }

  return false;
}

bool G_TeamsIterate(team_iterator_t *iter) {
  if (!M_IDHashIterate(&teams, &iter->iter)) {
    return false;
  }

  iter->team = (team_t *)iter->iter.obj;

  return true;
}

void G_TeamsIterateRemove(team_iterator_t *iter) {
  M_IDHashIterateRemove(&iter->iter);
}

void G_TeamRemove(team_t *team) {
  M_IDHashRemoveID(&teams, team->id);
}

/* vi: set et ts=2 sw=2: */
