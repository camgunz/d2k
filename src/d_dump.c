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

#include "m_delta.h"
#include "m_random.h"
#include "am_map.h"
#include "g_comp.h"
#include "g_demo.h"
#include "g_game.h"
#include "p_defs.h"
#include "p_enemy.h"
#include "p_ident.h"
#include "p_map.h"
#include "p_mobj.h"
#include "p_spec.h"
#include "p_tick.h"
#include "pl_main.h"
#include "r_defs.h"
#include "s_advsound.h"

extern int forceOldBsp;
extern int numspechit;

static const char *dump_file_name = NULL;
static pbuf_t     *dump_buf1      = NULL;
static pbuf_t     *dump_buf2      = NULL;
static buf_t      *delta_buf      = NULL;
static pbuf_t     *size_buf       = NULL;
static int         dump_fd        = -1;
static FILE       *dump_fobj      = NULL;
static uint32_t    state_count    = 0;

static void close_dump_file(void) {
  fflush(dump_fobj);
  fclose(dump_fobj);
  close(dump_fd);
}

static void dump_sector_index(pbuf_t *savebuffer, sector_t *s, const char *fn) {
  int sector_id;

  if (!s) {
    M_PBufWriteInt(savebuffer, 0);
    return;
  }

  sector_id = s->iSectorID;

  if (sector_id < 0) {
    I_Error("%s: Invalid sector %d", fn, sector_id);
  }

  if (sector_id > numsectors) {
    I_Error("%s: Invalid sector %d", fn, sector_id);
  }

  M_PBufWriteInt(savebuffer, sector_id + 1);
}

static void dump_line_index(pbuf_t *savebuffer, line_t *li, const char *fn) {
  uint64_t line_index;

  if (li < lines) {
    // I_Error("%s: Invalid line %p < %p", fn, li, lines);
    line_index = 0;
  }
  else {
    line_index = li - lines;
  }

  if (line_index > numlines) {
    // I_Error("%s: Invalid line %p", fn, li);
    line_index = 0;
  }

  M_PBufWriteULong(savebuffer, line_index + 1);
}

void D_Dump(pbuf_t *savebuffer) {
  unsigned int actor_count = 0;
  thinker_t *th = NULL;
  unsigned int target_id = 0;
  unsigned int tracer_id = 0;
  unsigned int lastenemy_id = 0;
  uint64_t     state_index = 0;
  unsigned int player_index = 0;

  /** Config **/

  if (mbf_features) {
    switch(compatibility_level) {
      case mbf_compatibility:
        M_PBufWriteInt(savebuffer, 203);
        break;
      case prboom_2_compatibility:
        M_PBufWriteInt(savebuffer, 210);
        break;
      case prboom_3_compatibility:
        M_PBufWriteInt(savebuffer, 211);
        break;
      case prboom_4_compatibility:
        M_PBufWriteInt(savebuffer, 212);
        break;
      case prboom_5_compatibility:
        M_PBufWriteInt(savebuffer, 213);
        break;
      case prboom_6_compatibility:
        M_PBufWriteInt(savebuffer, 214);
        break;
      default:
        I_Error("D_Dump: PrBoom compatibility level unrecognized?");
        break;
    }

    M_PBufWriteInt(savebuffer, gameskill);
    M_PBufWriteInt(savebuffer, gameepisode);
    M_PBufWriteInt(savebuffer, gamemap);
    M_PBufWriteInt(savebuffer, deathmatch);
    M_PBufWriteInt(savebuffer, P_GetConsolePlayer()->id - 1);
    M_PBufWriteInt(savebuffer, monsters_remember);
    M_PBufWriteInt(savebuffer, variable_friction);
    M_PBufWriteInt(savebuffer, weapon_recoil);
    M_PBufWriteInt(savebuffer, allow_pushers);
    M_PBufWriteInt(savebuffer, player_bobbing);
    /* [CG] respawnparm is > 1 in some demos */
    M_PBufWriteInt(savebuffer, respawnparm ? 1 : 0);
    /* [CG] fastparm is > 1 in some demos */
    M_PBufWriteInt(savebuffer, fastparm ? 1 : 0);
    /* [CG] nomonsters is > 1 in some demos */
    M_PBufWriteInt(savebuffer, nomonsters ? 1 : 0);
    M_PBufWriteInt(savebuffer, demo_insurance);
    M_PBufWriteUInt(savebuffer, rngseed);
    M_PBufWriteInt(savebuffer, monster_infighting);
#ifdef DOGS
    M_PBufWriteInt(savebuffer, dogs);
#else
    M_PBufWriteInt(savebuffer, 0);
#endif

    M_PBufWriteInt(savebuffer, distfriend);
    M_PBufWriteInt(savebuffer, monster_backing);
    M_PBufWriteInt(savebuffer, monster_avoid_hazards);
    M_PBufWriteInt(savebuffer, monster_friction);
    M_PBufWriteInt(savebuffer, help_friends);

#ifdef DOGS
    M_PBufWriteInt(savebuffer, dog_jumping);
#else
    M_PBufWriteInt(savebuffer, 0);
#endif

    M_PBufWriteInt(savebuffer, monkeys);

    for (int i = 0; i < COMP_TOTAL; i++) {
      M_PBufWriteInt(savebuffer, comp[i]);
    }

    if ((compatibility_level >= prboom_2_compatibility) && forceOldBsp) {
      M_PBufWriteInt(savebuffer, 1);
    }
    else {
      M_PBufWriteInt(savebuffer, 0);
    }
  }
  else if (compatibility_level > boom_compatibility_compatibility) {
    switch (compatibility_level) {
      case boom_compatibility_compatibility:
        M_PBufWriteInt(savebuffer, 202);
        M_PBufWriteInt(savebuffer, 1);
        break;
      case boom_201_compatibility:
        M_PBufWriteInt(savebuffer, 201);
        M_PBufWriteInt(savebuffer, 0);
        break;
      case boom_202_compatibility:
        M_PBufWriteInt(savebuffer, 202);
        M_PBufWriteInt(savebuffer, 0);
        break;
      default:
        I_Error("D_Dump: Boom compatibility level unrecognized?");
        break;
    }

    M_PBufWriteInt(savebuffer, gameskill);
    M_PBufWriteInt(savebuffer, gameepisode);
    M_PBufWriteInt(savebuffer, gamemap);
    M_PBufWriteInt(savebuffer, deathmatch);
    M_PBufWriteInt(savebuffer, P_GetConsolePlayer()->id);
    M_PBufWriteInt(savebuffer, monsters_remember);
    M_PBufWriteInt(savebuffer, variable_friction);
    M_PBufWriteInt(savebuffer, weapon_recoil);
    M_PBufWriteInt(savebuffer, allow_pushers);
    M_PBufWriteInt(savebuffer, player_bobbing);
    /* [CG] respawnparm is > 1 in some demos */
    M_PBufWriteInt(savebuffer, respawnparm ? 1 : 0);
    /* [CG] fastparm is > 1 in some demos */
    M_PBufWriteInt(savebuffer, fastparm ? 1 : 0);
    /* [CG] nomonsters is > 1 in some demos */
    M_PBufWriteInt(savebuffer, nomonsters ? 1 : 0);
    M_PBufWriteInt(savebuffer, demo_insurance);
    M_PBufWriteUInt(savebuffer, rngseed);
    M_PBufWriteInt(savebuffer, monster_infighting);
#ifdef DOGS
    M_PBufWriteInt(savebuffer, dogs);
#else
    M_PBufWriteInt(savebuffer, 0);
#endif

    M_PBufWriteInt(savebuffer, distfriend);
    M_PBufWriteInt(savebuffer, monster_backing);
    M_PBufWriteInt(savebuffer, monster_avoid_hazards);
    M_PBufWriteInt(savebuffer, monster_friction);
    M_PBufWriteInt(savebuffer, help_friends);

#ifdef DOGS
    M_PBufWriteInt(savebuffer, dog_jumping);
#else
    M_PBufWriteInt(savebuffer, 0);
#endif

    M_PBufWriteInt(savebuffer, monkeys);

    for (int i = 0; i < COMP_TOTAL; i++) {
      M_PBufWriteInt(savebuffer, comp[i]);
    }

    if ((compatibility_level >= prboom_2_compatibility) && forceOldBsp) {
      M_PBufWriteInt(savebuffer, 1);
    }
    else {
      M_PBufWriteInt(savebuffer, 0);
    }
  }
  else {
    if (longtics) {
      M_PBufWriteInt(savebuffer, 111);
    }
    else if (compatibility_level == doom_1666_compatibility) {
      M_PBufWriteInt(savebuffer, 106);
    }
    else if (compatibility_level == tasdoom_compatibility) {
      M_PBufWriteInt(savebuffer, 110);
    }
    else {
      M_PBufWriteInt(savebuffer, 109);
    }

    M_PBufWriteInt(savebuffer, gameskill);
    M_PBufWriteInt(savebuffer, gameepisode);
    M_PBufWriteInt(savebuffer, gamemap);
    M_PBufWriteInt(savebuffer, deathmatch);
    /* [CG] respawnparm is > 1 in some demos */
    M_PBufWriteInt(savebuffer, respawnparm ? 1 : 0);
    /* [CG] fastparm is > 1 in some demos */
    M_PBufWriteInt(savebuffer, fastparm ? 1 : 0);
    /* [CG] nomonsters is > 1 in some demos */
    M_PBufWriteInt(savebuffer, nomonsters ? 1 : 0);
    M_PBufWriteInt(savebuffer, P_GetConsolePlayer()->id);
  }

  for (int i = 0; i < 5; i++) {
    M_PBufWriteInt(savebuffer, default_comperr[i]);
  }

  /** Misc **/

  M_PBufWriteInt(savebuffer, gametic);
  M_PBufWriteInt(savebuffer, leveltime);
  M_PBufWriteInt(savebuffer, totalleveltimes);
  // killough 11/98: save revenant tracer state
  M_PBufWriteInt(savebuffer, basetic);

  for (int i = 0; i < VANILLA_MAXPLAYERS; i++) {
    M_PBufWriteInt(savebuffer, playerstarts[i].x);
    M_PBufWriteInt(savebuffer, playerstarts[i].y);
    M_PBufWriteInt(savebuffer, playerstarts[i].angle);
    M_PBufWriteInt(savebuffer, playerstarts[i].type);
    M_PBufWriteInt(savebuffer, playerstarts[i].options);
  }

  M_PBufWriteInt(savebuffer, deathmatch_p - deathmatchstarts);

  for (int i = 0; i < (deathmatch_p - deathmatchstarts); i++) {
    M_PBufWriteInt(savebuffer, deathmatchstarts[i].x);
    M_PBufWriteInt(savebuffer, deathmatchstarts[i].y);
    M_PBufWriteInt(savebuffer, deathmatchstarts[i].angle);
    M_PBufWriteInt(savebuffer, deathmatchstarts[i].type);
    M_PBufWriteInt(savebuffer, deathmatchstarts[i].options);
  }

  /** RNG **/

  M_PBufWriteInt(savebuffer, rng.rndindex);
  M_PBufWriteInt(savebuffer, rng.prndindex);

  /** Players **/

  for (uint32_t i = 1; i <= VANILLA_MAXPLAYERS; i++) {
    if (P_PlayersLookup(i)) {
      M_PBufWriteInt(savebuffer, 1);
    }
    else {
      M_PBufWriteInt(savebuffer, 0);
    }
  }

  for (int i = VANILLA_MAXPLAYERS; i < MIN_MAXPLAYERS; i++) {
    M_PBufWriteInt(savebuffer, 0);
  }

  for (uint32_t i = 1; i <= VANILLA_MAXPLAYERS; i++) {
    player_t *player = P_PlayersLookup(i);

    if (player) {
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
      for (int j = 0; j < NUMPOWERS; j++)
        M_PBufWriteInt(savebuffer, player->powers[i]);
      for (int j = 0; j < NUMCARDS; j++)
        M_PBufWriteInt(savebuffer, player->cards[i]);
      M_PBufWriteInt(savebuffer, player->backpack);

      for (uint32_t j = 1; j <= VANILLA_MAXPLAYERS; j++) {
        if (i != j) {
          M_PBufWriteInt(savebuffer, PL_GetFrags(player, P_PlayersLookup(j)));
        }
        else {
          M_PBufWriteInt(savebuffer, (
            PL_GetTotalMurders(player) +
            PL_GetTotalDeaths(player) +
            PL_GetTotalSuicides(player)
          ));
        }
      }

      M_PBufWriteInt(savebuffer, player->readyweapon);
      M_PBufWriteInt(savebuffer, player->pendingweapon);
      for (int j = 0; j < NUMWEAPONS; j++)
        M_PBufWriteInt(savebuffer, player->weaponowned[j]);
      for (int j = 0; j < NUMAMMO; j++)
        M_PBufWriteInt(savebuffer, player->ammo[j]);
      for (int j = 0; j < NUMAMMO; j++)
        M_PBufWriteInt(savebuffer, player->maxammo[j]);

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
      for (int j = 0; j < NUMPSPRITES; j++) {
        if (player->psprites[j].state) {
          uint64_t state_index;

          if (player->psprites[j].state < states) {
            I_Error(
              "D_Dump: Invalid psprite state %p", player->psprites[j].state
            );
          }

          state_index = player->psprites[j].state - states;

          if (state_index > NUMSTATES) {
            I_Error(
              "D_Dump: Invalid psprite state %p", player->psprites[j].state
            );
          }

          M_PBufWriteULong(savebuffer, state_index + 1);
          M_PBufWriteInt(savebuffer, player->psprites[j].tics);
          M_PBufWriteInt(savebuffer, player->psprites[j].sx);
          M_PBufWriteInt(savebuffer, player->psprites[j].sy);
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
      // M_PBufWriteInt(savebuffer, player->jumpTics);
    }
  }

  /** World **/

  M_PBufWriteInt(savebuffer, numspechit);
  M_PBufWriteUInt(savebuffer, numsectors);
  M_PBufWriteUInt(savebuffer, numlines);

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
  }

  for (line_t *li = lines; (li - lines) < numlines; li++) {
    // M_PBufWriteShort(savebuffer, li->flags);
    M_PBufWriteShort(savebuffer, li->special);
    M_PBufWriteShort(savebuffer, li->tag);

    for (int j = 0; j < 2; j++) {
      M_PBufWriteUChar(savebuffer, j);

      if (li->sidenum[j] != NO_INDEX) {
        side_t *si = &sides[li->sidenum[j]];

        M_PBufWriteInt(savebuffer, 1);
        // killough 10/98: save full sidedef offsets,
        // preserving fractional scroll offsets
        M_PBufWriteInt(savebuffer, si->textureoffset);
        M_PBufWriteInt(savebuffer, si->rowoffset);
        M_PBufWriteShort(savebuffer, si->toptexture);
        M_PBufWriteShort(savebuffer, si->bottomtexture);
        M_PBufWriteShort(savebuffer, si->midtexture);
      }
      else {
        M_PBufWriteInt(savebuffer, 0);
      }
    }

    M_PBufWriteInt(savebuffer, li->soundorg.x);
    M_PBufWriteInt(savebuffer, li->soundorg.y);
    M_PBufWriteInt(savebuffer, li->soundorg.z);
  }

  M_PBufWriteInt(savebuffer, musinfo.current_item);

  /** Actors **/

  M_PBufWriteUInt(savebuffer, P_IdentGetMaxID());

  // killough 3/26/98: Save boss brain state
  M_PBufWriteInt(savebuffer, brain.easy);
  M_PBufWriteInt(savebuffer, brain.targeton);

  for (thinker_t *th = thinkercap.next; th != &thinkercap; th = th->next) {
    if (th->function == P_MobjThinker) {
      actor_count++;
    }
  }

  M_PBufWriteUInt(savebuffer, actor_count);

  for (thinker_t *th = thinkercap.next; th != &thinkercap; th = th->next) {
    mobj_t *mobj;

    if (th->function != P_MobjThinker) {
      continue;
    }

    mobj = (mobj_t *)th;

    M_PBufWriteInt(savebuffer, mobj->type);
    M_PBufWriteInt(savebuffer, mobj->index);
    M_PBufWriteInt(savebuffer, mobj->x);
    M_PBufWriteInt(savebuffer, mobj->y);
    M_PBufWriteInt(savebuffer, mobj->z);
    M_PBufWriteUInt(savebuffer, mobj->angle);
    M_PBufWriteUInt(savebuffer, mobj->pitch);
    M_PBufWriteUInt(savebuffer, mobj->id);
    M_PBufWriteInt(savebuffer, mobj->tics);
    M_PBufWriteULong(savebuffer, mobj->flags);

    M_PBufWriteInt(savebuffer, mobj->floorz);
    M_PBufWriteInt(savebuffer, mobj->ceilingz);
    M_PBufWriteInt(savebuffer, mobj->dropoffz);
    M_PBufWriteInt(savebuffer, mobj->momx);
    M_PBufWriteInt(savebuffer, mobj->momy);
    M_PBufWriteInt(savebuffer, mobj->momz);
    M_PBufWriteInt(savebuffer, mobj->validcount);
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
    // M_PBufWriteShort(savebuffer, mobj->patch_width);
    M_PBufWriteInt(savebuffer, mobj->iden_nums);
    /* CG 06/26/2014: Try not storing indices in pointers */

    // killough 2/14/98: convert pointers into indices.
    // Fixes many savegame problems, by properly saving
    // target and tracer fields. Note: we store NULL if
    // the thinker pointed to by these fields is not a
    // mobj thinker.

    if (mobj->target && mobj->target->thinker.function == P_MobjThinker) {
      target_id = mobj->target->id;
    }

    if (mobj->tracer && mobj->tracer->thinker.function == P_MobjThinker) {
      tracer_id = mobj->tracer->id;
    }

    // killough 2/14/98: new field: save last known enemy. Prevents
    // monsters from going to sleep after killing monsters and not
    // seeing player anymore.

    if (mobj->lastenemy && mobj->lastenemy->thinker.function == P_MobjThinker) {
      lastenemy_id = mobj->lastenemy->id;
    }

    // killough 2/14/98: end changes

    if (mobj->state < states) {
      I_Error("D_Dump: Invalid mobj state %p", mobj->state);
    }

    state_index = (uint64_t)(mobj->state - states);

    if (state_index >= NUMSTATES) {
      I_Error("D_Dump: Invalid mobj state %p", mobj->state);
    }

    state_index++;

    if (mobj->player) {
      for (unsigned int i = 0; i < VANILLA_MAXPLAYERS; i++) {
        if (mobj->player == P_PlayersLookup(i + 1)) {
          player_index = i + 1;
          break;
        }
      }
      if (player_index == 0) {
        I_Error("D_Dump: Invalid mobj player %p", mobj->player);
      }
    }

    M_PBufWriteULong(savebuffer, state_index);
    M_PBufWriteUInt(savebuffer, target_id);
    M_PBufWriteUInt(savebuffer, player_index);
    M_PBufWriteUInt(savebuffer, tracer_id);
    M_PBufWriteUInt(savebuffer, lastenemy_id);
  }

  // killough 9/14/98: save soundtargets
  for (int i = 0; i < numsectors; i++) {
    mobj_t *target = sectors[i].soundtarget;

    if (target && target->thinker.function == P_MobjThinker) {
      M_PBufWriteUInt(savebuffer, target->id);
    }
    else {
      M_PBufWriteUInt(savebuffer, 0);
    }
  }

  /** Specials **/

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
      M_PBufWriteInt(savebuffer, ceiling->type);
      dump_sector_index(savebuffer, ceiling->sector, "ceiling");
      M_PBufWriteInt(savebuffer, ceiling->bottomheight);
      M_PBufWriteInt(savebuffer, ceiling->topheight);
      M_PBufWriteInt(savebuffer, ceiling->speed);
      M_PBufWriteInt(savebuffer, ceiling->oldspeed);
      M_PBufWriteInt(savebuffer, ceiling->crush);
      M_PBufWriteInt(savebuffer, ceiling->newspecial);
      M_PBufWriteInt(savebuffer, ceiling->oldspecial);
      M_PBufWriteShort(savebuffer, ceiling->texture);
      M_PBufWriteInt(savebuffer, ceiling->direction);
      M_PBufWriteInt(savebuffer, ceiling->tag);
      M_PBufWriteInt(savebuffer, ceiling->olddirection);

      continue;
    }

    if (th->function == T_VerticalDoor) {
      vldoor_t *door = (vldoor_t *)th;

      M_PBufWriteInt(savebuffer, tc_door);
      M_PBufWriteInt(savebuffer, door->type);
      dump_sector_index(savebuffer, door->sector, "verticaldoor");
      M_PBufWriteInt(savebuffer, door->topheight);
      M_PBufWriteInt(savebuffer, door->speed);
      M_PBufWriteInt(savebuffer, door->direction);
      M_PBufWriteInt(savebuffer, door->topwait);
      M_PBufWriteInt(savebuffer, door->topcountdown);
      dump_line_index(savebuffer, door->line, "verticaldoor");
      M_PBufWriteInt(savebuffer, door->lighttag);

      continue;
    }

    if (th->function == T_MoveFloor) {
      floormove_t *floor = (floormove_t *)th;

      M_PBufWriteInt(savebuffer, tc_floor);
      M_PBufWriteInt(savebuffer, floor->type);
      M_PBufWriteInt(savebuffer, floor->crush);
      dump_sector_index(savebuffer, floor->sector, "floor");
      M_PBufWriteInt(savebuffer, floor->direction);
      M_PBufWriteInt(savebuffer, floor->newspecial);
      M_PBufWriteInt(savebuffer, floor->oldspecial);
      M_PBufWriteShort(savebuffer, floor->texture);
      M_PBufWriteInt(savebuffer, floor->floordestheight);
      M_PBufWriteInt(savebuffer, floor->speed);

      continue;
    }

    if (th->function == T_PlatRaise) {
      plat_t *plat = NULL;

      plat:   // killough 2/14/98: added fix for original plat height above

      plat = (plat_t *)th;

      M_PBufWriteInt(savebuffer, tc_plat);
      dump_sector_index(savebuffer, plat->sector, "plat");
      M_PBufWriteInt(savebuffer, plat->speed);
      M_PBufWriteInt(savebuffer, plat->low);
      M_PBufWriteInt(savebuffer, plat->high);
      M_PBufWriteInt(savebuffer, plat->wait);
      M_PBufWriteInt(savebuffer, plat->count);
      M_PBufWriteInt(savebuffer, plat->status);
      M_PBufWriteInt(savebuffer, plat->oldstatus);
      M_PBufWriteInt(savebuffer, plat->crush);
      M_PBufWriteInt(savebuffer, plat->tag);
      M_PBufWriteInt(savebuffer, plat->type);

      continue;
    }

    if (th->function == T_LightFlash) {
      lightflash_t *flash = (lightflash_t *)th;

      M_PBufWriteInt(savebuffer, tc_flash);
      dump_sector_index(savebuffer, flash->sector, "lightflash");
      M_PBufWriteInt(savebuffer, flash->count);
      M_PBufWriteInt(savebuffer, flash->maxlight);
      M_PBufWriteInt(savebuffer, flash->minlight);
      M_PBufWriteInt(savebuffer, flash->maxtime);
      M_PBufWriteInt(savebuffer, flash->mintime);

      continue;
    }

    if (th->function == T_StrobeFlash) {
      strobe_t *strobe = (strobe_t *)th;

      M_PBufWriteInt(savebuffer, tc_strobe);
      dump_sector_index(savebuffer, strobe->sector, "strobeflash");
      M_PBufWriteInt(savebuffer, strobe->count);
      M_PBufWriteInt(savebuffer, strobe->minlight);
      M_PBufWriteInt(savebuffer, strobe->maxlight);
      M_PBufWriteInt(savebuffer, strobe->darktime);
      M_PBufWriteInt(savebuffer, strobe->brighttime);

      continue;
    }

    if (th->function == T_Glow) {
      glow_t *glow = (glow_t *)th;

      M_PBufWriteInt(savebuffer, tc_glow);
      dump_sector_index(savebuffer, glow->sector, "glow");
      M_PBufWriteInt(savebuffer, glow->maxlight);
      M_PBufWriteInt(savebuffer, glow->minlight);
      M_PBufWriteInt(savebuffer, glow->direction);

      continue;
    }

    // killough 10/4/98: save flickers
    if (th->function == T_FireFlicker) {
      fireflicker_t *flicker = (fireflicker_t *)th;

      M_PBufWriteInt(savebuffer, tc_flicker);
      dump_sector_index(savebuffer, flicker->sector, "fireflicker");
      M_PBufWriteInt(savebuffer, flicker->count);
      M_PBufWriteInt(savebuffer, flicker->maxlight);
      M_PBufWriteInt(savebuffer, flicker->minlight);

      continue;
    }

    //jff 2/22/98 new case for elevators
    if (th->function == T_MoveElevator) {
      elevator_t *elevator = (elevator_t *)th;         //jff 2/22/98

      M_PBufWriteInt(savebuffer, tc_elevator);
      M_PBufWriteInt(savebuffer, elevator->type);
      dump_sector_index(savebuffer, elevator->sector, "elevator");
      M_PBufWriteInt(savebuffer, elevator->direction);
      M_PBufWriteInt(savebuffer, elevator->floordestheight);
      M_PBufWriteInt(savebuffer, elevator->ceilingdestheight);
      M_PBufWriteInt(savebuffer, elevator->speed);

      continue;
    }

    // killough 3/7/98: Scroll effect thinkers
    if (th->function == T_Scroll) {
      scroll_t *scroll = (scroll_t *)th;

      M_PBufWriteInt(savebuffer, tc_scroll);
      M_PBufWriteInt(savebuffer, scroll->dx);
      M_PBufWriteInt(savebuffer, scroll->dy);
      M_PBufWriteInt(savebuffer, scroll->affectee);
      M_PBufWriteInt(savebuffer, scroll->control);
      /*
       * [CG] scroll->last_height is uninitialized in Add_Scroller if there is
       *      no control sector, so only dump this if there is a control
       *      sector.
       */
      if (scroll->control == -1) {
        M_PBufWriteInt(savebuffer, 0);
      }
      else {
        M_PBufWriteInt(savebuffer, scroll->last_height);
      }
      M_PBufWriteInt(savebuffer, scroll->vdx);
      M_PBufWriteInt(savebuffer, scroll->vdy);
      M_PBufWriteInt(savebuffer, scroll->accel);
      M_PBufWriteInt(savebuffer, scroll->type);

      continue;
    }

    // phares 3/22/98: Push/Pull effect thinkers
    if (th->function == T_Pusher) {
      pusher_t *pusher = (pusher_t *)th;

      M_PBufWriteInt(savebuffer, tc_pusher);
      M_PBufWriteInt(savebuffer, pusher->type);
      M_PBufWriteInt(savebuffer, pusher->x_mag);
      M_PBufWriteInt(savebuffer, pusher->y_mag);
      M_PBufWriteInt(savebuffer, pusher->magnitude);
      M_PBufWriteInt(savebuffer, pusher->radius);
      M_PBufWriteInt(savebuffer, pusher->x);
      M_PBufWriteInt(savebuffer, pusher->y);
      M_PBufWriteInt(savebuffer, pusher->affectee);

      continue;
    }
  }

  // add a terminating marker
  M_PBufWriteInt(savebuffer, tc_endspecials);

  /** AutoMap **/
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

bool D_DumpEnabled(void) {
  if (dump_buf1) {
    return true;
  }

  return false;
}

void D_DumpInit(const char *input_dump_file_name) {
  dump_file_name = input_dump_file_name;
  dump_buf1 = M_PBufNew();
  dump_buf2 = M_PBufNew();
  delta_buf = M_BufferNew();
  size_buf = M_PBufNew();

  dump_fd = open(
    dump_file_name,
    O_WRONLY | O_CREAT | O_TRUNC,
    S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP
  );

  if (dump_fd == -1) {
    perror("Error opening dump file");
    I_Error("Opening %s failed, exiting\n", dump_file_name);
  }

  dump_fobj = fdopen(dump_fd, "wb");

  if (!dump_fobj) {
    perror("Error opening dump file");
    I_Error("Opening %s failed, exiting\n", dump_file_name);
  }

  atexit(close_dump_file);

  M_InitDeltas();
}

void D_DumpUpdate(void) {
  int status;

  M_PBufClear(dump_buf2);

  D_Dump(dump_buf2);

  M_PBufClear(size_buf);

  if (state_count == 0) {
    M_PBufWriteUInt(size_buf, M_PBufGetSize(dump_buf2));

    status = fwrite(
      M_PBufGetData(size_buf), 1, M_PBufGetSize(size_buf), dump_fobj
    );

    if (status != M_PBufGetSize(size_buf)) {
      I_Error("Error writing dump size to file");
    }

    status = fwrite(
      M_PBufGetData(dump_buf2), 1, M_PBufGetSize(dump_buf2), dump_fobj
    );

    if (status != M_PBufGetSize(dump_buf2)) {
      I_Error("Error writing initial dump to file");
    }
  }
  else {
    M_BuildDelta(dump_buf1, dump_buf2, delta_buf);

    M_PBufClear(size_buf);
    M_PBufWriteUInt(size_buf, M_BufferGetSize(delta_buf));

    status = fwrite(
      M_PBufGetData(size_buf), 1, M_PBufGetSize(size_buf), dump_fobj
    );

    if (status != M_PBufGetSize(size_buf)) {
      I_Error("Error writing dump size to file");
    }

    status = fwrite(
      M_BufferGetData(delta_buf), 1, M_BufferGetSize(delta_buf), dump_fobj
    );

    if (status != M_BufferGetSize(delta_buf)) {
      I_Error("Error writing dump delta to file");
    }
  }

  M_PBufSetData(
    dump_buf1, M_PBufGetData(dump_buf2), M_PBufGetSize(dump_buf2)
  );

  M_PBufClear(dump_buf2);

  state_count++;

#ifdef STATE_COUNT_LIMIT
  if (state_count > STATE_COUNT_LIMIT) {
    I_Error("Only dumping %d states", STATE_COUNT_LIMIT);
  }
#endif
}

/* vi: set et ts=2 sw=2: */

