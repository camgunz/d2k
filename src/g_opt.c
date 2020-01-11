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

#include "g_opt.h"

// These functions are used to read and write game-specific options in demos
// and savegames so that demo sync is preserved and savegame restoration is
// complete. Not all options (for example "compatibility"), however, should
// be loaded and saved here. It is extremely important to use the same
// positions as before for the variables, so if one becomes obsolete, the
// byte(s) should still be skipped over or padded with 0's.
// Lee Killough 3/1/98

void G_WriteOptions(unsigned char game_options[]) {
  int i = 0;

  memset(game_options, 0, GAME_OPTION_SIZE * sizeof(unsigned char));

  game_options[i++] = monsters_remember; // part of monster AI
  game_options[i++] = variable_friction; // ice & mud
  game_options[i++] = weapon_recoil;     // weapon recoil
  game_options[i++] = allow_pushers;     // MT_PUSH Things

  game_options[i++] = leave_weapons;

  game_options[i++] = player_bobbing;    // whether player bobs or not

  // killough 3/6/98: add parameters to savegame, move around some in demos
  game_options[i++] = respawnparm;
  game_options[i++] = fastparm;
  game_options[i++] = nomonsters;

  game_options[i++] = demo_insurance;    // killough 3/31/98

  // killough 3/26/98: Added rngseed. 3/31/98: moved here
  game_options[i++] = (unsigned char)((rngseed >> 24) & 0xff);
  game_options[i++] = (unsigned char)((rngseed >> 16) & 0xff);
  game_options[i++] = (unsigned char)((rngseed >>  8) & 0xff);
  game_options[i++] = (unsigned char)( rngseed        & 0xff);

  // Options new to v2.03 begin here
  if (mbf_features) {
    game_options[i++] = monster_infighting;

#ifdef DOGS
    game_options[i++] = dogs;
#else
    i++;
#endif

    i += 2;

    game_options[i++] = (distfriend >> 8) & 0xff;  // killough 8/8/98
    game_options[i++] =  distfriend       & 0xff;

    game_options[i++] = monster_backing;       // killough 9/8/98
    game_options[i++] = monster_avoid_hazards; // killough 9/9/98
    game_options[i++] = monster_friction;      // killough 10/98
    game_options[i++] = help_friends;          // killough 9/9/98

#ifdef DOGS
    game_options[i++] = dog_jumping; // killough 10/98
#else
    i++;
#endif

    game_options[i++] = monkeys;

    // killough 10/98: a compatibility vector now
    for (int o = 0; o < COMP_TOTAL; o++) {
      game_options[i + o] = (comp[o] != 0);
    }

    i += COMP_TOTAL;

    // cph 2002/07/20
    if ((compatibility_level >= prboom_2_compatibility) && forceOldBsp) {
      game_options[i++] = 1;
    }
    else {
      game_options[i++] = 0;
    }
  }
}

/* Same, but read instead of write
 * cph - const byte*'s
 */

void G_ReadOptions(unsigned char game_options[]) {
  int i = 0;

  monsters_remember = game_options[i++];
  variable_friction = game_options[i++]; // ice & mud
  weapon_recoil = game_options[i++];     // weapon recoil
  allow_pushers = game_options[i++];     // MT_PUSH Things

  leave_weapons = game_options[i++];

  player_bobbing = game_options[i++];    // Whether player bobs or not
  // killough 3/6/98: add parameters to savegame, move from demo
  respawnparm = game_options[i++];
  fastparm = game_options[i++];
  nomonsters = game_options[i++];
  demo_insurance = game_options[i++]; // killough 3/31/98
  rngseed = ((game_options[i] & 0xFF) << 24) +
            ((game_options[i + 1] & 0xFF) << 16) +
            ((game_options[i + 2] & 0xFF) <<  8) +
            ((game_options[i + 3] & 0xFF));

  i += 4;

  // Options new to v2.03
  if (mbf_features) {
    monster_infighting = game_options[i++]; // killough 7/19/98

#ifdef DOGS
    dogs = game_options[i++]; // killough 7/19/98
#else
    dogs = 0;
    i++;
#endif

    i += 2;

    distfriend = ((game_options[i] & 0xFF) << 8) +
                 ((game_options[i + 1] & 0xFF)); // killough 8/8/98
    i += 2;
    monster_backing = game_options[i++]; // killough 9/8/98
    monster_avoid_hazards = game_options[i++]; // killough 9/9/98
    monster_friction = game_options[i++]; // killough 10/98
    help_friends = game_options[i++]; // killough 9/9/98

#ifdef DOGS
    dog_jumping = game_options[i++]; // killough 10/98
#else
    dog_jumping = 0;
    i++;
#endif

    monkeys = game_options[i++];

    for (int o = 0; o < COMP_TOTAL; o++) {
      comp[o] = game_options[i + o];
    }

    i += COMP_TOTAL;

    forceOldBsp = game_options[i++]; // cph 2002/07/20
  }

  // G_Compatibility();
}

// e6y
// save/restore all data which could be changed by G_ReadDemoHeader
void G_SaveRestoreGameOptions(int save) {
  typedef struct gameoption_s {
    int type;
    int value_int;
    int *value_p;
  } gameoption_t;

  static gameoption_t gameoptions[] = {
    {1, 0, &demover},
    {1, 0, (int*)&compatibility_level},
    {1, 0, &basetic},
    {3, 0, (int*)&rngseed},

    {1, 0, (int*)&gameskill},
    {1, 0, &gameepisode},
    {1, 0, &gamemap},

    {2, 0, (int*)&deathmatch},
    {2, 0, (int*)&respawnparm},
    {2, 0, (int*)&fastparm},
    {2, 0, (int*)&nomonsters},
    {1, 0, &consoleplayer},
    {2, 0, (int*)&netgame},
    {2, 0, (int*)&netdemo},

    {1, 0, &longtics},
    {1, 0, &monsters_remember},
    {1, 0, &variable_friction},
    {1, 0, &weapon_recoil},
    {1, 0, &allow_pushers},
    {1, 0, &player_bobbing},
    {1, 0, &demo_insurance},
    {1, 0, &monster_infighting},
#ifdef DOGS
    {1, 0, &dogs},
#endif
    {1, 0, &distfriend},
    {1, 0, &monster_backing},
    {1, 0, &monster_avoid_hazards},
    {1, 0, &monster_friction},
    {1, 0, &help_friends},
#ifdef DOGS
    {1, 0, &dog_jumping},
#endif
    {1, 0, &monkeys},
  
    {2, 0, &forceOldBsp},
    {-1, -1, NULL}
  };

  static bool was_saved_once = false;
  static bool playeringame_o[VANILLA_MAXPLAYERS];
  static bool playerscheats_o[VANILLA_MAXPLAYERS];
  static int  comp_o[COMP_TOTAL];

  size_t i = 0;

  if (save) {
    was_saved_once = true;
  }
  else {
    if (!was_saved_once) {
      I_Error("G_SaveRestoreGameOptions: Trying to restore unsaved data");
    }
  }

  while (gameoptions[i].value_p) {
    switch (gameoptions[i].type) {
      case 1: //int
      case 2: //bool
      case 3: //unsigned long
        if (save)
          gameoptions[i].value_int = *gameoptions[i].value_p;
        else
          *gameoptions[i].value_p = gameoptions[i].value_int;
      break;
      default: // unrecognised type
        I_Error("G_SaveRestoreGameOptions: Unrecognised type of option");
      break;
    }

    i++;
  }

  for (i = 0; i < VANILLA_MAXPLAYERS; i++) {
    if (save) {
      player_t *player = P_PlayersLookup(i + 1);

      playeringame_o[i] = player != NULL;
      playerscheats_o[i] = player ? player->cheats : 0;
    }
    else if (playeringame_o[i]) {
      player_t *player = P_PlayersGetNew();
      player->cheats = playerscheats_o[i];
    }
  }

  for (i = 0; i < COMP_TOTAL; i++) {
    if (save) {
      comp_o[i] = comp[i];
    }
    else {
      comp[i] = comp_o[i];
    }
  }
}

/* vi: set et ts=2 sw=2: */
