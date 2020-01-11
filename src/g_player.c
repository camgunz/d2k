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

#include "m_random.h"
#include "am_map.h"
#include "g_comp.h"
#include "g_corpse.h"
#include "g_game.h"
#include "g_player.h"
#include "p_inter.h"
#include "p_mobj.h"
#include "p_map.h"
#include "pl_cmd.h"
#include "pl_main.h"
#include "r_defs.h"
#include "r_draw.h"
#include "r_main.h"
#include "s_sound.h"
#include "sounds.h"

//
// PLAYER STRUCTURE FUNCTIONS
// also see P_SpawnPlayer in P_Things
//

//
// G_PlayerFinishLevel
// Can when a player completes a level.
//

void G_PlayerFinishLevel(player_t *player) {
  memset(player->powers, 0, sizeof(player->powers));
  memset(player->cards, 0, sizeof(player->cards));

  player->mo = NULL;         // cph - this is allocated PU_LEVEL so it's gone
  player->extralight = 0;    // cancel gun flashes
  player->fixedcolormap = 0; // cancel ir gogles
  player->damagecount = 0;   // no palette changes
  player->bonuscount = 0;

  PL_ResetCommands(player);

  player->telefragged_by_spawn = false;

  if (deathmatch) {
    player->playerstate = PST_REBORN;
  }
}

// CPhipps - G_SetPlayerColour
// Player colours stuff
//
// G_SetPlayerColour

void G_ChangedPlayerColour(int pn, int cl) {
  if (!netgame) {
    return;
  }

  vanilla_mapplayer_colors[pn % VANILLA_MAXPLAYERS] = cl;

  // Rebuild colour translation tables accordingly
  R_InitTranslationTables();

  // Change translations on existing player mobj's
  if (gamestate == GS_LEVEL) {
    PLAYERS_FOR_EACH(iter) {
      if (iter.player->mo) {
        uint32_t playernum = iter.player->id - 1;
        uint32_t translation_index = playernum % VANILLA_MAXPLAYERS;
        uint64_t translation_flag = playernumtotrans[translation_index];

        iter.player->mo->flags &= ~MF_TRANSLATION;
        iter.player->mo->flags |= translation_flag << MF_TRANSSHIFT;
      }
    }
  }
}

//
// G_PlayerReborn
// Called after a player dies
// almost everything is cleared and initialized
//

void G_PlayerReborn(player_t *player) {
  if (!player->telefragged_by_spawn) {
    player->armorpoints = 0;
    player->armortype = 0;
    memset(player->powers, 0, sizeof(int) * NUMPOWERS);
    memset(player->cards, 0,  sizeof(bool) * NUMCARDS);
    player->backpack = 0;
    memset(player->weaponowned, 0, sizeof(int) * NUMWEAPONS);
    memset(player->ammo, 0, sizeof(int) * NUMAMMO);
    memset(player->maxammo, 0, sizeof(int) * NUMAMMO);
    player->didsecret = 0;

    // Ty 03/12/98 - use dehacked values
    player->ammo[am_clip] = initial_bullets;

    for (int i = 0; i < NUMAMMO; i++) {
      player->maxammo[i] = maxammo[i];
    }
  }

  player->mo = NULL;
  player->playerstate = PST_LIVE;
  memset(&player->cmd, 0, sizeof(ticcmd_t));
  player->viewz = 0;
  player->viewheight = 0;
  player->deltaviewheight = 0;
  player->bob = 0;
  player->refire = 0;
  player->damagecount = 0;
  player->bonuscount = 0;
  player->attacker = NULL;
  player->extralight = 0;
  player->fixedcolormap = 0;
  player->colormap = 0;
  memset(player->psprites, 0, sizeof(player->psprites));
  player->momx = 0;
  player->momy = 0;
  player->prev_viewz = 0;
  player->prev_viewangle = 0;
  player->prev_viewpitch = 0;
  player->jumpTics = 0;

  player->usedown = player->attackdown = true;  // don't do anything immediately
  player->readyweapon = player->pendingweapon = wp_pistol;
  player->weaponowned[wp_fist] = true;
  player->weaponowned[wp_pistol] = true;

  /*
   * [CG] Yeah, this lets you get all your health back by having a player die
   *      then telefrag you with their spawn.  If you really want to cheat that
   *      way, OK.
   */
  player->health = initial_health; // Ty 03/12/98 - use dehacked values

  PL_ResetCommands(player);

  player->telefragged_by_spawn = false;
}

//
// G_CheckSpot
// Returns false if the player cannot be respawned
// at the given mapthing_t spot
// because something is occupying it
//

bool G_CheckSpot(player_t *player, mapthing_t *mthing) {
  fixed_t       x = mthing->x << FRACBITS;
  fixed_t       y = mthing->y << FRACBITS;
  fixed_t      xa;
  fixed_t      ya;
  subsector_t *ss;
  int           i;
  int          an;
  mobj_t      *mo; 

  if (!player->mo) {
    // first spawn of level, before corpses
    PLAYERS_FOR_EACH(iter) {
      if (iter.player == player) {
        break;
      }

      if (!iter.player->mo) {
        continue;
      }

      if (iter.player->mo->x == x && iter.player->mo->y == y) {
        return false;
      }
    }

    return true;
  }

  // killough 4/2/98: fix bug where P_CheckPosition() uses a non-solid
  // corpse to detect collisions with other players in DM starts
  //
  // Old code:
  // if (!P_CheckPosition (players[playernum].mo, x, y))
  //    return false;

  /*
   * [CG] [FIXME] This presumes the corpse isn't solid, which isn't wrong per
   *              se, but still unintended.  If the corpse is solid, it won't
   *              be after this check.
   */

  player->mo->flags |= MF_SOLID;
  i = P_CheckPosition(player->mo, x, y);
  player->mo->flags &= ~MF_SOLID;

  if (!i) {
    return false;
  }

  // flush an old corpse if needed
  // killough 2/8/98: make corpse queue have an adjustable limit
  // killough 8/1/98: Fix bugs causing strange crashes
  if (G_CorpseQueueGetLen() == 0) {
    P_RemoveMobj(player->mo);
  }
  else {
    if (!G_CorpseQueueInitialized()) {
      G_CorpseQueueInit();
    }

    if (G_CorpseQueueGetLen() == G_CorpseQueueGetMaxLen()) {
      G_CorpseQueuePop();
    }

    G_CorpseQueuePush(player->mo);
  }

  // spawn a teleport fog
  ss = R_PointInSubsector(x, y);

  // Teleport fog at respawn point

  /* BUG: an can end up negative, because mthing->angle is (signed) short.
   * We have to emulate original Doom's behaviour, deferencing past the start
   * of the array, into the previous array (finetangent) */

  /*
  an = ( ANG45 * ((signed)mthing->angle/45) ) >> ANGLETOFINESHIFT;
  */

  /*
   * [CG] Shifting these signed values is implementation-defined, and it turns
   *      out that clang will (in some configurations) mess this up.  We can
   *      use this lookup here because spawnpoints only have 8 potential
   *      angles.
   */

  switch (mthing->angle) {
    case 45:
      an = 1024;
      break;
    case 90:
      an = 2048;
      break;
    case 135:
      an = 3072;
      break;
    case 180:
      an = -4096;
      break;
    case 225:
      an = -3072;
      break;
    case 270:
      an = -2048;
      break;
    case 315:
      an = -1024;
      break;
    case 360:
    case 0:
      an = 0;
      break;
    default:
      an = 0;
      I_Error("Unexpected angle %d\n", mthing->angle);
      break;
  }

  xa = finecosine[an];
  ya = finesine[an];

  if (compatibility_level <= finaldoom_compatibility ||
      compatibility_level == prboom_4_compatibility) {
    switch (an) {
      case -4096:
        xa = finetangent[2048]; // finecosine[-4096]
        ya = finetangent[0];              // finesine[-4096]
      break;
      case -3072:
        xa = finetangent[3072]; // finecosine[-3072]
        ya = finetangent[1024];           // finesine[-3072]
      break;
      case -2048:
        xa = finesine[0];       // finecosine[-2048]
        ya = finetangent[2048];           // finesine[-2048]
      break;
      case -1024:
        xa = finesine[1024];    // finecosine[-1024]
        ya = finetangent[3072];           // finesine[-1024]
      break;
      case 1024:
      case 2048:
      case 3072:
      case 4096:
      case 0:
      break; /* correct angles set above */
      default:
        I_Error("(%d) G_CheckSpot: unexpected angle %d (%d)\n",
          gametic, an, mthing->angle
        );
        break;
    }
  }

  mo = P_SpawnMobj(x + 20 * xa, y + 20 * ya, ss->sector->floorheight, MT_TFOG);

  if (P_GetConsolePlayer()->viewz != 1) {
    S_StartSound(mo, sfx_telept);  // don't start sound on first frame
  }

  return true;
}

// G_DeathMatchSpawnPlayer
// Spawns a player at one of the random death match spots
// called at level load and each death
//
void G_DeathMatchSpawnPlayer(player_t *player) {
  int selections = deathmatch_p - deathmatchstarts;

  /*
  if (selections < MAXPLAYERS) {
    I_Error("G_DeathMatchSpawnPlayer: Only %i deathmatch spots, %d required",
      selections, MAXPLAYERS
    );
  }
  */

  for (int j = 0; j < 20; j++) {
    int i = P_Random(pr_dmspawn) % selections;

    if (G_CheckSpot(player, &deathmatchstarts[i])) {
      deathmatchstarts[i].type = PL_GetVanillaNum(player) + 1;
      PL_Spawn(player, &deathmatchstarts[i % num_deathmatchstarts]);
      return;
    }
  }

  // no good spot, so the player will probably get stuck
  PL_Spawn(player, &playerstarts[(player->id - 1) % num_playerstarts]);

  // [CG] Nah, blow up whatever's there
  if (MULTINET) {
    P_StompSpawnPointBlockers(player->mo);
  }
}

/* vi: set et ts=2 sw=2: */
