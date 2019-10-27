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


#ifndef G_TEAM_H__
#define G_TEAM_H__

#include "m_idhash.h"

typedef struct team_s {
  uint32_t  id;
  char     *name;         /* ex: "Blue" */
  char     *message_name; /* ex: "the Blue Team" */
} team_t;

typedef struct {
  id_hash_iterator_t iter;
  team_t *team;
} team_iterator_t;

#define TEAMS_FOR_EACH(_it) \
  for (team_iterator_t _it = { { 0 }, NULL }; G_TeamsIterate(&_it);)

void    G_TeamsInit(void);
team_t* G_TeamsAdd(const char *name, const char *message_name);
team_t* G_TeamsAddRaw(uint32_t id, const char *name, const char *message_name);
team_t* G_TeamsLookup(uint32_t id);
size_t  G_TeamsGetCount(void);
bool    G_TeamsTeamExists(uint32_t id);
bool    G_TeamsIterate(team_iterator_t *iter);
void    G_TeamsIterateRemove(team_iterator_t *iter);
void    G_TeamRemove(team_t *team);

#endif

/* vi: set et ts=2 sw=2: */
