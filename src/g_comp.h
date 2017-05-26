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


#ifndef G_COMP_H__
#define G_COMP_H__

typedef int complevel_t;

typedef enum {
  doom_12_compatibility,            /* Doom v1.2 */
  doom_1666_compatibility,          /* Doom v1.666 */
  doom2_19_compatibility,           /* Doom & Doom 2 v1.9 */
  ultdoom_compatibility,            /* Ultimate Doom and Doom95 */
  finaldoom_compatibility,          /* Final Doom */
  dosdoom_compatibility,            /* DosDoom 0.47 */
  tasdoom_compatibility,            /* TASDoom */
  boom_compatibility_compatibility, /* Boom's compatibility mode */
  boom_201_compatibility,           /* Boom v2.01 */
  boom_202_compatibility,           /* Boom v2.02 */
  lxdoom_1_compatibility,           /* LxDoom v1.3.2+ */
  mbf_compatibility,                /* MBF */
  prboom_1_compatibility,           /* PrBoom 2.03beta? */
  prboom_2_compatibility,           /* PrBoom 2.1.0-2.1.1 */
  prboom_3_compatibility,           /* PrBoom 2.2.x */
  prboom_4_compatibility,           /* PrBoom 2.3.x */
  prboom_5_compatibility,           /* PrBoom 2.4.0 */
  prboom_6_compatibility,           /* Latest PrBoom */
  d2k_0_compatibility,              /* D2K compatibility, v0 */
  MAX_COMPATIBILITY_LEVEL,          /* Must be last entry */
  /* Aliases follow */
  boom_compatibility = boom_201_compatibility, /* Alias used by G_Compatibility */
  best_compatibility = prboom_6_compatibility,
  d2k_compatibility  = d2k_0_compatibility,
} complevel_t_e;

// CPhipps - new compatibility handling
extern complevel_t compatibility_level;
extern complevel_t default_compatibility_level;

// CPhipps - old compatibility testing flags aliased to new handling
#define compatibility (compatibility_level <= boom_compatibility_compatibility)
#define demo_compatibility \
  (compatibility_level < boom_compatibility_compatibility)
#define mbf_features (compatibility_level >= mbf_compatibility)

#define comperr(i) (default_comperr[i] && !demorecording && \
                                          !demoplayback && \
                                          !democontinue && \
                                          !netgame)

// -------------------------------------------
// killough 10/98: compatibility vector

enum {
  comp_telefrag,
  comp_dropoff,
  comp_vile,
  comp_pain,
  comp_skull,
  comp_blazing,
  comp_doorlight,
  comp_model,
  comp_god,
  comp_falloff,
  comp_floors,
  comp_skymap,
  comp_pursuit,
  comp_doorstuck,
  comp_staylift,
  comp_zombie,
  comp_stairs,
  comp_infcheat,
  comp_zerotags,
  comp_moveblock,
  comp_respawn,  /* cph - this is the inverse of comp_respawnfix from eternity */
  comp_sound,
  comp_666,
  comp_soul,
  comp_maskedanim,

  //e6y
  comp_ouchface,
  comp_maxhealth,
  comp_translucency,

  COMP_NUM,      /* cph - should be last in sequence */
  COMP_TOTAL=32  // Some extra room for additional variables
};

enum {
  comperr_zerotag,
  comperr_passuse,
  comperr_hangsolid,
  comperr_blockmap,
  comperr_allowjump,

  COMPERR_NUM
};

extern int comp[COMP_TOTAL];
extern int default_comp[COMP_TOTAL];
extern int default_comperr[COMPERR_NUM];

/* cph - compatibility level strings */
extern const char *comp_lev_str[];

void G_Compatibility(void);

#endif

/* vi: set et ts=2 sw=2: */
