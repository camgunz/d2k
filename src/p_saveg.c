/* Emacs style mode select   -*- C++ -*-
 *-----------------------------------------------------------------------------
 *
 *
 *  PrBoom: a Doom port merged with LxDoom and LSDLDoom
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 1999 by
 *  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
 *  Copyright (C) 1999-2000 by
 *  Jess Haas, Nicolas Kalkhof, Colin Phipps, Florian Schulze
 *  Copyright 2005, 2006 by
 *  Florian Schulze, Colin Phipps, Neil Stevens, Andrey Budko
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 *  02111-1307, USA.
 *
 * DESCRIPTION:
 *      Archiving: SaveGame I/O.
 *
 *-----------------------------------------------------------------------------*/

#include "z_zone.h"

#include "doomstat.h"
#include "g_game.h"
#include "r_main.h"
#include "p_map.h"
#include "p_maputl.h"
#include "p_setup.h"
#include "p_spec.h"
#include "p_tick.h"
#include "p_saveg.h"
#include "m_random.h"
#include "am_map.h"
#include "p_enemy.h"
#include "lprintf.h"
#include "s_advsound.h"
#include "e6y.h"//e6y

#include "n_net.h"

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

static int number_of_thinkers;

// CG: Broke out ouf P_UnArchiveThinkers to avoid reallocating a new array
//     every time a savegame is loaded.  For the old netcode that was fine,
//     for the new netcode that's untenable.
static int old_thinker_count = 0;
static mobj_t **mobj_p = NULL; // killough 2/14/98: Translation table

// savegame file stores ints in the corresponding * field; this function
// safely casts them back to int.
static int P_GetMobj(mobj_t* mi, size_t s) {
  size_t i = (size_t)mi;

  if (i > s)
    I_Error("Corrupt savegame (thinker %zu > max thinker count %zu)", i, s);

  return i;
}

/*
 * killough 11/98
 *
 * Same as P_SetTarget() in p_tick.c, except that the target is nullified
 * first, so that no old target's reference count is decreased (when loading
 * savegames, old targets are indices, not really pointers to targets).
 */

static void P_SetNewTarget(mobj_t **mop, mobj_t *targ) {
  *mop = NULL;
  P_SetTarget(mop, targ);
}

static void P_ArchivePlayer(buf_t *savebuffer, player_t *player) {
  M_BufferWriteInt(savebuffer, player->playerstate);
  M_BufferWriteChar(savebuffer, player->cmd.forwardmove);
  M_BufferWriteChar(savebuffer, player->cmd.sidemove);
  M_BufferWriteShort(savebuffer, player->cmd.angleturn);
  M_BufferWriteShort(savebuffer, player->cmd.consistancy);
  M_BufferWriteUChar(savebuffer, player->cmd.chatchar);
  M_BufferWriteUChar(savebuffer, player->cmd.buttons);
  M_BufferWriteInt(savebuffer, player->viewz);
  M_BufferWriteInt(savebuffer, player->viewheight);
  M_BufferWriteInt(savebuffer, player->deltaviewheight);
  M_BufferWriteInt(savebuffer, player->bob);
  M_BufferWriteInt(savebuffer, player->health);
  M_BufferWriteInt(savebuffer, player->armorpoints);
  M_BufferWriteInt(savebuffer, player->armortype);
  for (int i = 0; i < NUMPOWERS; i++)
    M_BufferWriteInt(savebuffer, player->powers[i]);
  for (int i = 0; i < NUMCARDS; i++)
    M_BufferWriteInt(savebuffer, player->cards[i]);
  M_BufferWriteInt(savebuffer, player->backpack);
  for (int i = 0; i < MAXPLAYERS; i++)
    M_BufferWriteInt(savebuffer, player->frags[i]);
  M_BufferWriteInt(savebuffer, player->readyweapon);
  M_BufferWriteInt(savebuffer, player->pendingweapon);
  for (int i = 0; i < NUMWEAPONS; i++)
    M_BufferWriteInt(savebuffer, player->weaponowned[i]);
  for (int i = 0; i < NUMAMMO; i++)
    M_BufferWriteInt(savebuffer, player->ammo[i]);
  for (int i = 0; i < NUMAMMO; i++)
    M_BufferWriteInt(savebuffer, player->maxammo[i]);
  M_BufferWriteInt(savebuffer, player->attackdown);
  M_BufferWriteInt(savebuffer, player->usedown);
  M_BufferWriteInt(savebuffer, player->cheats);
  M_BufferWriteInt(savebuffer, player->refire);
  M_BufferWriteInt(savebuffer, player->killcount);
  M_BufferWriteInt(savebuffer, player->itemcount);
  M_BufferWriteInt(savebuffer, player->secretcount);
  if (player->message) {
    size_t message_length = MIN(MAX_MESSAGE_SIZE - 1, strlen(player->message));

    M_BufferWriteLong(savebuffer, message_length);

    if (message_length > 0)
      M_BufferWriteString(savebuffer, (char *)player->message, message_length);
  }
  else {
    M_BufferWriteLong(savebuffer, 0);
  }
  M_BufferWriteInt(savebuffer, player->damagecount);
  M_BufferWriteInt(savebuffer, player->bonuscount);
  M_BufferWriteInt(savebuffer, player->extralight);
  M_BufferWriteInt(savebuffer, player->fixedcolormap);
  M_BufferWriteInt(savebuffer, player->colormap);
  for (int i = 0; i < NUMPSPRITES; i++) {
    M_BufferWriteLong(savebuffer, player->psprites[i].state - states);
  }
  M_BufferWriteInt(savebuffer, player->didsecret);
  M_BufferWriteInt(savebuffer, player->momx);
  M_BufferWriteInt(savebuffer, player->momy);
  M_BufferWriteInt(savebuffer, player->resurectedkillcount);
  M_BufferWriteInt(savebuffer, player->prev_viewz);
  M_BufferWriteInt(savebuffer, player->prev_viewangle);
  M_BufferWriteInt(savebuffer, player->prev_viewpitch);
  M_BufferWriteInt(savebuffer, player->jumpTics);
  /*
   * CG: TODO: Settle the maximum name size, don't just use MAX_MESSAGE_SIZE
   *           here, which is something like 500.
   */
  if (player->name) {
    size_t name_length = MIN(MAX_MESSAGE_SIZE - 1, strlen(player->name));

    M_BufferWriteLong(savebuffer, name_length);

    if (name_length > 0)
      M_BufferWriteString(savebuffer, (char *)player->name, name_length);
  }
  else {
    M_BufferWriteLong(savebuffer, 0);
  }
  M_BufferWriteUChar(savebuffer, player->team);
  /*
   * CG: Don't use P_GetPlayerCommands here; that would overwrite the local
   *     command buffer.
   */
  M_CBufConsolidate(&player->commands);
  M_BufferWriteInt(savebuffer, M_CBufGetObjectCount(&player->commands));
  CBUF_FOR_EACH(&player->commands, entry) {
    netticcmd_t *ncmd = (netticcmd_t *)entry.obj;

    M_BufferWriteInt(savebuffer, ncmd->tic);
    M_BufferWriteChar(savebuffer, ncmd->cmd.forwardmove);
    M_BufferWriteChar(savebuffer, ncmd->cmd.sidemove);
    M_BufferWriteShort(savebuffer, ncmd->cmd.angleturn);
    M_BufferWriteShort(savebuffer, ncmd->cmd.consistancy);
    M_BufferWriteUChar(savebuffer, ncmd->cmd.chatchar);
    M_BufferWriteUChar(savebuffer, ncmd->cmd.buttons);
  }
}

static void P_UnArchivePlayer(buf_t *savebuffer, player_t *player) {
  static char msg[MAX_MESSAGE_SIZE];
  static char name[MAX_MESSAGE_SIZE];

  int command_count = 0;
  size_t message_length, name_length;

  memset(msg, 0, MAX_MESSAGE_SIZE * sizeof(char));
  memset(name, 0, MAX_MESSAGE_SIZE * sizeof(char));

  M_BufferReadInt(savebuffer, (int *)&player->playerstate);
  M_BufferReadChar(savebuffer, &player->cmd.forwardmove);
  M_BufferReadChar(savebuffer, &player->cmd.sidemove);
  M_BufferReadShort(savebuffer, &player->cmd.angleturn);
  M_BufferReadShort(savebuffer, &player->cmd.consistancy);
  M_BufferReadUChar(savebuffer, &player->cmd.chatchar);
  M_BufferReadUChar(savebuffer, &player->cmd.buttons);
  M_BufferReadInt(savebuffer, &player->viewz);
  M_BufferReadInt(savebuffer, &player->viewheight);
  M_BufferReadInt(savebuffer, &player->deltaviewheight);
  M_BufferReadInt(savebuffer, &player->bob);
  M_BufferReadInt(savebuffer, &player->health);
  M_BufferReadInt(savebuffer, &player->armorpoints);
  M_BufferReadInt(savebuffer, &player->armortype);
  for (int i = 0; i < NUMPOWERS; i++)
    M_BufferReadInt(savebuffer, &player->powers[i]);
  for (int i = 0; i < NUMCARDS; i++)
    M_BufferReadInt(savebuffer, (int *)&player->cards[i]);
  M_BufferReadInt(savebuffer, (int *)&player->backpack);
  for (int i = 0; i < MAXPLAYERS; i++)
    M_BufferReadInt(savebuffer, &player->frags[i]);
  M_BufferReadInt(savebuffer, (int *)&player->readyweapon);
  M_BufferReadInt(savebuffer, (int *)&player->pendingweapon);
  for (int i = 0; i < NUMWEAPONS; i++)
    M_BufferReadInt(savebuffer, (int *)&player->weaponowned[i]);
  for (int i = 0; i < NUMAMMO; i++)
    M_BufferReadInt(savebuffer, &player->ammo[i]);
  for (int i = 0; i < NUMAMMO; i++)
    M_BufferReadInt(savebuffer, &player->maxammo[i]);
  M_BufferReadInt(savebuffer, &player->attackdown);
  M_BufferReadInt(savebuffer, &player->usedown);
  M_BufferReadInt(savebuffer, &player->cheats);
  M_BufferReadInt(savebuffer, &player->refire);
  M_BufferReadInt(savebuffer, &player->killcount);
  M_BufferReadInt(savebuffer, &player->itemcount);
  M_BufferReadInt(savebuffer, &player->secretcount);
  M_BufferReadLong(savebuffer, (int_64_t *)&message_length);
  if (message_length > 0) {
    message_length = MIN(MAX_MESSAGE_SIZE, message_length + 1);
    M_BufferReadString(savebuffer, msg, message_length);
    doom_pprintf(player - players, "%s", msg);
  }
  else {
    player->message = "";
  }
  M_BufferReadInt(savebuffer, &player->damagecount);
  M_BufferReadInt(savebuffer, &player->bonuscount);
  M_BufferReadInt(savebuffer, &player->extralight);
  M_BufferReadInt(savebuffer, &player->fixedcolormap);
  M_BufferReadInt(savebuffer, &player->colormap);
  for (int i = 0; i < NUMPSPRITES; i++) {
    int_64_t state_index = 0;

    M_BufferReadLong(savebuffer, &state_index);
    player->psprites[i].state = &states[state_index];
  }
  M_BufferReadInt(savebuffer, (int *)&player->didsecret);
  M_BufferReadInt(savebuffer, &player->momx);
  M_BufferReadInt(savebuffer, &player->momy);
  M_BufferReadInt(savebuffer, &player->resurectedkillcount);
  M_BufferReadInt(savebuffer, &player->prev_viewz);
  M_BufferReadInt(savebuffer, (int *)&player->prev_viewangle);
  M_BufferReadInt(savebuffer, (int *)&player->prev_viewpitch);
  M_BufferReadInt(savebuffer, &player->jumpTics);
  M_BufferReadLong(savebuffer, (int_64_t *)&name_length);
  if (name_length > 0) {
    name_length = MIN(MAX_MESSAGE_SIZE, name_length + 1);
    M_BufferReadString(savebuffer, name, name_length);
    if (player->name != NULL)
      free(player->name);
    player->name = strdup(name);
  }
  else {
    player->name = "";
  }
  M_BufferReadUChar(savebuffer, &player->team);
  M_BufferReadInt(savebuffer, &command_count);
  if (command_count > 10000)
    I_Error("Command count too high\n");

  M_CBufClear(&player->commands);
  M_CBufEnsureCapacity(&player->commands, command_count);

  while (command_count--) {
    netticcmd_t *ncmd = M_CBufGetFirstFreeOrNewSlot(&player->commands);

    M_BufferReadInt(savebuffer, &ncmd->tic);
    M_BufferReadChar(savebuffer, &ncmd->cmd.forwardmove);
    M_BufferReadChar(savebuffer, &ncmd->cmd.sidemove);
    M_BufferReadShort(savebuffer, &ncmd->cmd.angleturn);
    M_BufferReadShort(savebuffer, &ncmd->cmd.consistancy);
    M_BufferReadUChar(savebuffer, &ncmd->cmd.chatchar);
    M_BufferReadUChar(savebuffer, &ncmd->cmd.buttons);
  }
}

//
// P_ArchivePlayers
//
void P_ArchivePlayers(buf_t *savebuffer) {
  for (int i = 0; i < MAXPLAYERS; i++) {
    if (!playeringame[i])
      continue;

    P_ArchivePlayer(savebuffer, &players[i]);
  }
}

//
// P_UnArchivePlayers
//
void P_UnArchivePlayers(buf_t *savebuffer) {
  for (int i = 0; i < MAXPLAYERS; i++) {
    if (!playeringame[i])
      continue;

    P_UnArchivePlayer(savebuffer, &players[i]);

    // will be set when unarc thinker
    players[i].mo = NULL;
    // players[i].message = NULL;
    players[i].attacker = NULL;
  }
}

//
// P_ArchiveWorld
//
void P_ArchiveWorld(buf_t *savebuffer) {
  int i;
  const sector_t *sec;
  const line_t *li;

  // do sectors
  for (i = 0, sec = sectors; i < numsectors; i++, sec++) {
    // killough 10/98: save full floor & ceiling heights, including fraction
    M_BufferWriteInt(savebuffer, sec->floorheight);
    M_BufferWriteInt(savebuffer, sec->ceilingheight);

    M_BufferWriteShort(savebuffer, sec->floorpic);
    M_BufferWriteShort(savebuffer, sec->ceilingpic);
    M_BufferWriteShort(savebuffer, sec->lightlevel);
    // needed?   yes -- transfer types
    M_BufferWriteShort(savebuffer, sec->special);
    // needed?   need them -- killough
    M_BufferWriteShort(savebuffer, sec->tag);
  }

  // do lines
  for (i = 0, li = lines; i < numlines; i++, li++) {
    int j;

    M_BufferWriteShort(savebuffer, li->flags);
    M_BufferWriteShort(savebuffer, li->special);
    M_BufferWriteShort(savebuffer, li->tag);

    for (j = 0; j < 2; j++) {
      if (li->sidenum[j] != NO_INDEX) {
        side_t *si = &sides[li->sidenum[j]];

        // killough 10/98: save full sidedef offsets,
        // preserving fractional scroll offsets
        M_BufferWriteInt(savebuffer, si->textureoffset);
        M_BufferWriteInt(savebuffer, si->rowoffset);
        M_BufferWriteShort(savebuffer, si->toptexture);
        M_BufferWriteShort(savebuffer, si->bottomtexture);
        M_BufferWriteShort(savebuffer, si->midtexture);
      }
    }
  }

  M_BufferWriteInt(savebuffer, musinfo.current_item);
}

//
// P_UnArchiveWorld
//
void P_UnArchiveWorld(buf_t *savebuffer) {
  int i;
  sector_t *sec;
  line_t *li;

  // do sectors
  for (i = 0, sec = sectors; i < numsectors; i++, sec++) {
    // killough 10/98: load full floor & ceiling heights, including fractions
    M_BufferReadInt(savebuffer, &sec->floorheight);
    M_BufferReadInt(savebuffer, &sec->ceilingheight);
    M_BufferReadShort(savebuffer, &sec->floorpic);
    M_BufferReadShort(savebuffer, &sec->ceilingpic);
    M_BufferReadShort(savebuffer, &sec->lightlevel);
    M_BufferReadShort(savebuffer, &sec->special);
    M_BufferReadShort(savebuffer, &sec->tag);

    sec->ceilingdata = 0; //jff 2/22/98 now three thinker fields, not two
    sec->floordata = 0;
    sec->lightingdata = 0;
    sec->soundtarget = 0;
  }

  // do lines
  for (i = 0, li = lines; i < numlines; i++, li++) {
    int j;

    M_BufferReadShort(savebuffer, (short *)&li->flags);
    M_BufferReadShort(savebuffer, &li->special);
    M_BufferReadShort(savebuffer, &li->tag);

    for (j = 0; j < 2; j++) {
      if (li->sidenum[j] != NO_INDEX) {
        side_t *si = &sides[li->sidenum[j]];

        // killough 10/98: load full sidedef offsets, including fractions
        M_BufferReadInt(savebuffer, &si->textureoffset);
        M_BufferReadInt(savebuffer, &si->rowoffset);
        M_BufferReadShort(savebuffer, &si->toptexture);
        M_BufferReadShort(savebuffer, &si->bottomtexture);
        M_BufferReadShort(savebuffer, &si->midtexture);
      }
    }
  }

  M_BufferReadInt(savebuffer, &musinfo.current_item);
}

//
// Thinkers
//

// phares 9/13/98: Moved this code outside of P_ArchiveThinkers so the
// thinker indices could be used by the code that saves sector info.

void P_ThinkerToIndex(void) {
  thinker_t *th;

  // killough 2/14/98:
  // count the number of thinkers, and mark each one with its index, using
  // the prev field as a placeholder, since it can be restored later.

  number_of_thinkers = 0;
  for (th = thinkercap.next ; th != &thinkercap ; th=th->next) {
    if (th->function == P_MobjThinker) {
      th->prev = (thinker_t *) ++number_of_thinkers;
    }
  }
}

// phares 9/13/98: Moved this code outside of P_ArchiveThinkers so the
// thinker indices could be used by the code that saves sector info.

void P_IndexToThinker(void) {
  // killough 2/14/98: restore prev pointers
  thinker_t *th;
  thinker_t *prev = &thinkercap;

  for (th = thinkercap.next; th != &thinkercap; prev = th, th = th->next)
    th->prev = prev;
}

//
// P_ArchiveThinkers
//
// 2/14/98 killough: substantially modified to fix savegame bugs
//

void P_ArchiveThinkers(buf_t *savebuffer) {
  int i;
  thinker_t *th;
  unsigned int thinker_count = 0;

  // killough 3/26/98: Save boss brain state
  M_BufferWriteInt(savebuffer, brain.easy);
  M_BufferWriteInt(savebuffer, brain.targeton);

  // save off the current thinkers
  for (th = thinkercap.next; th != &thinkercap; th = th->next) {
    if (th->function == P_MobjThinker) {
      thinker_count++;
    }
  }

  M_BufferWriteInt(savebuffer, (int)thinker_count);

  for (th = thinkercap.next; th != &thinkercap; th = th->next) {
    if (th->function == P_MobjThinker) {
      mobj_t *mobj = (mobj_t *)th;
      mobj_t *target = mobj->target;
      mobj_t *tracer = mobj->tracer;
      mobj_t *lastenemy = mobj->lastenemy;
      state_t *state = mobj->state;
      player_t *player = mobj->player;

      // killough 2/14/98: convert pointers into indices.
      // Fixes many savegame problems, by properly saving
      // target and tracer fields. Note: we store NULL if
      // the thinker pointed to by these fields is not a
      // mobj thinker.

      if (mobj->target) {
        if (mobj->target->thinker.function == P_MobjThinker)
          mobj->target = (mobj_t *)mobj->target->thinker.prev;
        else
          mobj->target = NULL;
      }

      if (mobj->tracer) {
        if (mobj->tracer->thinker.function == P_MobjThinker)
          mobj->tracer = (mobj_t *)mobj->tracer->thinker.prev;
        else
          mobj->tracer = NULL;
      }

      // killough 2/14/98: new field: save last known enemy. Prevents
      // monsters from going to sleep after killing monsters and not
      // seeing player anymore.

      if (mobj->lastenemy) {
        if (mobj->lastenemy->thinker.function == P_MobjThinker)
          mobj->lastenemy = (mobj_t *)mobj->lastenemy->thinker.prev;
        else
          mobj->lastenemy = NULL;
      }

      // killough 2/14/98: end changes

      mobj->state = (state_t *)(state - states);

      if (player) {
        uint_64_t playernum = (uint_64_t)(mobj->player - players) + 1;

        mobj->player = (player_t *)playernum;
      }

      M_BufferWrite(savebuffer, mobj, sizeof(mobj_t));

      mobj->state = state;

      if (player) {
        mobj->player = player;
        player->mo = mobj;
      }

      mobj->target = target;
      mobj->tracer = tracer;
      mobj->lastenemy = lastenemy;
    }
  }

  // killough 9/14/98: save soundtargets
  for (i = 0; i < numsectors; i++) {
    mobj_t *target = sectors[i].soundtarget;
    // Fix crash on reload when a soundtarget points to a removed corpse
    // (prboom bug #1590350)
    if (target && target->thinker.function == P_MobjThinker)
      target = (mobj_t *) target->thinker.prev;
    else
      target = NULL;
    M_BufferWriteLong(savebuffer, (int_64_t)target);
  }
}

//
// P_UnArchiveThinkers
//
// 2/14/98 killough: substantially modified to fix savegame bugs
//

void P_UnArchiveThinkers(buf_t *savebuffer) {
  int         i;
  thinker_t  *th;
  unsigned int thinker_count = 0;

  totallive = 0; // CG: This is a global that lives in g_game.c, just FYI

  // killough 3/26/98: Load boss brain state
  M_BufferReadInt(savebuffer, &brain.easy);
  M_BufferReadInt(savebuffer, &brain.targeton);

  // remove all the current thinkers
  for (th = thinkercap.next; th != &thinkercap;) {
    thinker_t *next = th->next;
    if (th->function == P_MobjThinker) {
      P_RemoveMobj((mobj_t *)th);
      P_RemoveThinkerDelayed(th); // fix mobj leak
    }
    else {
      Z_Free(th);
    }
    th = next;
  }

  P_InitThinkers();
  for (int i = 0; i < numsectors; i++) {
    sectors[i].thinglist = NULL;
    sectors[i].touching_thinglist = NULL;
  }
  memset(blocklinks, 0, bmapwidth * bmapheight * sizeof(*blocklinks));
  P_FreeSecNodeList();

  M_BufferReadInt(savebuffer, (int *)&thinker_count);

  if (thinker_count > old_thinker_count) {
    // table of pointers
    // first table entry special: 0 maps to NULL
    old_thinker_count = thinker_count;
    mobj_p = realloc(mobj_p, (thinker_count + 1) * sizeof(*mobj_p));
  }
  memset(mobj_p, 0, (thinker_count + 1) * sizeof(*mobj_p));

  for (i = 1; i < (thinker_count + 1); i++) {
    // killough 2/14/98
    mobj_t *mobj = Z_Malloc(sizeof(mobj_t), PU_LEVEL, NULL);

    // killough 2/14/98 -- insert pointers to thinkers into table, in order:
    mobj_p[i + 1] = mobj;

    M_BufferRead(savebuffer, mobj, sizeof(mobj_t));

    mobj->state = states + (uint_64_t)mobj->state;

    if (mobj->player) {
      uint_64_t playernum = (uint_64_t)mobj->player;

      mobj->player = &players[playernum - 1];
      mobj->player->mo = mobj;
    }

    mobj->snext = NULL;
    mobj->sprev = NULL;
    P_SetThingPosition(mobj);
    mobj->info = &mobjinfo[mobj->type];

    // killough 2/28/98:
    // Fix for falling down into a wall after savegame loaded:
    //      mobj->floorz = mobj->subsector->sector->floorheight;
    //      mobj->ceilingz = mobj->subsector->sector->ceilingheight;

    mobj->thinker.function = P_MobjThinker;
    P_AddThinker(&mobj->thinker);

    if (!((mobj->flags ^ MF_COUNTKILL) &
        (MF_FRIEND | MF_COUNTKILL | MF_CORPSE))) {
      totallive++;
    }
  }

  // killough 2/14/98: adjust target and tracer fields, plus
  // lastenemy field, to correctly point to mobj thinkers.
  // NULL entries automatically handled by first table entry.
  //
  // killough 11/98: use P_SetNewTarget() to set fields

  for (th = thinkercap.next; th != &thinkercap; th = th->next) {
    P_SetNewTarget(
      &((mobj_t *)th)->target,
      mobj_p[P_GetMobj(((mobj_t *)th)->target, thinker_count)]
    );

    P_SetNewTarget(
      &((mobj_t *)th)->tracer,
      mobj_p[P_GetMobj(((mobj_t *)th)->tracer, thinker_count)]
    );

    P_SetNewTarget(
      &((mobj_t *)th)->lastenemy,
      mobj_p[P_GetMobj(((mobj_t *)th)->lastenemy, thinker_count)]
    );
  }

  // killough 9/14/98: restore soundtargets
  for (i = 0; i < numsectors; i++) {
    int_64_t target;

    M_BufferReadLong(savebuffer, &target);
    // Must verify soundtarget. See P_ArchiveThinkers.
    P_SetNewTarget(
      &sectors[i].soundtarget,
      mobj_p[P_GetMobj((mobj_t *)target, thinker_count)]
    );
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

void P_ArchiveSpecials(buf_t *savebuffer) {
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
      sector_t *sector = NULL;

      ceiling:                               // killough 2/14/98

      ceiling = (ceiling_t *)th;
      sector = ceiling->sector;

      ceiling->sector = (sector_t *)(ceiling->sector->iSectorID);

      M_BufferWriteUChar(savebuffer, tc_ceiling);
      M_BufferWrite(savebuffer, ceiling, sizeof(*ceiling));

      ceiling->sector = sector;

      continue;
    }

    if (th->function == T_VerticalDoor) {
      vldoor_t *door = (vldoor_t *)th;
      sector_t *sector = door->sector;
      line_t   *line = door->line;

      door->sector = (sector_t *)(door->sector->iSectorID);
      if (door->line)
        door->line = (line_t *)(door->line - lines);
      else
        door->line = (line_t *)-1;

      M_BufferWriteUChar(savebuffer, tc_door);
      M_BufferWrite(savebuffer, door, sizeof(*door));

      door->sector = sector;
      if (door->line == (line_t *)-1)
        door->line = NULL;
      else
        door->line = line;

      continue;
    }

    if (th->function == T_MoveFloor) {
      floormove_t *floor = (floormove_t *)th;
      sector_t *sector = floor->sector;

      floor->sector = (sector_t *)(floor->sector->iSectorID);

      M_BufferWriteUChar(savebuffer, tc_floor);
      M_BufferWrite(savebuffer, floor, sizeof(*floor));

      floor->sector = sector;

      continue;
    }

    if (th->function == T_PlatRaise) {
      plat_t *plat = NULL;
      sector_t *sector = NULL;

      plat:   // killough 2/14/98: added fix for original plat height above

      plat = (plat_t *)th;
      sector = plat->sector;

      plat->sector = (sector_t *)(plat->sector->iSectorID);

      M_BufferWriteUChar(savebuffer, tc_plat);
      M_BufferWrite(savebuffer, plat, sizeof(*plat));

      plat->sector = sector;

      continue;
    }

    if (th->function == T_LightFlash) {
      lightflash_t *flash = (lightflash_t *)th;
      sector_t *sector = flash->sector;

      flash->sector = (sector_t *)(flash->sector->iSectorID);

      M_BufferWriteUChar(savebuffer, tc_flash);
      M_BufferWrite(savebuffer, flash, sizeof(*flash));

      flash->sector = sector;

      continue;
    }

    if (th->function == T_StrobeFlash) {
      strobe_t *strobe = (strobe_t *)th;
      sector_t *sector = strobe->sector;

      strobe->sector = (sector_t *)(strobe->sector->iSectorID);

      M_BufferWriteUChar(savebuffer, tc_strobe);
      M_BufferWrite(savebuffer, strobe, sizeof(*strobe));

      strobe->sector = sector;

      continue;
    }

    if (th->function == T_Glow) {
      glow_t *glow = (glow_t *)th;
      sector_t *sector = glow->sector;

      glow->sector = (sector_t *)(glow->sector->iSectorID);

      M_BufferWriteUChar(savebuffer, tc_glow);
      M_BufferWrite(savebuffer, glow, sizeof(*glow));

      glow->sector = sector;

      continue;
    }

    // killough 10/4/98: save flickers
    if (th->function == T_FireFlicker) {
      fireflicker_t *flicker = (fireflicker_t *)th;
      sector_t *sector = flicker->sector;

      flicker->sector = (sector_t *)(flicker->sector->iSectorID);

      M_BufferWriteUChar(savebuffer, tc_flicker);
      M_BufferWrite(savebuffer, flicker, sizeof(*flicker));

      flicker->sector = sector;

      continue;
    }

    //jff 2/22/98 new case for elevators
    if (th->function == T_MoveElevator) {
      elevator_t *elevator = (elevator_t *)th;         //jff 2/22/98
      sector_t *sector = elevator->sector;

      elevator->sector = (sector_t *)(elevator->sector->iSectorID);

      M_BufferWriteUChar(savebuffer, tc_elevator);
      M_BufferWrite(savebuffer, elevator, sizeof(*elevator));

      elevator->sector = sector;

      continue;
    }

    // killough 3/7/98: Scroll effect thinkers
    if (th->function == T_Scroll) {
      M_BufferWriteUChar(savebuffer, tc_scroll);
      M_BufferWrite(savebuffer, th, sizeof(scroll_t));

      continue;
    }

    // phares 3/22/98: Push/Pull effect thinkers
    if (th->function == T_Pusher) {
      M_BufferWriteUChar(savebuffer, tc_pusher);
      M_BufferWrite(savebuffer, th, sizeof(pusher_t));

      continue;
    }
  }

  // add a terminating marker
  M_BufferWriteUChar(savebuffer, tc_endspecials);
}

//
// P_UnArchiveSpecials
//
void P_UnArchiveSpecials(buf_t *savebuffer) {
  byte tclass;

  M_BufferReadUChar(savebuffer, &tclass);

  // read in saved thinkers
  // killough 2/14/98
  for (; tclass != tc_endspecials; M_BufferReadUChar(savebuffer, &tclass)) {
    switch (tclass) {
      case tc_ceiling:
      {
        ceiling_t *ceiling = Z_Malloc(sizeof(*ceiling), PU_LEVEL, NULL);

        M_BufferRead(savebuffer, ceiling, sizeof(*ceiling));
        ceiling->sector = &sectors[(int)ceiling->sector];
        ceiling->sector->ceilingdata = ceiling; //jff 2/22/98

        if (ceiling->thinker.function)
          ceiling->thinker.function = T_MoveCeiling;

        P_AddThinker(&ceiling->thinker);
        P_AddActiveCeiling(ceiling);
      }
      break;
      case tc_door:
      {
        vldoor_t *door = Z_Malloc(sizeof(*door), PU_LEVEL, NULL);

        M_BufferRead(savebuffer, door, sizeof(*door));
        door->sector = &sectors[(int)door->sector];

        //jff 1/31/98 unarchive line remembered by door as well
        door->line = (int)door->line != -1 ? &lines[(int)door->line] : NULL;

        door->sector->ceilingdata = door;       //jff 2/22/98
        door->thinker.function = T_VerticalDoor;
        P_AddThinker(&door->thinker);
      }
      break;
      case tc_floor:
      {
        floormove_t *floor = Z_Malloc (sizeof(*floor), PU_LEVEL, NULL);

        M_BufferRead(savebuffer, floor, sizeof(*floor));
        floor->sector = &sectors[(int)floor->sector];
        floor->sector->floordata = floor; //jff 2/22/98
        floor->thinker.function = T_MoveFloor;
        P_AddThinker(&floor->thinker);
      }
      break;
      case tc_plat:
      {
        plat_t *plat = Z_Malloc(sizeof(*plat), PU_LEVEL, NULL);

        M_BufferRead(savebuffer, plat, sizeof(*plat));
        plat->sector = &sectors[(int)plat->sector];
        plat->sector->floordata = plat; //jff 2/22/98

        if (plat->thinker.function)
          plat->thinker.function = T_PlatRaise;

        P_AddThinker(&plat->thinker);
        P_AddActivePlat(plat);
      }
      break;
      case tc_flash:
      {
        lightflash_t *flash = Z_Malloc(sizeof(*flash), PU_LEVEL, NULL);

        M_BufferRead(savebuffer, flash, sizeof(*flash));
        flash->sector = &sectors[(int)flash->sector];
        flash->thinker.function = T_LightFlash;
        P_AddThinker(&flash->thinker);
      }
      break;
      case tc_strobe:
      {
        strobe_t *strobe = Z_Malloc(sizeof(*strobe), PU_LEVEL, NULL);

        M_BufferRead(savebuffer, strobe, sizeof(*strobe));
        strobe->sector = &sectors[(int)strobe->sector];
        strobe->thinker.function = T_StrobeFlash;
        P_AddThinker(&strobe->thinker);
      }
      break;
      case tc_glow:
      {
        glow_t *glow = Z_Malloc(sizeof(*glow), PU_LEVEL, NULL);

        M_BufferRead(savebuffer, glow, sizeof(*glow));
        glow->sector = &sectors[(int)glow->sector];
        glow->thinker.function = T_Glow;
        P_AddThinker(&glow->thinker);
      }
      break;
      case tc_flicker:           // killough 10/4/98
      {
        fireflicker_t *flicker = Z_Malloc(sizeof(*flicker), PU_LEVEL, NULL);

        M_BufferRead(savebuffer, flicker, sizeof(*flicker));
        flicker->sector = &sectors[(int)flicker->sector];
        flicker->thinker.function = T_FireFlicker;
        P_AddThinker(&flicker->thinker);
      }
      break;
      case tc_elevator: //jff 2/22/98 new case for elevators
      {
        elevator_t *elevator = Z_Malloc(sizeof(*elevator), PU_LEVEL, NULL);

        M_BufferRead(savebuffer, elevator, sizeof(*elevator));
        elevator->sector = &sectors[(int)elevator->sector];
        elevator->sector->floordata = elevator; //jff 2/22/98
        elevator->sector->ceilingdata = elevator; //jff 2/22/98
        elevator->thinker.function = T_MoveElevator;
        P_AddThinker(&elevator->thinker);
      }
      break;
      case tc_scroll:       // killough 3/7/98: scroll effect thinkers
      {
        scroll_t *scroll = Z_Malloc(sizeof(scroll_t), PU_LEVEL, NULL);

        M_BufferRead(savebuffer, scroll, sizeof(scroll_t));
        scroll->thinker.function = T_Scroll;
        P_AddThinker(&scroll->thinker);
      }
      break;
      case tc_pusher:   // phares 3/22/98: new Push/Pull effect thinkers
      {
        pusher_t *pusher = Z_Malloc(sizeof(pusher_t), PU_LEVEL, NULL);

        M_BufferRead(savebuffer, pusher, sizeof(pusher_t));
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
void P_ArchiveRNG(buf_t *savebuffer) {
  M_BufferWrite(savebuffer, &rng, sizeof(rng));
}

void P_UnArchiveRNG(buf_t *savebuffer) {
  M_BufferRead(savebuffer, &rng, sizeof(rng));
}

// killough 2/22/98: Save/restore automap state
void P_ArchiveMap(buf_t *savebuffer) {
  M_BufferWrite(savebuffer, &automapmode, sizeof(automapmode));
  M_BufferWrite(savebuffer, &markpointnum, sizeof(markpointnum));

  for (int i = 0; i < markpointnum; i++) {
    M_BufferWrite(savebuffer, &markpoints[i].x, sizeof(markpoints[i].x));
    M_BufferWrite(savebuffer, &markpoints[i].y, sizeof(markpoints[i].y));
  }
}

void P_UnArchiveMap(buf_t *savebuffer) {
  M_BufferRead(savebuffer, &automapmode, sizeof(automapmode));

  if (automapmode & am_active)
    AM_Start();

  M_BufferRead(savebuffer, &markpointnum, sizeof(markpointnum));

  if (markpointnum) {
    if (markpointnum >= markpointnum_max) {
      while (markpointnum >= markpointnum_max) {
        if (markpointnum_max == 0)
          markpointnum_max = 8;

        markpointnum_max *= 2;
      }

      markpoints = realloc(
        markpoints, sizeof(*markpoints) * markpointnum_max
      );
    }

    for (int i = 0; i < markpointnum; i++) {
      M_BufferRead(savebuffer, &markpoints[i].x, sizeof(markpoints[i].x));
      M_BufferRead(savebuffer, &markpoints[i].y, sizeof(markpoints[i].y));

      AM_setMarkParams(i);
    }
  }
}

/* vi: set et ts=2 sw=2: */

