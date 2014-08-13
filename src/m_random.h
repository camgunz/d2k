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


#ifndef M_RANDOM_H__
#define M_RANDOM_H__

#include "doomtype.h"

// killough 1/19/98: rewritten to use to use a better random number generator
// in the new engine, although the old one is available for compatibility.

// killough 2/16/98:
//
// Make every random number generator local to each control-equivalent block.
// Critical for demo sync. Changing the order of this list breaks all previous
// versions' demos. The random number generators are made local to reduce the
// chances of sync problems. In Doom, if a single random number generator call
// was off, it would mess up all random number generators. This reduces the
// chances of it happening by making each RNG local to a control flow block.
//
// Notes to developers: if you want to reduce your demo sync hassles, follow
// this rule: for each call to P_Random you add, add a new class to the enum
// type below for each block of code which calls P_Random. If two calls to
// P_Random are not in "control-equivalent blocks", i.e. there are any cases
// where one is executed, and the other is not, put them in separate classes.
//
// Keep all current entries in this list the same, and in the order
// indicated by the #'s, because they're critical for preserving demo
// sync. Do not remove entries simply because they become unused later.

typedef enum {
  pr_skullfly,                // #0
  pr_damage,                  // #1
  pr_crush,                   // #2
  pr_genlift,                 // #3
  pr_killtics,                // #4
  pr_damagemobj,              // #5
  pr_painchance,              // #6
  pr_lights,                  // #7
  pr_explode,                 // #8
  pr_respawn,                 // #9
  pr_lastlook,                // #10
  pr_spawnthing,              // #11
  pr_spawnpuff,               // #12
  pr_spawnblood,              // #13
  pr_missile,                 // #14
  pr_shadow,                  // #15
  pr_plats,                   // #16
  pr_punch,                   // #17
  pr_punchangle,              // #18
  pr_saw,                     // #19
  pr_plasma,                  // #20
  pr_gunshot,                 // #21
  pr_misfire,                 // #22
  pr_shotgun,                 // #23
  pr_bfg,                     // #24
  pr_slimehurt,               // #25
  pr_dmspawn,                 // #26
  pr_missrange,               // #27
  pr_trywalk,                 // #28
  pr_newchase,                // #29
  pr_newchasedir,             // #30
  pr_see,                     // #31
  pr_facetarget,              // #32
  pr_posattack,               // #33
  pr_sposattack,              // #34
  pr_cposattack,              // #35
  pr_spidrefire,              // #36
  pr_troopattack,             // #37
  pr_sargattack,              // #38
  pr_headattack,              // #39
  pr_bruisattack,             // #40
  pr_tracer,                  // #41
  pr_skelfist,                // #42
  pr_scream,                  // #43
  pr_brainscream,             // #44
  pr_cposrefire,              // #45
  pr_brainexp,                // #46
  pr_spawnfly,                // #47
  pr_misc,                    // #48
  pr_all_in_one,              // #49
  /* CPhipps - new entries from MBF, mostly unused for now */
  pr_opendoor,                // #50
  pr_targetsearch,            // #51
  pr_friends,                 // #52
  pr_threshold,               // #53
  pr_skiptarget,              // #54
  pr_enemystrafe,             // #55
  pr_avoidcrush,              // #57
  pr_stayonlift,              // #57
  pr_helpfriend,              // #58
  pr_dropoff,                 // #59
  pr_randomjump,              // #60
  pr_defect,                  // #61  // Start new entries -- add new entries below

  // End of new entries
  NUMPRCLASS               // MUST be last item in list
} pr_class_t;

// The random number generator's state.
typedef struct {
  unsigned int seed[NUMPRCLASS];       // Each block's random seed
  int rndindex, prndindex;             // For compatibility support
} rng_t;

extern rng_t rng;                      // The rng's state

extern unsigned int rngseed;           // The starting seed (not part of state)

// As M_Random, but used by the play simulation.
int P_Random(pr_class_t DA(const char *, int));

#ifdef INSTRUMENTED
#define P_Random(a) (P_Random) (a, __FILE__,__LINE__)
#endif

// Returns a number from 0 to 255,
#define M_Random() P_Random(pr_misc)

/* CG: 08/13/2014: A non-sync-critical RNG */
int D_Random(void);

// Fix randoms for demos.
void M_ClearRandom(void);

#endif

/* vi: set et ts=2 sw=2: */

