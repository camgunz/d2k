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

#include "g_comp.h"
#include "g_game.h"

void e6y_G_Compatibility(void);

// comp_options_by_version removed - see G_Compatibility
#if 0
static unsigned char map_old_comp_levels[] = {
  0, 1, 2, 4, 5, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16
};
#endif

const char * comp_lev_str[MAX_COMPATIBILITY_LEVEL] = {
  "Doom v1.2",
  "Doom v1.666",
  "Doom/Doom2 v1.9",
  "Ultimate Doom/Doom95",
  "Final Doom",
  "early DosDoom",
  "TASDoom",
  "\"boom compatibility\"",
  "boom v2.01",
  "boom v2.02",
  "lxdoom v1.3.2+",
  "MBF",
  "PrBoom 2.03beta",
  "PrBoom v2.1.0-2.1.1",
  "PrBoom v2.1.2-v2.2.6",
  "PrBoom v2.3.x",
  "PrBoom 2.4.0",
  "Current PrBoom"
};

// CPhipps - compatibility vars
complevel_t compatibility_level;
complevel_t default_compatibility_level;

int comp[COMP_TOTAL];
int default_comp[COMP_TOTAL];    // killough 10/98
int default_comperr[COMPERR_NUM];

void G_Compatibility(void) {
  unsigned int i;

  static const struct {
    complevel_t fix; // level at which fix/change was introduced
    complevel_t opt; // level at which fix/change was made optional
  } levels[] = {
    // comp_telefrag - monsters used to telefrag only on MAP30, now they do it for spawners only
    { mbf_compatibility, mbf_compatibility },
    // comp_dropoff - MBF encourages things to drop off of overhangs
    { mbf_compatibility, mbf_compatibility },
    // comp_vile - original Doom archville bugs like ghosts
    { boom_compatibility, mbf_compatibility },
    // comp_pain - original Doom limits Pain Elementals from spawning too many skulls
    { boom_compatibility, mbf_compatibility },
    // comp_skull - original Doom let skulls be spit through walls by Pain Elementals
    { boom_compatibility, mbf_compatibility },
    // comp_blazing - original Doom duplicated blazing door sound
    { boom_compatibility, mbf_compatibility },
    // e6y: "Tagged doors don't trigger special lighting" handled wrong
    // http://sourceforge.net/tracker/index.php?func=detail&aid=1411400&group_id=148658&atid=772943
    // comp_doorlight - MBF made door lighting changes more gradual
    { boom_compatibility, mbf_compatibility },
    // comp_model - improvements to the game physics
    { boom_compatibility, mbf_compatibility },
    // comp_god - fixes to God mode
    { boom_compatibility, mbf_compatibility },
    // comp_falloff - MBF encourages things to drop off of overhangs
    { mbf_compatibility, mbf_compatibility },
    // comp_floors - fixes for moving floors bugs
    { boom_compatibility_compatibility, mbf_compatibility },
    // comp_skymap
    { mbf_compatibility, mbf_compatibility },
    // comp_pursuit - MBF AI change, limited pursuit?
    { mbf_compatibility, mbf_compatibility },
    // comp_doorstuck - monsters stuck in doors fix
    { boom_202_compatibility, mbf_compatibility },
    // comp_staylift - MBF AI change, monsters try to stay on lifts
    { mbf_compatibility, mbf_compatibility },
    // comp_zombie - prevent dead players triggering stuff
    { lxdoom_1_compatibility, mbf_compatibility },
    // comp_stairs - see p_floor.c
    { boom_202_compatibility, mbf_compatibility },
    // comp_infcheat - FIXME
    { mbf_compatibility, mbf_compatibility },
    // comp_zerotags - allow zero tags in wads */
    { boom_compatibility, mbf_compatibility },
    // comp_moveblock - enables keygrab and mancubi shots going thru walls
    { lxdoom_1_compatibility, prboom_2_compatibility },
    // comp_respawn - objects which aren't on the map at game start respawn at (0,0)
    { prboom_2_compatibility, prboom_2_compatibility },
    // comp_sound - see s_sound.c
    { boom_compatibility_compatibility, prboom_3_compatibility },
    // comp_666 - emulate pre-Ultimate BossDeath behaviour
    { ultdoom_compatibility, prboom_4_compatibility },
    // comp_soul - enables lost souls bouncing (see P_ZMovement)
    { prboom_4_compatibility, prboom_4_compatibility },
    // comp_maskedanim - 2s mid textures don't animate
    { doom_1666_compatibility, prboom_4_compatibility },
    //e6y
    // comp_ouchface - Use Doom's buggy "Ouch" face code
    { prboom_1_compatibility, prboom_6_compatibility },
    // comp_maxhealth - Max Health in DEH applies only to potions
    { boom_compatibility_compatibility, prboom_6_compatibility },
    // comp_translucency - No predefined translucency for some things
    { boom_compatibility_compatibility, prboom_6_compatibility },
  };

  if (sizeof(levels) / sizeof(*levels) != COMP_NUM) {
    I_Error("G_Compatibility: consistency error");
  }

  for (i = 0; i < sizeof(levels) / sizeof(*levels); i++) {
    if (compatibility_level < levels[i].opt) {
      comp[i] = (compatibility_level < levels[i].fix);
    }
  }

  e6y_G_Compatibility();//e6y

  if (!mbf_features) {
    monster_infighting = 1;
    monster_backing = 0;
    monster_avoid_hazards = 0;
    monster_friction = 0;
    help_friends = 0;

#ifdef DOGS
    dogs = 0;
    dog_jumping = 0;
#endif

    monsters_climb = 0;
  }
}

/* vi: set et ts=2 sw=2: */
