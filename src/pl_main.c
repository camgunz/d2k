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

#include "m_idlist.h"
#include "p_user.h"
#include "p_player.h"

static id_list_t new_players;
static player_t *console_player = NULL;
static player_t *display_player = NULL;

static void free_player(gpointer data) {
  player_t *player = (player_t *)data;

  g_hash_table_destroy(player->frags);
  g_hash_table_destroy(player->deaths);
  free(player);
}

void P_PlayersInit(void) {
  M_IDListInit(&new_players, free_player);
}

uint32_t P_PlayersGetCount(void) {
  return M_IDListGetSize(&new_players);
}

bool P_PlayersIterate(players_iter_t *iter) {
  return M_IDListIterate(
    &new_players,
    &iter->node,
    &iter->player,
    iter->wraparound
  );
}

player_t* P_PlayersGetNew(void) {
  player_t *player = PL_New();

  player->id = M_IDListAdd(&new_players, player);

  return player;
}

player_t* P_PlayersGetNewWithID(uint32_t id) {
  player_t *player = PL_New();

  player->id = M_IDListAssign(&new_players, player, id);

  return player;
}

player_t* P_PlayersLookup(uint32_t id) {
  return M_IDListLookupObj(&new_players, id);
}

void P_PlayerRemove(player_t *player) {
  M_IDListReleaseID(&new_players, player->id);
}

player_t* P_GetConsolePlayer(void) {
  return console_player;
}

player_t* P_GetDisplayPlayer(void) {
  return display_player;
}

void P_SetConsolePlayer(player_t *player) {
  console_player = player;
}

void P_SetDisplayPlayer(player_t *player) {
  display_player = player;
}

player_t* PL_New(void) {
  player_t *player = calloc(1, sizeof(player_t));

  PL_InitMessaging(player);
  PL_InitCommandQueue(player);

  player->frags = g_hash_table_new(NULL, NULL);
  player->deaths = g_hash_table_new(NULL, NULL);
}

bool PL_IsConsolePlayer(player_t *player) {
  return player == console_player;
}

bool PL_IsDisplayPlayer(player_t *player) {
  return player == display_player;
}

void PL_SetNameRaw(player_t *player, const char *name) {
  if (player->name) {
    free(player->name);
  }

  player->name = name;
}

void PL_SetName(player_t *player, const char *name) {
  if (!name) {
    return;
  }

  if (!strlen(name)) {
    return;
  }

  P_Printf(consoleplayer, "%s is now %s.\n", player->name, name);

  if (SERVER) {
    SV_BroadcastPrintf("%s is now %s.\n", player->name, name);
  }

  P_SetNameRaw(player, strdup(name));

  if (!PL_IsConsolePlayer(player)) {
    return;
  }

  if (CLIENT) {
    CL_SendNameChange(player->name);
  }

  if (SERVER) {
    SV_BroadcastPlayerNameChanged(player, player->name);
  }
}

void PL_AddDeath(player_t *victim, player_t *fragger) {
  uint32_t fragger_id = 0;

  if (fragger) {
    if (fragger != victim) {
      uint32_t frag_count = G_POINTER_TO_UINT(g_hash_table_lookup(
        fragger->frags,
        GUINT_TO_POINTER(victim->id)
      )) + 1;

      g_hash_table_insert(G_UINT_TO_POINTER(
        fragger->frags,
        GUINT_TO_POINTER(victim->id),
        GUINT_TO_POINTER(frag_count)
      ));
    }

    fragger_id = fragger->id;
  }

  uint32_t death_count = G_POINTER_TO_UINT(g_hash_table_lookup(
    victim->deaths,
    GUINT_TO_POINTER(fragger_id),
  )) + 1;

  g_hash_table_insert(G_UINT_TO_POINTER(
    victim->deaths,
    GUINT_TO_POINTER(fragger_id),
    GUINT_TO_POINTER(death_count)
  ));
}

uint32_t PL_GetFrags(player_t *fragger, player_t *victim) {
  if (fragger == victim) {
    return 0;
  }

  return G_POINTER_TO_UINT(g_hash_table_lookup(
    fragger->frags,
    GUINT_TO_POINTER(victim->id)
  ));
}

uint32_t PL_GetTotalFrags(player_t *player) {
  uint32_t total_frags = 0;
  GHashTableIter iter;
  gpointer key;
  gpointer value;

  g_hash_table_iter_init(&iter, player->frags);

  while (g_hash_table_iter_next(&iter, &key, &value)) {
    total_frags += G_POINTER_TO_UINT(value);
  }

  return total_frags;
}

uint32_t PL_GetMurders(player_t *victim, player_t *fragger) {
  return G_POINTER_TO_UINT(g_hash_table_lookup(
    victim->deaths,
    GUINT_TO_POINTER(fragger->id)
  ));
}

uint32_t PL_GetTotalMurders(player_t *victim) {
  uint32_t total_murders = 0;
  GHashTableIter iter;
  gpointer key;
  gpointer value;

  g_hash_table_iter_init(&iter, victim->deaths);

  while (g_hash_table_iter_next(&iter, &key, &value)) {
    uint32_t fragger_id = G_POINTER_TO_UINT(key);

    if ((fragger_id != 0) && (fragger_id != player->id)) {
      total_murders += G_POINTER_TO_UINT(value);
    }
  }

  return total_murders;
}

uint32_t PL_GetTotalDeaths(player_t *victim) {
  return G_POINTER_TO_UINT(g_hash_table_lookup(
    victim->deaths,
    GUINT_TO_POINTER(0)
  )):
}

uint32_t PL_GetTotalSuicides(player_t *victim) {
  return G_POINTER_TO_UINT(g_hash_table_lookup(
    victim->deaths,
    GUINT_TO_POINTER(victim->id)
  )):
}

int32_t PL_GetFragScore(player_t *player) {
  uint32_t frag_count = PL_GetTotalFrags(player);
  uint32_t death_count = 0;
  GHashTableIter iter;
  gpointer key;
  gpointer value;

  g_hash_table_iter_init(&iter, player->deaths);

  while (g_hash_table_iter_next(&iter, &key, &value)) {
    death_count += G_POINTER_TO_UINT(value);
  }

  return frag_count - death_count;
}

void PL_ClearFragsAndDeaths(player_t *player) {
  g_hash_table_remove_all(player->frags);
  g_hash_table_remove_all(player->deaths);
}

int P_PlayerGetPing(player_t *player) {
  return player->ping;
}

int P_PlayerGetTime(player_t *player) {
  if (!MULTINET) {
    return gametic / TICRATE;
  }

  return (gametic - player->connect_tic) / TICRATE;
}

//
// PL_Spawn
// Called when a player is spawned on the level.
// Most of the player structure stays unchanged
//  between levels.
//

void PL_Spawn(player_t *player, const mapthing_t *mthing) {
  int i;
  fixed_t x;
  fixed_t y;
  fixed_t z;
  mobj_t *mobj;

  // e6y
  // playeringame overflow detection
  // it detects and emulates overflows on vex6d.wad\bug_wald(toke).lmp, etc.
  // http://www.doom2.net/doom2/research/runningbody.zip
  if ((!MULTINET) && PlayeringameOverrun(mthing)) {
    return;
  }

  if (p->playerstate == PST_REBORN) {
    G_PlayerReborn(playernum);
  }

  /* cph 2001/08/14 - use the options field of memorised player starts to
   * indicate whether the start really exists in the level.
   */
  if (!mthing->options) {
    I_Error(
      "P_SpawnPlayer: attempt to spawn player at unavailable start point"
    );
  }

  x    = mthing->x << FRACBITS;
  y    = mthing->y << FRACBITS;
  z    = ONFLOORZ;
  mobj = P_SpawnMobj(x, y, z, MT_PLAYER);

  if (deathmatch) {
    mobj->index = TracerGetDeathmatchStart(player);
  }
  else {
    mobj->index = TracerGetPlayerStart(mthing->type - 1);
  }

  // set color translations for player sprites

  mobj->flags |= playernumtotrans[P_GetVanillaNum(player)] << MF_TRANSSHIFT;

  /* CG: Make the server's player invisible, etc. */
  if (MULTINET && player->id == 1) {
    mobj->flags &= ~MF_SOLID;
    mobj->flags &= ~MF_SHOOTABLE;
    mobj->flags |=  MF_NOSECTOR;
    mobj->flags |=  MF_NOBLOCKMAP;
  }

  mobj->angle      = ANG45 * (mthing->angle / 45);
  mobj->player     = player;
  mobj->health     = player->health;
  mobj->player->prev_viewangle = mobj->angle + viewangleoffset;

  player->mo            = mobj;
  player->playerstate   = PST_LIVE;
  player->refire        = 0;
  PL_ClearMessages(player);
  player->damagecount   = 0;
  player->bonuscount    = 0;
  player->extralight    = 0;
  player->fixedcolormap = 0;
  player->viewheight    = VIEWHEIGHT;

  player->momx = player->momy = 0; // killough 10/98: initialize bobbing to 0.

  // setup gun psprite

  P_SetupPsprites(player);

  // give all cards in death match mode

  if (deathmatch) {
    for (i = 0; i < NUMCARDS; i++) {
      player->cards[i] = true;
    }
  }

  if (mthing->type == P_GetConsolePlayer()->id) {
    ST_Start(); // wake up the status bar
    HU_Start(); // wake up the heads up text
  }

  PL_ClearCommands(player);

  R_SmoothPlaying_Reset(player); // e6y
}

/* vi: set et ts=2 sw=2: */
