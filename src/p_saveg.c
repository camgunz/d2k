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

#include "doomstat.h"
#include "am_map.h"
#include "g_game.h"
#include "lprintf.h"
#include "m_random.h"
#include "n_net.h"
#include "p_cmd.h"
#include "p_enemy.h"
#include "p_ident.h"
#include "p_map.h"
#include "p_maputl.h"
#include "p_setup.h"
#include "p_spec.h"
#include "p_tick.h"
#include "p_saveg.h"
#include "p_user.h"
#include "r_main.h"
#include "s_advsound.h"
#include "e6y.h"//e6y

extern int numspechit;

#define MAX_PLAYER_MESSAGE_SIZE 256
#define MAX_COMMAND_COUNT 10000

static uint32_t current_thinker_count;

typedef enum {
  tc_end,
  tc_mobj
} thinkerclass_t;

enum {
  tc_ceiling,
  tc_door,
  tc_floor,
  tc_plat,
  tc_flash,
  tc_strobe,
  tc_glow,
  tc_elevator,    //jff 2/22/98 new elevator type thinker
  tc_scroll,      // killough 3/7/98: new scroll effect thinker
  tc_pusher,      // phares 3/22/98:  new push/pull effect thinker
  tc_flicker,     // killough 10/4/98
  tc_endspecials
} specials_e;

static void serialize_corpse(gpointer data, gpointer user_data) {
  mobj_t *corpse = (mobj_t *)data;
  pbuf_t *savebuffer = (pbuf_t *)user_data;

  M_PBufWriteUInt(savebuffer, corpse->id);
}

static void P_ArchivePlayer(pbuf_t *savebuffer, player_t *player) {
  M_PBufWriteInt(savebuffer, player->playerstate);
  M_PBufWriteChar(savebuffer, player->cmd.forwardmove);
  M_PBufWriteChar(savebuffer, player->cmd.sidemove);
  M_PBufWriteShort(savebuffer, player->cmd.angleturn);
  M_PBufWriteShort(savebuffer, player->cmd.consistancy);
  M_PBufWriteUChar(savebuffer, player->cmd.chatchar);
  M_PBufWriteUChar(savebuffer, player->cmd.buttons);
  M_PBufWriteInt(savebuffer, player->viewz);
  M_PBufWriteInt(savebuffer, player->viewheight);
  M_PBufWriteInt(savebuffer, player->deltaviewheight);
  M_PBufWriteInt(savebuffer, player->bob);
  M_PBufWriteInt(savebuffer, player->health);
  M_PBufWriteInt(savebuffer, player->armorpoints);
  M_PBufWriteInt(savebuffer, player->armortype);
  for (int i = 0; i < NUMPOWERS; i++)
    M_PBufWriteInt(savebuffer, player->powers[i]);
  for (int i = 0; i < NUMCARDS; i++)
    M_PBufWriteBool(savebuffer, player->cards[i]);
  M_PBufWriteBool(savebuffer, player->backpack);
  for (int i = 0; i < MAXPLAYERS; i++)
    M_PBufWriteInt(savebuffer, player->frags[i]);
  M_PBufWriteInt(savebuffer, player->readyweapon);
  M_PBufWriteInt(savebuffer, player->pendingweapon);
  for (int i = 0; i < NUMWEAPONS; i++)
    M_PBufWriteBool(savebuffer, player->weaponowned[i]);
  for (int i = 0; i < NUMAMMO; i++)
    M_PBufWriteInt(savebuffer, player->ammo[i]);
  for (int i = 0; i < NUMAMMO; i++)
    M_PBufWriteInt(savebuffer, player->maxammo[i]);
  M_PBufWriteInt(savebuffer, player->attackdown);
  M_PBufWriteInt(savebuffer, player->usedown);
  M_PBufWriteInt(savebuffer, player->cheats);
  M_PBufWriteInt(savebuffer, player->refire);
  M_PBufWriteInt(savebuffer, player->killcount);
  M_PBufWriteInt(savebuffer, player->itemcount);
  M_PBufWriteInt(savebuffer, player->secretcount);
  M_PBufWriteInt(savebuffer, player->damagecount);
  M_PBufWriteInt(savebuffer, player->bonuscount);
  M_PBufWriteInt(savebuffer, player->extralight);
  M_PBufWriteInt(savebuffer, player->fixedcolormap);
  M_PBufWriteInt(savebuffer, player->colormap);
  for (int i = 0; i < NUMPSPRITES; i++) {
    if (player->psprites[i].state) {
      uint_64_t state_index;

      if (player->psprites[i].state < states) {
        I_Error(
          "P_ArchivePlayer: Invalid psprite state %p",
          player->psprites[i].state
        );
      }

      state_index = player->psprites[i].state - states;

      if (state_index > NUMSTATES) {
        I_Error(
          "P_ArchivePlayer: Invalid psprite state %p",
          player->psprites[i].state
        );
      }

      M_PBufWriteULong(savebuffer, state_index + 1);
      M_PBufWriteInt(savebuffer, player->psprites[i].tics);
      M_PBufWriteInt(savebuffer, player->psprites[i].sx);
      M_PBufWriteInt(savebuffer, player->psprites[i].sy);
    }
    else {
      M_PBufWriteULong(savebuffer, 0);
      M_PBufWriteInt(savebuffer, 0);
      M_PBufWriteInt(savebuffer, 0);
      M_PBufWriteInt(savebuffer, 0);
    }
  }

  M_PBufWriteInt(savebuffer, player->didsecret);
  M_PBufWriteInt(savebuffer, player->momx);
  M_PBufWriteInt(savebuffer, player->momy);
  M_PBufWriteInt(savebuffer, player->resurectedkillcount);
  M_PBufWriteInt(savebuffer, player->prev_viewz);
  M_PBufWriteInt(savebuffer, player->prev_viewangle);
  M_PBufWriteInt(savebuffer, player->prev_viewpitch);
  M_PBufWriteInt(savebuffer, player->jumpTics);
  if (player->name)
    M_PBufWriteString(savebuffer, player->name, strlen(player->name));
  else
    M_PBufWriteString(savebuffer, "", 0);
  M_PBufWriteUChar(savebuffer, player->team);
}

static void P_UnArchivePlayer(pbuf_t *savebuffer, player_t *player) {
  static buf_t player_message_buf;
  static buf_t name_buf;
  static bool player_message_buf_initialized = false;
  static bool name_buf_initialized = false;

  if (!player_message_buf_initialized) {
    M_BufferInit(&player_message_buf);
    player_message_buf_initialized = true;
  }

  if (!name_buf_initialized) {
    M_BufferInitWithCapacity(&name_buf, MAX_NAME_LENGTH);
    name_buf_initialized = true;
  }

  M_BufferClear(&name_buf);

  player->mo = NULL;

  M_PBufReadInt(savebuffer, (int *)&player->playerstate);
  M_PBufReadChar(savebuffer, &player->cmd.forwardmove);
  M_PBufReadChar(savebuffer, &player->cmd.sidemove);
  M_PBufReadShort(savebuffer, &player->cmd.angleturn);
  M_PBufReadShort(savebuffer, &player->cmd.consistancy);
  M_PBufReadUChar(savebuffer, &player->cmd.chatchar);
  M_PBufReadUChar(savebuffer, &player->cmd.buttons);
  M_PBufReadInt(savebuffer, &player->viewz);
  M_PBufReadInt(savebuffer, &player->viewheight);
  M_PBufReadInt(savebuffer, &player->deltaviewheight);
  M_PBufReadInt(savebuffer, &player->bob);
  M_PBufReadInt(savebuffer, &player->health);
  M_PBufReadInt(savebuffer, &player->armorpoints);
  M_PBufReadInt(savebuffer, &player->armortype);
  for (int i = 0; i < NUMPOWERS; i++)
    M_PBufReadInt(savebuffer, &player->powers[i]);
  for (int i = 0; i < NUMCARDS; i++)
    M_PBufReadBool(savebuffer, (int *)&player->cards[i]);
  M_PBufReadBool(savebuffer, (int *)&player->backpack);
  for (int i = 0; i < MAXPLAYERS; i++)
    M_PBufReadInt(savebuffer, &player->frags[i]);
  M_PBufReadInt(savebuffer, (int *)&player->readyweapon);
  M_PBufReadInt(savebuffer, (int *)&player->pendingweapon);
  for (int i = 0; i < NUMWEAPONS; i++)
    M_PBufReadBool(savebuffer, (int *)&player->weaponowned[i]);
  for (int i = 0; i < NUMAMMO; i++)
    M_PBufReadInt(savebuffer, &player->ammo[i]);
  for (int i = 0; i < NUMAMMO; i++)
    M_PBufReadInt(savebuffer, &player->maxammo[i]);
  M_PBufReadInt(savebuffer, &player->attackdown);
  M_PBufReadInt(savebuffer, &player->usedown);
  M_PBufReadInt(savebuffer, &player->cheats);
  M_PBufReadInt(savebuffer, &player->refire);
  M_PBufReadInt(savebuffer, &player->killcount);
  M_PBufReadInt(savebuffer, &player->itemcount);
  M_PBufReadInt(savebuffer, &player->secretcount);
  M_PBufReadInt(savebuffer, &player->damagecount);
  M_PBufReadInt(savebuffer, &player->bonuscount);
  M_PBufReadInt(savebuffer, &player->extralight);
  M_PBufReadInt(savebuffer, &player->fixedcolormap);
  M_PBufReadInt(savebuffer, &player->colormap);
  for (int i = 0; i < NUMPSPRITES; i++) {
    uint_64_t state_index = 0;

    M_PBufReadULong(savebuffer, &state_index);

    if (state_index > NUMSTATES) {
      I_Error(
        "P_UnArchivePlayer: State index %" PRId64 " out of range\n",
        state_index
      );
    }

    if (state_index == 0)
      player->psprites[i].state = NULL;
    else
      player->psprites[i].state = &states[state_index - 1];

    M_PBufReadInt(savebuffer, &player->psprites[i].tics);
    M_PBufReadInt(savebuffer, &player->psprites[i].sx);
    M_PBufReadInt(savebuffer, &player->psprites[i].sy);
  }

  M_PBufReadInt(savebuffer, (int *)&player->didsecret);
  M_PBufReadInt(savebuffer, &player->momx);
  M_PBufReadInt(savebuffer, &player->momy);
  M_PBufReadInt(savebuffer, &player->resurectedkillcount);
  M_PBufReadInt(savebuffer, &player->prev_viewz);
  M_PBufReadInt(savebuffer, (int *)&player->prev_viewangle);
  M_PBufReadInt(savebuffer, (int *)&player->prev_viewpitch);
  M_PBufReadInt(savebuffer, &player->jumpTics);
  M_PBufReadString(savebuffer, &name_buf, MAX_NAME_LENGTH);
  if (M_BufferGetSize(&name_buf) > 0) {
    if (player->name)
      free(player->name);

    M_BufferSeek(&name_buf, 0);
    M_BufferReadStringDup(&name_buf, &player->name);
  }
  M_PBufReadUChar(savebuffer, &player->team);
}

static void P_ArchiveActorPointers(pbuf_t *savebuffer, mobj_t *mobj) {
  /* CG 06/26/2014: Try not storing indices in pointers */
  unsigned int target_id = 0;
  unsigned int tracer_id = 0;
  unsigned int lastenemy_id = 0;
  uint_64_t    state_index = 0;
  unsigned int player_index = 0;

  // killough 2/14/98: convert pointers into indices.
  // Fixes many savegame problems, by properly saving
  // target and tracer fields. Note: we store NULL if
  // the thinker pointed to by these fields is not a
  // mobj thinker.

  if (mobj->target && mobj->target->thinker.function == P_MobjThinker)
    target_id = mobj->target->id;

  if (mobj->tracer && mobj->tracer->thinker.function == P_MobjThinker)
    tracer_id = mobj->tracer->id;

  // killough 2/14/98: new field: save last known enemy. Prevents
  // monsters from going to sleep after killing monsters and not
  // seeing player anymore.

  if (mobj->lastenemy && mobj->lastenemy->thinker.function == P_MobjThinker)
    lastenemy_id = mobj->lastenemy->id;

  // killough 2/14/98: end changes

  if (mobj->state < states)
    I_Error("P_ArchiveActor: Invalid mobj state %p", mobj->state);

  state_index = (uint_64_t)(mobj->state - states);

  if (state_index >= NUMSTATES) 
    I_Error("P_ArchiveActor: Invalid mobj state %p", mobj->state);

  state_index++;

  if (mobj->player) {
    for (unsigned int i = 0; i < MAXPLAYERS; i++) {
      if (mobj->player == &players[i]) {
        player_index = i + 1;
        break;
      }
    }
    if (player_index == 0)
      I_Error("P_ArchiveActor: Invalid mobj player %p", mobj->player);
  }

  M_PBufWriteUInt(savebuffer, mobj->id);
  M_PBufWriteULong(savebuffer, state_index);
  M_PBufWriteUInt(savebuffer, target_id);
  M_PBufWriteUInt(savebuffer, player_index);
  M_PBufWriteUInt(savebuffer, tracer_id);
  M_PBufWriteUInt(savebuffer, lastenemy_id);

  /*
  D_Log(LOG_SAVE, "  Actor %u: {%d, %d, %d, %d, %d, %d, %d}\n",
    mobj->id,
    mobj->validcount,
    mobj->movedir,
    mobj->movecount,
    mobj->strafecount,
    mobj->reactiontime,
    mobj->threshold,
    mobj->pursuecount
  );
  */
}

static void P_UnArchiveActorPointers(pbuf_t *savebuffer, mobj_t *mobj) {
  unsigned int target_id = 0;
  unsigned int tracer_id = 0;
  unsigned int lastenemy_id = 0;
  uint_64_t    state_index = 0;
  unsigned int player_index = 0;
  mobj_t *target = NULL;
  mobj_t *tracer = NULL;
  mobj_t *lastenemy = NULL;

  mobj->target = NULL;
  mobj->tracer = NULL;
  mobj->lastenemy = NULL;
  mobj->state = NULL;
  mobj->player = NULL;

  M_PBufReadULong(savebuffer, &state_index);
  M_PBufReadUInt(savebuffer, &target_id);
  M_PBufReadUInt(savebuffer, &player_index);
  M_PBufReadUInt(savebuffer, &tracer_id);
  M_PBufReadUInt(savebuffer, &lastenemy_id);

  if (state_index > NUMSTATES)
    I_Error("P_UnArchiveActor: Invalid mobj state %" PRIu64, state_index);

  if (player_index > MAXPLAYERS)
    I_Error("P_UnArchiveActor: Invalid mobj player %u", player_index);

  if (state_index)
    mobj->state = &states[state_index - 1];
  else
    mobj->state = (state_t *)S_NULL;

  if (target_id) {
    target = P_IdentLookup(target_id);

    if (target == NULL)
      I_Error("P_UnArchiveActor: Invalid mobj target %u", target_id);

    P_SetTarget(&mobj->target, P_IdentLookup(target_id));
  }

  if (tracer_id) {
    tracer = P_IdentLookup(tracer_id);

    if (tracer == NULL)
      I_Error("P_UnArchiveActor: Invalid mobj tracer %u", tracer_id);

    P_SetTarget(&mobj->tracer, P_IdentLookup(tracer_id));
  }

  if (lastenemy_id) {
    lastenemy = P_IdentLookup(lastenemy_id);

    if (lastenemy == NULL)
      I_Error("P_UnArchiveActor: Invalid mobj lastenemy %u", lastenemy_id);

    P_SetTarget(&mobj->lastenemy, P_IdentLookup(lastenemy_id));
  }

  if (player_index) {
    mobj->player = &players[player_index - 1];
    mobj->player->mo = mobj;
  }

  /*
  D_Log(LOG_SAVE, "  Actor %u: {%d, %d, %d, %d, %d, %d, %d}\n",
    mobj->id,
    mobj->validcount,
    mobj->movedir,
    mobj->movecount,
    mobj->strafecount,
    mobj->reactiontime,
    mobj->threshold,
    mobj->pursuecount
  );
  */
}

static void P_ArchiveActor(pbuf_t *savebuffer, mobj_t *mobj) {
  M_PBufWriteInt(savebuffer, mobj->x);
  M_PBufWriteInt(savebuffer, mobj->y);
  M_PBufWriteInt(savebuffer, mobj->z);
  M_PBufWriteUInt(savebuffer, mobj->angle);
  M_PBufWriteInt(savebuffer, mobj->sprite);
  M_PBufWriteInt(savebuffer, mobj->frame);
  M_PBufWriteInt(savebuffer, mobj->floorz);
  M_PBufWriteInt(savebuffer, mobj->ceilingz);
  M_PBufWriteInt(savebuffer, mobj->dropoffz);
  M_PBufWriteInt(savebuffer, mobj->radius);
  M_PBufWriteInt(savebuffer, mobj->height);
  M_PBufWriteInt(savebuffer, mobj->momx);
  M_PBufWriteInt(savebuffer, mobj->momy);
  M_PBufWriteInt(savebuffer, mobj->momz);
  M_PBufWriteInt(savebuffer, mobj->validcount);
  M_PBufWriteInt(savebuffer, mobj->type);
  M_PBufWriteInt(savebuffer, mobj->tics);
  M_PBufWriteULong(savebuffer, mobj->flags);
  M_PBufWriteInt(savebuffer, mobj->intflags);
  M_PBufWriteInt(savebuffer, mobj->health);
  M_PBufWriteShort(savebuffer, mobj->movedir);
  M_PBufWriteShort(savebuffer, mobj->movecount);
  M_PBufWriteShort(savebuffer, mobj->strafecount);
  M_PBufWriteShort(savebuffer, mobj->reactiontime);
  M_PBufWriteShort(savebuffer, mobj->threshold);
  M_PBufWriteShort(savebuffer, mobj->pursuecount);
  M_PBufWriteShort(savebuffer, mobj->gear);
  M_PBufWriteShort(savebuffer, mobj->lastlook);
  M_PBufWriteShort(savebuffer, mobj->spawnpoint.x);
  M_PBufWriteShort(savebuffer, mobj->spawnpoint.y);
  M_PBufWriteShort(savebuffer, mobj->spawnpoint.angle);
  M_PBufWriteShort(savebuffer, mobj->spawnpoint.type);
  M_PBufWriteShort(savebuffer, mobj->spawnpoint.options);
  M_PBufWriteInt(savebuffer, mobj->friction);
  M_PBufWriteInt(savebuffer, mobj->movefactor);
  M_PBufWriteInt(savebuffer, mobj->PrevX);
  M_PBufWriteInt(savebuffer, mobj->PrevY);
  M_PBufWriteInt(savebuffer, mobj->PrevZ);
  M_PBufWriteUInt(savebuffer, mobj->pitch);
  M_PBufWriteInt(savebuffer, mobj->index);
  M_PBufWriteShort(savebuffer, mobj->patch_width);
  M_PBufWriteInt(savebuffer, mobj->iden_nums);
  M_PBufWriteUInt(savebuffer, mobj->id);
}

static void P_UnArchiveActor(pbuf_t *savebuffer, mobj_t *mobj) {
  M_PBufReadInt(savebuffer, &mobj->x);
  M_PBufReadInt(savebuffer, &mobj->y);
  M_PBufReadInt(savebuffer, &mobj->z);
  M_PBufReadUInt(savebuffer, &mobj->angle);
  M_PBufReadInt(savebuffer, (int *)&mobj->sprite);
  M_PBufReadInt(savebuffer, &mobj->frame);
  M_PBufReadInt(savebuffer, &mobj->floorz);
  M_PBufReadInt(savebuffer, &mobj->ceilingz);
  M_PBufReadInt(savebuffer, &mobj->dropoffz);
  M_PBufReadInt(savebuffer, &mobj->radius);
  M_PBufReadInt(savebuffer, &mobj->height);
  M_PBufReadInt(savebuffer, &mobj->momx);
  M_PBufReadInt(savebuffer, &mobj->momy);
  M_PBufReadInt(savebuffer, &mobj->momz);
  M_PBufReadInt(savebuffer, &mobj->validcount);
  M_PBufReadInt(savebuffer, (int *)&mobj->type);
  M_PBufReadInt(savebuffer, &mobj->tics);
  M_PBufReadULong(savebuffer, &mobj->flags);
  M_PBufReadInt(savebuffer, &mobj->intflags);
  M_PBufReadInt(savebuffer, &mobj->health);
  M_PBufReadShort(savebuffer, &mobj->movedir);
  M_PBufReadShort(savebuffer, &mobj->movecount);
  M_PBufReadShort(savebuffer, &mobj->strafecount);
  M_PBufReadShort(savebuffer, &mobj->reactiontime);
  M_PBufReadShort(savebuffer, &mobj->threshold);
  M_PBufReadShort(savebuffer, &mobj->pursuecount);
  M_PBufReadShort(savebuffer, &mobj->gear);
  M_PBufReadShort(savebuffer, &mobj->lastlook);
  M_PBufReadShort(savebuffer, &mobj->spawnpoint.x);
  M_PBufReadShort(savebuffer, &mobj->spawnpoint.y);
  M_PBufReadShort(savebuffer, &mobj->spawnpoint.angle);
  M_PBufReadShort(savebuffer, &mobj->spawnpoint.type);
  M_PBufReadShort(savebuffer, &mobj->spawnpoint.options);
  M_PBufReadInt(savebuffer, &mobj->friction);
  M_PBufReadInt(savebuffer, &mobj->movefactor);
  M_PBufReadInt(savebuffer, &mobj->PrevX);
  M_PBufReadInt(savebuffer, &mobj->PrevY);
  M_PBufReadInt(savebuffer, &mobj->PrevZ);
  M_PBufReadUInt(savebuffer, &mobj->pitch);
  M_PBufReadInt(savebuffer, &mobj->index);
  M_PBufReadShort(savebuffer, &mobj->patch_width);
  M_PBufReadInt(savebuffer, &mobj->iden_nums);
  M_PBufReadUInt(savebuffer, &mobj->id);

  P_IdentAssignID(mobj, mobj->id);
}

//
// P_ArchivePlayers
//
void P_ArchivePlayers(pbuf_t *savebuffer) {
  for (int i = 0; i < MAXPLAYERS; i++) {
    if (playeringame[i])
      P_ArchivePlayer(savebuffer, &players[i]);
  }
}

//
// P_UnArchivePlayers
//
void P_UnArchivePlayers(pbuf_t *savebuffer) {
  for (int i = 0; i < MAXPLAYERS; i++) {
    if (!playeringame[i])
      continue;

    P_UnArchivePlayer(savebuffer, &players[i]);

    // will be set when unarc thinker
    players[i].mo = NULL;
    players[i].attacker = NULL;
  }
}

//
// P_ArchiveWorld
//
void P_ArchiveWorld(pbuf_t *savebuffer) {
  M_PBufWriteInt(savebuffer, numspechit);
  M_PBufWriteUInt(savebuffer, numsectors);
  M_PBufWriteUInt(savebuffer, numlines);

  // do sectors
  for (sector_t *sec = sectors; (sec - sectors) < numsectors; sec++) {
    // killough 10/98: save full floor & ceiling heights, including fraction
    M_PBufWriteInt(savebuffer, sec->floorheight);
    M_PBufWriteInt(savebuffer, sec->ceilingheight);

    M_PBufWriteShort(savebuffer, sec->floorpic);
    M_PBufWriteShort(savebuffer, sec->ceilingpic);
    M_PBufWriteShort(savebuffer, sec->lightlevel);

    // needed?   yes -- transfer types
    M_PBufWriteShort(savebuffer, sec->special);

    // needed?   need them -- killough
    M_PBufWriteShort(savebuffer, sec->tag);

    M_PBufWriteInt(savebuffer, sec->soundorg.x);
    M_PBufWriteInt(savebuffer, sec->soundorg.y);
    M_PBufWriteInt(savebuffer, sec->soundorg.z);
    M_PBufWriteUInt(savebuffer, sec->soundorg.id);
  }

  // do lines
  for (line_t *li = lines; (li - lines) < numlines; li++) {
    M_PBufWriteShort(savebuffer, li->flags);
    M_PBufWriteShort(savebuffer, li->special);
    M_PBufWriteShort(savebuffer, li->tag);

    for (int j = 0; j < 2; j++) {
      M_PBufWriteUChar(savebuffer, j);

      if (li->sidenum[j] == NO_INDEX) {
        M_PBufWriteBool(savebuffer, false);
      }
      else {
        side_t *si = &sides[li->sidenum[j]];

        M_PBufWriteBool(savebuffer, true);
        // killough 10/98: save full sidedef offsets,
        // preserving fractional scroll offsets
        M_PBufWriteInt(savebuffer, si->textureoffset);
        M_PBufWriteInt(savebuffer, si->rowoffset);
        M_PBufWriteShort(savebuffer, si->toptexture);
        M_PBufWriteShort(savebuffer, si->bottomtexture);
        M_PBufWriteShort(savebuffer, si->midtexture);
      }
    }

    M_PBufWriteInt(savebuffer, li->soundorg.x);
    M_PBufWriteInt(savebuffer, li->soundorg.y);
    M_PBufWriteInt(savebuffer, li->soundorg.z);
    M_PBufWriteUInt(savebuffer, li->soundorg.id);
  }

  M_PBufWriteInt(savebuffer, musinfo.current_item);
}

//
// P_UnArchiveWorld
//
void P_UnArchiveWorld(pbuf_t *savebuffer) {
  uint32_t sector_count;
  uint32_t line_count;

  M_PBufReadInt(savebuffer, &numspechit);
  M_PBufReadUInt(savebuffer, &sector_count);
  M_PBufReadUInt(savebuffer, &line_count);

  if (sector_count != numsectors) {
    int32_t junk_int;
    int16_t junk_short;
    uint32_t junk_uint;

    while (sector_count--) {
      M_PBufReadInt(savebuffer, &junk_int);
      M_PBufReadInt(savebuffer, &junk_int);
      M_PBufReadShort(savebuffer, &junk_short);
      M_PBufReadShort(savebuffer, &junk_short);
      M_PBufReadShort(savebuffer, &junk_short);
      M_PBufReadShort(savebuffer, &junk_short);
      M_PBufReadShort(savebuffer, &junk_short);

      M_PBufReadInt(savebuffer, &junk_int);
      M_PBufReadInt(savebuffer, &junk_int);
      M_PBufReadInt(savebuffer, &junk_int);
      M_PBufReadUInt(savebuffer, &junk_uint);
    }
  }
  else {
    for (sector_t *sec = sectors; (sec - sectors) < numsectors; sec++) {
      // killough 10/98: load full floor & ceiling heights, including fractions
      M_PBufReadInt(savebuffer, &sec->floorheight);
      M_PBufReadInt(savebuffer, &sec->ceilingheight);
      M_PBufReadShort(savebuffer, &sec->floorpic);
      M_PBufReadShort(savebuffer, &sec->ceilingpic);
      M_PBufReadShort(savebuffer, &sec->lightlevel);
      M_PBufReadShort(savebuffer, &sec->special);
      M_PBufReadShort(savebuffer, &sec->tag);

      M_PBufReadInt(savebuffer, &sec->soundorg.x);
      M_PBufReadInt(savebuffer, &sec->soundorg.y);
      M_PBufReadInt(savebuffer, &sec->soundorg.z);
      M_PBufReadUInt(savebuffer, &sec->soundorg.id);
      P_IdentAssignID(&sec->soundorg, sec->soundorg.id);

      sec->ceilingdata = 0; //jff 2/22/98 now three thinker fields, not two
      sec->floordata = 0;
      sec->lightingdata = 0;
      sec->soundtarget = 0;
    }
  }

  if (line_count != numlines) {
    while (line_count--) {
      line_t junk_line;
      side_t junk_side;
      line_t *li = &junk_line;
      side_t *si = &junk_side;

      M_PBufReadShort(savebuffer, (short *)&li->flags);
      M_PBufReadShort(savebuffer, &li->special);
      M_PBufReadShort(savebuffer, &li->tag);

      for (int j = 0; j < 2; j++) {
        unsigned char sidenum_index;
        dboolean sidenum_index_valid;

        M_PBufReadUChar(savebuffer, &sidenum_index);

        if (sidenum_index != j)
          I_Error("P_UnArchiveWorld: side index %d != %d\n", sidenum_index, j);

        M_PBufReadBool(savebuffer, &sidenum_index_valid);

        if (sidenum_index_valid) {
          // killough 10/98: load full sidedef offsets, including fractions
          M_PBufReadInt(savebuffer, &si->textureoffset);
          M_PBufReadInt(savebuffer, &si->rowoffset);
          M_PBufReadShort(savebuffer, &si->toptexture);
          M_PBufReadShort(savebuffer, &si->bottomtexture);
          M_PBufReadShort(savebuffer, &si->midtexture);
        }
      }

      M_PBufReadInt(savebuffer, &li->soundorg.x);
      M_PBufReadInt(savebuffer, &li->soundorg.y);
      M_PBufReadInt(savebuffer, &li->soundorg.z);
      M_PBufReadUInt(savebuffer, &li->soundorg.id);
    }
  }
  else {
    for (line_t *li = lines; (li - lines) < numlines; li++) {
      M_PBufReadShort(savebuffer, (short *)&li->flags);
      M_PBufReadShort(savebuffer, &li->special);
      M_PBufReadShort(savebuffer, &li->tag);

      for (int j = 0; j < 2; j++) {
        unsigned char sidenum_index;
        dboolean sidenum_index_valid;

        M_PBufReadUChar(savebuffer, &sidenum_index);

        if (sidenum_index != j)
          I_Error("P_UnArchiveWorld: side index %d != %d\n", sidenum_index, j);

        M_PBufReadBool(savebuffer, &sidenum_index_valid);

        if (sidenum_index_valid) {
          side_t *si = &sides[li->sidenum[j]];

          // killough 10/98: load full sidedef offsets, including fractions
          M_PBufReadInt(savebuffer, &si->textureoffset);
          M_PBufReadInt(savebuffer, &si->rowoffset);
          M_PBufReadShort(savebuffer, &si->toptexture);
          M_PBufReadShort(savebuffer, &si->bottomtexture);
          M_PBufReadShort(savebuffer, &si->midtexture);
        }
      }

      M_PBufReadInt(savebuffer, &li->soundorg.x);
      M_PBufReadInt(savebuffer, &li->soundorg.y);
      M_PBufReadInt(savebuffer, &li->soundorg.z);
      M_PBufReadUInt(savebuffer, &li->soundorg.id);
      P_IdentAssignID(&li->soundorg, li->soundorg.id);
    }
  }

  M_PBufReadInt(savebuffer, &musinfo.current_item);
}

//
// P_ArchiveThinkers
//
// 2/14/98 killough: substantially modified to fix savegame bugs
//

void P_ArchiveThinkers(pbuf_t *savebuffer) {
  unsigned int thinker_count = 0;

  // killough 3/26/98: Save boss brain state
  M_PBufWriteInt(savebuffer, brain.easy);
  M_PBufWriteInt(savebuffer, brain.targeton);

  // save off the current thinkers
  for (thinker_t *th = thinkercap.next; th != &thinkercap; th = th->next) {
    if (th->function == P_MobjThinker) {
      thinker_count++;
    }
  }

  M_PBufWriteUInt(savebuffer, thinker_count);

  for (thinker_t *th = thinkercap.next; th != &thinkercap; th = th->next) {
    if (th->function == P_MobjThinker)
      P_ArchiveActor(savebuffer, (mobj_t *)th);
  }

  for (thinker_t *th = thinkercap.next; th != &thinkercap; th = th->next) {
    if (th->function == P_MobjThinker)
      P_ArchiveActorPointers(savebuffer, (mobj_t *)th);
  }

  // killough 9/14/98: save soundtargets
  for (int i = 0; i < numsectors; i++) {
    mobj_t *target = sectors[i].soundtarget;

    if (target && target->thinker.function == P_MobjThinker)
      M_PBufWriteUInt(savebuffer, target->id);
    else
      M_PBufWriteUInt(savebuffer, 0);
  }

  if (corpse_queue) {
    M_PBufWriteInt(savebuffer, corpse_queue_size);
    M_PBufWriteUInt(savebuffer, g_queue_get_length(corpse_queue));
    g_queue_foreach(corpse_queue, serialize_corpse, savebuffer);
  }
  else {
    M_PBufWriteInt(savebuffer, 0);
  }

}

//
// P_UnArchiveThinkers
//
// 2/14/98 killough: substantially modified to fix savegame bugs
//

void P_UnArchiveThinkers(pbuf_t *savebuffer) {
  totallive = 0; /* CG: This is a global that lives in g_game.c, just FYI */

  // killough 3/26/98: Load boss brain state
  M_PBufReadInt(savebuffer, &brain.easy);
  M_PBufReadInt(savebuffer, &brain.targeton);

  for (thinker_t *th = thinkercap.next; th != &thinkercap; ) {
    thinker_t *next = th->next;

    if (th->function == P_MobjThinker)
      P_RemoveMobj((mobj_t *)th);

    th = next;
  }

  // remove all the current thinkers
  for (thinker_t *th = thinkercap.next; th != &thinkercap; ) {
    thinker_t *next = th->next;

    Z_Free(th);

    th = next;
  }

  P_InitThinkers();

  for (int i = 0; i < numsectors; i++) {
    sectors[i].thinglist = NULL;
    sectors[i].touching_thinglist = NULL;
  }

  memset(blocklinks, 0, bmapwidth * bmapheight * sizeof(*blocklinks));

  P_FreeSecNodeList();

  M_PBufReadUInt(savebuffer, &current_thinker_count);

  for (int i = 0; i < current_thinker_count; i++) {
    mobj_t *mo = Z_Malloc(sizeof(mobj_t), PU_LEVEL, NULL); // killough 2/14/98

    memset(mo, 0, sizeof(mobj_t));

    mo->thinker.function = P_MobjThinker;

    P_UnArchiveActor(savebuffer, mo);

    P_AddThinker(&mo->thinker);

    mo->sprev = NULL;
    mo->snext = NULL;
    P_SetThingPosition(mo);

    mo->info = &mobjinfo[mo->type];

    // killough 2/28/98:
    // Fix for falling down into a wall after savegame loaded:
    //      mobj->floorz = mobj->subsector->sector->floorheight;
    //      mobj->ceilingz = mobj->subsector->sector->ceilingheight;

    if (!((mo->flags ^ MF_COUNTKILL) & (MF_FRIEND | MF_COUNTKILL | MF_CORPSE)))
      totallive++;
  }

  for (int i = 0; i < current_thinker_count; i++) {
    uint32_t mobj_id;
    mobj_t *mo = NULL;
    
    M_PBufReadUInt(savebuffer, &mobj_id);
    mo = P_IdentLookup(mobj_id);

    if (mo)
      P_UnArchiveActorPointers(savebuffer, mo);
  }

  // killough 9/14/98: restore soundtargets
  for (int i = 0; i < numsectors; i++) {
    unsigned int st_id;
    mobj_t *sound_target = NULL;

    M_PBufReadUInt(savebuffer, &st_id);

    if (st_id == 0) {
      sectors[i].soundtarget = NULL;
      continue;
    }

    sound_target = P_IdentLookup(st_id);

    // Must verify soundtarget. See P_ArchiveThinkers.
    if (!sound_target)
      I_Error("P_UnArchiveThinkers: Invalid soundtarget index %u", st_id);

    P_SetTarget(&sectors[i].soundtarget, sound_target);
  }

  G_ClearCorpses();
  M_PBufReadInt(savebuffer, &corpse_queue_size);

  if (corpse_queue_size < 0) {
    I_Error(
      "P_UnArchiveThinkers: corpse_queue_size < 0 (%d)", corpse_queue_size
    );
  }

  if (corpse_queue_size > 0) {
    unsigned int corpse_count;

    M_PBufReadUInt(savebuffer, &corpse_count);

    if (corpse_count > corpse_queue_size) {
      I_Error("P_UnArchiveThinkers: corpse count > %d (%d)",
        corpse_count, corpse_queue_size
      );
    }

    for (int i = 0; i < corpse_count; i++) {
      mobj_t *mo;
      uint32_t corpse_id;

      M_PBufReadUInt(savebuffer, &corpse_id);

      mo = P_IdentLookup(corpse_id);

      if (mo == NULL)
        I_Error("P_UnArchiveThinkers: Invalid corpse ID %d", corpse_id);

      g_queue_push_tail(corpse_queue, mo);
    }
  }
  else {
    M_PBufWriteInt(savebuffer, 0);
  }

  // killough 3/26/98: Spawn icon landings:
  if (gamemode == commercial) {
    // P_SpawnBrainTargets overwrites brain.targeton and brain.easy with zero.
    struct brain_s brain_tmp = brain; // saving

    P_SpawnBrainTargets();

    // old demos with save/load tics should not be affected by this fix
    if (!prboom_comp[PC_RESET_MONSTERSPAWNER_PARAMS_AFTER_LOADING].state)
      brain = brain_tmp; // restoring
  }
}

//
// P_ArchiveSpecials
//

//
// Things to handle:
//
// T_MoveCeiling, (ceiling_t: sector_t * swizzle), - active list
// T_VerticalDoor, (vldoor_t: sector_t * swizzle),
// T_MoveFloor, (floormove_t: sector_t * swizzle),
// T_LightFlash, (lightflash_t: sector_t * swizzle),
// T_StrobeFlash, (strobe_t: sector_t *),
// T_Glow, (glow_t: sector_t *),
// T_PlatRaise, (plat_t: sector_t *), - active list
// T_MoveElevator, (plat_t: sector_t *), - active list      // jff 2/22/98
// T_Scroll                                                 // killough 3/7/98
// T_Pusher                                                 // phares 3/22/98
// T_FireFlicker                                            // killough 10/4/98
//

void P_ArchiveSectorIndex(pbuf_t *savebuffer, sector_t *s, const char *fun) {
  int sector_id;
  
  if (s == NULL) {
    M_PBufWriteInt(savebuffer, 0);
    return;
  }

  sector_id = s->iSectorID;

  if (sector_id < 0)
    I_Error("%s: Invalid sector %d", fun, sector_id);

  if (sector_id > numsectors)
    I_Error("%s: Invalid sector %d", fun, sector_id);

  M_PBufWriteInt(savebuffer, sector_id + 1);
}

void P_UnArchiveSectorIndex(pbuf_t *savebuffer, sector_t **s, const char *f) {
  int sector_id;
  
  M_PBufReadInt(savebuffer, &sector_id);

  if (sector_id < 0)
    I_Error("%s: Invalid sector index %d", f, sector_id);

  if (sector_id == 0) {
    *s = NULL;
    return;
  }

  sector_id--;

  if (sector_id >= numsectors)
    I_Error("%s: Invalid sector index %d", f, sector_id);

  *s = &sectors[sector_id];
}

void P_ArchiveLineIndex(pbuf_t *savebuffer, line_t *li, const char *fun) {
  uint_64_t line_index;

  if (li < lines)
    I_Error("%s: Invalid line %p", fun, li);

  line_index = li - lines;

  if (line_index > numlines)
    I_Error("%s: Invalid line %p", fun, li);

  M_PBufWriteULong(savebuffer, line_index + 1);
}

void P_UnArchiveLineIndex(pbuf_t *savebuffer, line_t **li, const char *fun) {
  uint_64_t line_index;
  
  M_PBufReadULong(savebuffer, &line_index);

  if (line_index == 0)
    I_Error("%s: Invalid line index %" PRIu64, fun, line_index);

  line_index--;

  if (line_index >= numlines)
    I_Error("%s: Invalid line index %" PRIu64, fun, line_index);

  if (line_index == 0)
    *li = NULL;
  else
    *li = &lines[line_index];
}

void P_ArchiveCeiling(pbuf_t *savebuffer, ceiling_t *ceiling) {
  M_PBufWriteInt(savebuffer, ceiling->type);
  P_ArchiveSectorIndex(savebuffer, ceiling->sector, __func__);
  M_PBufWriteInt(savebuffer, ceiling->bottomheight);
  M_PBufWriteInt(savebuffer, ceiling->topheight);
  M_PBufWriteInt(savebuffer, ceiling->speed);
  M_PBufWriteInt(savebuffer, ceiling->oldspeed);
  M_PBufWriteBool(savebuffer, ceiling->crush);
  M_PBufWriteInt(savebuffer, ceiling->newspecial);
  M_PBufWriteInt(savebuffer, ceiling->oldspecial);
  M_PBufWriteShort(savebuffer, ceiling->texture);
  M_PBufWriteInt(savebuffer, ceiling->direction);
  M_PBufWriteInt(savebuffer, ceiling->tag);
  M_PBufWriteInt(savebuffer, ceiling->olddirection);
}

void P_UnArchiveCeiling(pbuf_t *savebuffer, ceiling_t *ceiling) {
  M_PBufReadInt(savebuffer, (int *)&ceiling->type);
  P_UnArchiveSectorIndex(savebuffer, &ceiling->sector, __func__);
  M_PBufReadInt(savebuffer, &ceiling->bottomheight);
  M_PBufReadInt(savebuffer, &ceiling->topheight);
  M_PBufReadInt(savebuffer, &ceiling->speed);
  M_PBufReadInt(savebuffer, &ceiling->oldspeed);
  M_PBufReadBool(savebuffer, &ceiling->crush);
  M_PBufReadInt(savebuffer, &ceiling->newspecial);
  M_PBufReadInt(savebuffer, &ceiling->oldspecial);
  M_PBufReadShort(savebuffer, &ceiling->texture);
  M_PBufReadInt(savebuffer, &ceiling->direction);
  M_PBufReadInt(savebuffer, &ceiling->tag);
  M_PBufReadInt(savebuffer, &ceiling->olddirection);
}

void P_ArchiveDoor(pbuf_t *savebuffer, vldoor_t *door) {
  M_PBufWriteInt(savebuffer, door->type);
  P_ArchiveSectorIndex(savebuffer, door->sector, __func__);
  M_PBufWriteInt(savebuffer, door->topheight);
  M_PBufWriteInt(savebuffer, door->speed);
  M_PBufWriteInt(savebuffer, door->direction);
  M_PBufWriteInt(savebuffer, door->topwait);
  M_PBufWriteInt(savebuffer, door->topcountdown);
  P_ArchiveLineIndex(savebuffer, door->line, __func__);
  M_PBufWriteInt(savebuffer, door->lighttag);
}

void P_UnArchiveDoor(pbuf_t *savebuffer, vldoor_t *door) {
  M_PBufReadInt(savebuffer, (int *)&door->type);
  P_UnArchiveSectorIndex(savebuffer, &door->sector, __func__);
  M_PBufReadInt(savebuffer, &door->topheight);
  M_PBufReadInt(savebuffer, &door->speed);
  M_PBufReadInt(savebuffer, &door->direction);
  M_PBufReadInt(savebuffer, &door->topwait);
  M_PBufReadInt(savebuffer, &door->topcountdown);
  P_UnArchiveLineIndex(savebuffer, &door->line, __func__);
  M_PBufReadInt(savebuffer, &door->lighttag);
}

void P_ArchiveFloor(pbuf_t *savebuffer, floormove_t *floor) {
  M_PBufWriteInt(savebuffer, floor->type);
  M_PBufWriteBool(savebuffer, floor->crush);
  P_ArchiveSectorIndex(savebuffer, floor->sector, __func__);
  M_PBufWriteInt(savebuffer, floor->direction);
  M_PBufWriteInt(savebuffer, floor->newspecial);
  M_PBufWriteInt(savebuffer, floor->oldspecial);
  M_PBufWriteShort(savebuffer, floor->texture);
  M_PBufWriteInt(savebuffer, floor->floordestheight);
  M_PBufWriteInt(savebuffer, floor->speed);
}

void P_UnArchiveFloor(pbuf_t *savebuffer, floormove_t *floor) {
  M_PBufReadInt(savebuffer, (int *)&floor->type);
  M_PBufReadBool(savebuffer, &floor->crush);
  P_UnArchiveSectorIndex(savebuffer, &floor->sector, __func__);
  M_PBufReadInt(savebuffer, &floor->direction);
  M_PBufReadInt(savebuffer, &floor->newspecial);
  M_PBufReadInt(savebuffer, &floor->oldspecial);
  M_PBufReadShort(savebuffer, &floor->texture);
  M_PBufReadInt(savebuffer, &floor->floordestheight);
  M_PBufReadInt(savebuffer, &floor->speed);
}

void P_ArchivePlat(pbuf_t *savebuffer, plat_t *plat) {
  P_ArchiveSectorIndex(savebuffer, plat->sector, __func__);
  M_PBufWriteInt(savebuffer, plat->speed);
  M_PBufWriteInt(savebuffer, plat->low);
  M_PBufWriteInt(savebuffer, plat->high);
  M_PBufWriteInt(savebuffer, plat->wait);
  M_PBufWriteInt(savebuffer, plat->count);
  M_PBufWriteInt(savebuffer, plat->status);
  M_PBufWriteInt(savebuffer, plat->oldstatus);
  M_PBufWriteBool(savebuffer, plat->crush);
  M_PBufWriteInt(savebuffer, plat->tag);
  M_PBufWriteInt(savebuffer, plat->type);
}

void P_UnArchivePlat(pbuf_t *savebuffer, plat_t *plat) {
  P_UnArchiveSectorIndex(savebuffer, &plat->sector, __func__);
  M_PBufReadInt(savebuffer, &plat->speed);
  M_PBufReadInt(savebuffer, &plat->low);
  M_PBufReadInt(savebuffer, &plat->high);
  M_PBufReadInt(savebuffer, &plat->wait);
  M_PBufReadInt(savebuffer, &plat->count);
  M_PBufReadInt(savebuffer, (int *)&plat->status);
  M_PBufReadInt(savebuffer, (int *)&plat->oldstatus);
  M_PBufReadBool(savebuffer, &plat->crush);
  M_PBufReadInt(savebuffer, &plat->tag);
  M_PBufReadInt(savebuffer, (int *)&plat->type);
}

void P_ArchiveLightFlash(pbuf_t *savebuffer, lightflash_t *flash) {
  P_ArchiveSectorIndex(savebuffer, flash->sector, __func__);
  M_PBufWriteInt(savebuffer, flash->count);
  M_PBufWriteInt(savebuffer, flash->maxlight);
  M_PBufWriteInt(savebuffer, flash->minlight);
  M_PBufWriteInt(savebuffer, flash->maxtime);
  M_PBufWriteInt(savebuffer, flash->mintime);
}

void P_UnArchiveLightFlash(pbuf_t *savebuffer, lightflash_t *flash) {
  P_UnArchiveSectorIndex(savebuffer, &flash->sector, __func__);
  M_PBufReadInt(savebuffer, &flash->count);
  M_PBufReadInt(savebuffer, &flash->maxlight);
  M_PBufReadInt(savebuffer, &flash->minlight);
  M_PBufReadInt(savebuffer, &flash->maxtime);
  M_PBufReadInt(savebuffer, &flash->mintime);
}

void P_ArchiveStrobe(pbuf_t *savebuffer, strobe_t *strobe) {
  P_ArchiveSectorIndex(savebuffer, strobe->sector, __func__);
  M_PBufWriteInt(savebuffer, strobe->count);
  M_PBufWriteInt(savebuffer, strobe->minlight);
  M_PBufWriteInt(savebuffer, strobe->maxlight);
  M_PBufWriteInt(savebuffer, strobe->darktime);
  M_PBufWriteInt(savebuffer, strobe->brighttime);
}

void P_UnArchiveStrobe(pbuf_t *savebuffer, strobe_t *strobe) {
  P_UnArchiveSectorIndex(savebuffer, &strobe->sector, __func__);
  M_PBufReadInt(savebuffer, &strobe->count);
  M_PBufReadInt(savebuffer, &strobe->minlight);
  M_PBufReadInt(savebuffer, &strobe->maxlight);
  M_PBufReadInt(savebuffer, &strobe->darktime);
  M_PBufReadInt(savebuffer, &strobe->brighttime);
}

void P_ArchiveGlow(pbuf_t *savebuffer, glow_t *glow) {
  P_ArchiveSectorIndex(savebuffer, glow->sector, __func__);
  M_PBufWriteInt(savebuffer, glow->maxlight);
  M_PBufWriteInt(savebuffer, glow->minlight);
  M_PBufWriteInt(savebuffer, glow->direction);
}

void P_UnArchiveGlow(pbuf_t *savebuffer, glow_t *glow) {
  P_UnArchiveSectorIndex(savebuffer, &glow->sector, __func__);
  M_PBufReadInt(savebuffer, &glow->maxlight);
  M_PBufReadInt(savebuffer, &glow->minlight);
  M_PBufReadInt(savebuffer, &glow->direction);
}

void P_ArchiveFireFlicker(pbuf_t *savebuffer, fireflicker_t *flicker) {
  P_ArchiveSectorIndex(savebuffer, flicker->sector, __func__);
  M_PBufWriteInt(savebuffer, flicker->count);
  M_PBufWriteInt(savebuffer, flicker->maxlight);
  M_PBufWriteInt(savebuffer, flicker->minlight);
}

void P_UnArchiveFireFlicker(pbuf_t *savebuffer, fireflicker_t *flicker) {
  P_UnArchiveSectorIndex(savebuffer, &flicker->sector, __func__);
  M_PBufReadInt(savebuffer, &flicker->count);
  M_PBufReadInt(savebuffer, &flicker->maxlight);
  M_PBufReadInt(savebuffer, &flicker->minlight);
}

void P_ArchiveElevator(pbuf_t *savebuffer, elevator_t *elevator) {
  M_PBufWriteInt(savebuffer, elevator->type);
  P_ArchiveSectorIndex(savebuffer, elevator->sector, __func__);
  M_PBufWriteInt(savebuffer, elevator->direction);
  M_PBufWriteInt(savebuffer, elevator->floordestheight);
  M_PBufWriteInt(savebuffer, elevator->ceilingdestheight);
  M_PBufWriteInt(savebuffer, elevator->speed);
}

void P_UnArchiveElevator(pbuf_t *savebuffer, elevator_t *elevator) {
  M_PBufReadInt(savebuffer, (int *)&elevator->type);
  P_UnArchiveSectorIndex(savebuffer, &elevator->sector, __func__);
  M_PBufReadInt(savebuffer, &elevator->direction);
  M_PBufReadInt(savebuffer, &elevator->floordestheight);
  M_PBufReadInt(savebuffer, &elevator->ceilingdestheight);
  M_PBufReadInt(savebuffer, &elevator->speed);
}

void P_ArchiveScroll(pbuf_t *savebuffer, scroll_t *scroll) {
  M_PBufWriteInt(savebuffer, scroll->dx);
  M_PBufWriteInt(savebuffer, scroll->dy);
  M_PBufWriteInt(savebuffer, scroll->affectee);
  M_PBufWriteInt(savebuffer, scroll->control);
  M_PBufWriteInt(savebuffer, scroll->last_height);
  M_PBufWriteInt(savebuffer, scroll->vdx);
  M_PBufWriteInt(savebuffer, scroll->vdy);
  M_PBufWriteInt(savebuffer, scroll->accel);
  M_PBufWriteInt(savebuffer, scroll->type);
}

void P_UnArchiveScroll(pbuf_t *savebuffer, scroll_t *scroll) {
  M_PBufReadInt(savebuffer, &scroll->dx);
  M_PBufReadInt(savebuffer, &scroll->dy);
  M_PBufReadInt(savebuffer, &scroll->affectee);
  M_PBufReadInt(savebuffer, &scroll->control);
  M_PBufReadInt(savebuffer, &scroll->last_height);
  M_PBufReadInt(savebuffer, &scroll->vdx);
  M_PBufReadInt(savebuffer, &scroll->vdy);
  M_PBufReadInt(savebuffer, &scroll->accel);
  M_PBufReadInt(savebuffer, (int *)&scroll->type);
}

void P_ArchivePusher(pbuf_t *savebuffer, pusher_t *pusher) {
  M_PBufWriteInt(savebuffer, pusher->type);
  M_PBufWriteInt(savebuffer, pusher->x_mag);
  M_PBufWriteInt(savebuffer, pusher->y_mag);
  M_PBufWriteInt(savebuffer, pusher->magnitude);
  M_PBufWriteInt(savebuffer, pusher->radius);
  M_PBufWriteInt(savebuffer, pusher->x);
  M_PBufWriteInt(savebuffer, pusher->y);
  M_PBufWriteInt(savebuffer, pusher->affectee);
}

void P_UnArchivePusher(pbuf_t *savebuffer, pusher_t *pusher) {
  M_PBufReadInt(savebuffer, (int *)&pusher->type);
  M_PBufReadInt(savebuffer, &pusher->x_mag);
  M_PBufReadInt(savebuffer, &pusher->y_mag);
  M_PBufReadInt(savebuffer, &pusher->magnitude);
  M_PBufReadInt(savebuffer, &pusher->radius);
  M_PBufReadInt(savebuffer, &pusher->x);
  M_PBufReadInt(savebuffer, &pusher->y);
  M_PBufReadInt(savebuffer, &pusher->affectee);
}

void P_ArchiveSpecials(pbuf_t *savebuffer) {
  thinker_t *th;

  // save off the current thinkers
  for (th = thinkercap.next; th != &thinkercap; th = th->next) {
    if (!th->function) {
      platlist_t *pl;
      ceilinglist_t *cl;    //jff 2/22/98 add iter variable for ceilings

      // killough 2/8/98: fix plat original height bug.
      // Since acv==NULL, this could be a plat in stasis.
      // so check the active plats list, and save this
      // plat (jff: or ceiling) even if it is in stasis.

      for (pl = activeplats; pl; pl = pl->next)
        if (pl->plat == (plat_t *)th)      // killough 2/14/98
          goto plat;

      for (cl = activeceilings; cl; cl = cl->next)
        if (cl->ceiling == (ceiling_t *)th)      //jff 2/22/98
          goto ceiling;

      continue;
    }

    if (th->function == T_MoveCeiling) {
      ceiling_t *ceiling = NULL;

      ceiling:                               // killough 2/14/98

      ceiling = (ceiling_t *)th;

      M_PBufWriteInt(savebuffer, tc_ceiling);
      P_ArchiveCeiling(savebuffer, ceiling);

      continue;
    }

    if (th->function == T_VerticalDoor) {
      vldoor_t *door = (vldoor_t *)th;

      M_PBufWriteInt(savebuffer, tc_door);
      P_ArchiveDoor(savebuffer, door);

      continue;
    }

    if (th->function == T_MoveFloor) {
      floormove_t *floor = (floormove_t *)th;

      M_PBufWriteInt(savebuffer, tc_floor);
      P_ArchiveFloor(savebuffer, floor);

      continue;
    }

    if (th->function == T_PlatRaise) {
      plat_t *plat = NULL;

      plat:   // killough 2/14/98: added fix for original plat height above

      plat = (plat_t *)th;

      M_PBufWriteInt(savebuffer, tc_plat);
      P_ArchivePlat(savebuffer, plat);

      continue;
    }

    if (th->function == T_LightFlash) {
      lightflash_t *flash = (lightflash_t *)th;

      M_PBufWriteInt(savebuffer, tc_flash);
      P_ArchiveLightFlash(savebuffer, flash);

      continue;
    }

    if (th->function == T_StrobeFlash) {
      strobe_t *strobe = (strobe_t *)th;

      M_PBufWriteInt(savebuffer, tc_strobe);
      P_ArchiveStrobe(savebuffer, strobe);

      continue;
    }

    if (th->function == T_Glow) {
      glow_t *glow = (glow_t *)th;

      M_PBufWriteInt(savebuffer, tc_glow);
      P_ArchiveGlow(savebuffer, glow);

      continue;
    }

    // killough 10/4/98: save flickers
    if (th->function == T_FireFlicker) {
      fireflicker_t *flicker = (fireflicker_t *)th;

      M_PBufWriteInt(savebuffer, tc_flicker);
      P_ArchiveFireFlicker(savebuffer, flicker);

      continue;
    }

    //jff 2/22/98 new case for elevators
    if (th->function == T_MoveElevator) {
      elevator_t *elevator = (elevator_t *)th;         //jff 2/22/98

      M_PBufWriteInt(savebuffer, tc_elevator);
      P_ArchiveElevator(savebuffer, elevator);

      continue;
    }

    // killough 3/7/98: Scroll effect thinkers
    if (th->function == T_Scroll) {
      scroll_t *scroll = (scroll_t *)th;

      M_PBufWriteInt(savebuffer, tc_scroll);
      P_ArchiveScroll(savebuffer, scroll);

      continue;
    }

    // phares 3/22/98: Push/Pull effect thinkers
    if (th->function == T_Pusher) {
      pusher_t *pusher = (pusher_t *)th;

      M_PBufWriteInt(savebuffer, tc_pusher);
      P_ArchivePusher(savebuffer, pusher);

      continue;
    }
  }

  // add a terminating marker
  M_PBufWriteInt(savebuffer, tc_endspecials);
}

//
// P_UnArchiveSpecials
//
void P_UnArchiveSpecials(pbuf_t *savebuffer) {
  int tclass;

  M_PBufReadInt(savebuffer, &tclass);

  // read in saved thinkers
  // killough 2/14/98
  for (; tclass != tc_endspecials; M_PBufReadInt(savebuffer, &tclass)) {
    switch (tclass) {
      case tc_ceiling:
      {
        ceiling_t *ceiling = Z_Malloc(sizeof(*ceiling), PU_LEVEL, NULL);

        P_UnArchiveCeiling(savebuffer, ceiling);
        // ceiling->sector = &sectors[(int)ceiling->sector];
        ceiling->sector->ceilingdata = ceiling; //jff 2/22/98

        /*
        if (ceiling->thinker.function)
          ceiling->thinker.function = T_MoveCeiling;
        */
        ceiling->thinker.function = T_MoveCeiling;

        P_AddThinker(&ceiling->thinker);
        P_AddActiveCeiling(ceiling);
      }
      break;
      case tc_door:
      {
        vldoor_t *door = Z_Malloc(sizeof(*door), PU_LEVEL, NULL);

        P_UnArchiveDoor(savebuffer, door);
        // door->sector = &sectors[(int)door->sector];

        //jff 1/31/98 unarchive line remembered by door as well
        // door->line = (int)door->line != -1 ? &lines[(int)door->line] : NULL;

        door->sector->ceilingdata = door;       //jff 2/22/98
        door->thinker.function = T_VerticalDoor;
        P_AddThinker(&door->thinker);
      }
      break;
      case tc_floor:
      {
        floormove_t *floor = Z_Malloc (sizeof(*floor), PU_LEVEL, NULL);

        P_UnArchiveFloor(savebuffer, floor);
        // floor->sector = &sectors[(int)floor->sector];
        floor->sector->floordata = floor; //jff 2/22/98
        floor->thinker.function = T_MoveFloor;
        P_AddThinker(&floor->thinker);
      }
      break;
      case tc_plat:
      {
        plat_t *plat = Z_Malloc(sizeof(*plat), PU_LEVEL, NULL);

        P_UnArchivePlat(savebuffer, plat);
        // plat->sector = &sectors[(int)plat->sector];
        plat->sector->floordata = plat; //jff 2/22/98

        /*
        if (plat->thinker.function)
          plat->thinker.function = T_PlatRaise;
        */

        plat->thinker.function = T_PlatRaise;
        P_AddThinker(&plat->thinker);
        P_AddActivePlat(plat);
      }
      break;
      case tc_flash:
      {
        lightflash_t *flash = Z_Malloc(sizeof(*flash), PU_LEVEL, NULL);

        P_UnArchiveLightFlash(savebuffer, flash);
        // flash->sector = &sectors[(int)flash->sector];
        flash->thinker.function = T_LightFlash;
        P_AddThinker(&flash->thinker);
      }
      break;
      case tc_strobe:
      {
        strobe_t *strobe = Z_Malloc(sizeof(*strobe), PU_LEVEL, NULL);

        P_UnArchiveStrobe(savebuffer, strobe);
        // strobe->sector = &sectors[(int)strobe->sector];
        strobe->thinker.function = T_StrobeFlash;
        P_AddThinker(&strobe->thinker);
      }
      break;
      case tc_glow:
      {
        glow_t *glow = Z_Malloc(sizeof(*glow), PU_LEVEL, NULL);

        P_UnArchiveGlow(savebuffer, glow);
        // glow->sector = &sectors[(int)glow->sector];
        glow->thinker.function = T_Glow;
        P_AddThinker(&glow->thinker);
      }
      break;
      case tc_flicker:           // killough 10/4/98
      {
        fireflicker_t *flicker = Z_Malloc(sizeof(*flicker), PU_LEVEL, NULL);

        P_UnArchiveFireFlicker(savebuffer, flicker);
        // flicker->sector = &sectors[(int)flicker->sector];
        flicker->thinker.function = T_FireFlicker;
        P_AddThinker(&flicker->thinker);
      }
      break;
      case tc_elevator: //jff 2/22/98 new case for elevators
      {
        elevator_t *elevator = Z_Malloc(sizeof(*elevator), PU_LEVEL, NULL);

        P_UnArchiveElevator(savebuffer, elevator);
        // elevator->sector = &sectors[(int)elevator->sector];
        elevator->sector->floordata = elevator; //jff 2/22/98
        elevator->sector->ceilingdata = elevator; //jff 2/22/98
        elevator->thinker.function = T_MoveElevator;
        P_AddThinker(&elevator->thinker);
      }
      break;
      case tc_scroll:       // killough 3/7/98: scroll effect thinkers
      {
        scroll_t *scroll = Z_Malloc(sizeof(scroll_t), PU_LEVEL, NULL);

        P_UnArchiveScroll(savebuffer, scroll);
        scroll->thinker.function = T_Scroll;
        P_AddThinker(&scroll->thinker);
      }
      break;
      case tc_pusher:   // phares 3/22/98: new Push/Pull effect thinkers
      {
        pusher_t *pusher = Z_Malloc(sizeof(pusher_t), PU_LEVEL, NULL);

        P_UnArchivePusher(savebuffer, pusher);
        pusher->thinker.function = T_Pusher;
        pusher->source = P_GetPushThing(pusher->affectee);
        P_AddThinker(&pusher->thinker);
      }
      break;
      default:
        I_Error("P_UnarchiveSpecials: Unknown tclass %i in savegame", tclass);
    }
  }
}

// killough 2/16/98: save/restore random number generator state information
void P_ArchiveRNG(pbuf_t *savebuffer) {
  M_PBufWriteArray(savebuffer, NUMPRCLASS);

  for (int i = 0; i < NUMPRCLASS; i++)
    M_PBufWriteUInt(savebuffer, rng.seed[i]);

  M_PBufWriteInt(savebuffer, rng.rndindex);
  M_PBufWriteInt(savebuffer, rng.prndindex);
}

void P_UnArchiveRNG(pbuf_t *savebuffer) {
  unsigned int seed_count;

  M_PBufReadArray(savebuffer, &seed_count);

  if (seed_count != NUMPRCLASS) {
    I_Error(
      "P_UnArchiveRNG: RNG seed count (%u) != %u", seed_count, NUMPRCLASS
    );
  }

  for (int i = 0; i < NUMPRCLASS; i++)
    M_PBufReadUInt(savebuffer, &rng.seed[i]);

  M_PBufReadInt(savebuffer, &rng.rndindex);
  M_PBufReadInt(savebuffer, &rng.prndindex);
}

// killough 2/22/98: Save/restore automap state
void P_ArchiveMap(pbuf_t *savebuffer) {
  M_PBufWriteInt(savebuffer, automapmode);

  M_PBufWriteInt(savebuffer, markpointnum);

  for (int i = 0; i < markpointnum; i++) {
    M_PBufWriteInt(savebuffer, markpoints[i].x);
    M_PBufWriteInt(savebuffer, markpoints[i].y);
    M_PBufWriteInt(savebuffer, markpoints[i].w);
    M_PBufWriteInt(savebuffer, markpoints[i].h);
    for (int j = 0; j < 16; j++)
      M_PBufWriteChar(savebuffer, markpoints[i].label[j]);
    for (int j = 0; j < 16; j++)
      M_PBufWriteInt(savebuffer, markpoints[i].widths[j]);
  }
}

void P_UnArchiveMap(pbuf_t *savebuffer) {
  if (DELTACLIENT) {
    int count;
    int m_int;
    char m_char;

    M_PBufReadInt(savebuffer, &m_int);       // automapmode
    M_PBufReadInt(savebuffer, &count);       // markpointnum

    for (int i = 0; i < count; i++) {
      M_PBufReadInt(savebuffer, &m_int);     // &markpoints[i].x);
      M_PBufReadInt(savebuffer, &m_int);     // &markpoints[i].y);
      M_PBufReadInt(savebuffer, &m_int);     // &markpoints[i].w);
      M_PBufReadInt(savebuffer, &m_int);     // &markpoints[i].h);
      for (int j = 0; j < 16; j++)
        M_PBufReadChar(savebuffer, &m_char); // &markpoints[i].label[j]);
      for (int j = 0; j < 16; j++)
        M_PBufReadInt(savebuffer, &m_int);   // &markpoints[i].widths[j]);
    }

    return;
  }

  M_PBufReadInt(savebuffer, (int *)&automapmode);

  if (automapmode & am_active)
    AM_Start();

  M_PBufReadInt(savebuffer, &markpointnum);

  if (markpointnum_max < markpointnum) {
    while (markpointnum_max < markpointnum) {
      markpointnum_max = MAX(8, markpointnum_max);
      markpointnum *= 2;
    }
    markpoints = realloc(markpoints, sizeof(markpoint_t) * markpointnum_max);
  }

  for (int i = 0; i < markpointnum; i++) {
    M_PBufReadInt(savebuffer, &markpoints[i].x);
    M_PBufReadInt(savebuffer, &markpoints[i].y);
    M_PBufReadInt(savebuffer, &markpoints[i].w);
    M_PBufReadInt(savebuffer, &markpoints[i].h);
    for (int j = 0; j < 16; j++)
      M_PBufReadChar(savebuffer, &markpoints[i].label[j]);
    for (int j = 0; j < 16; j++)
      M_PBufReadInt(savebuffer, &markpoints[i].widths[j]);

    AM_setMarkParams(i);
  }
}

/* vi: set et ts=2 sw=2: */

