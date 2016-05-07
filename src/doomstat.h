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


#ifndef D_STATE_H__
#define D_STATE_H__

// We need the playr data structure as well.
#include "d_player.h"

#ifdef __GNUG__
#pragma interface
#endif

// ------------------------
// Command line parameters.
//

extern  bool nomonsters; // checkparm of -nomonsters
extern  bool respawnparm;  // checkparm of -respawn
extern  bool fastparm; // checkparm of -fast
extern  bool devparm;  // DEBUG: launched with -devparm

// -----------------------------------------------------
// Game Mode - identify IWAD as shareware, retail etc.
//

extern GameMode_t gamemode;
extern GameMission_t  gamemission;
extern const char *doomverstr;

// Set if homebrew PWAD stuff has been added.
extern  bool modifiedgame;

// CPhipps - new compatibility handling
extern complevel_t compatibility_level, default_compatibility_level;

// CPhipps - old compatibility testing flags aliased to new handling
#define compatibility (compatibility_level<=boom_compatibility_compatibility)
#define demo_compatibility (compatibility_level < boom_compatibility_compatibility)
#define mbf_features (compatibility_level>=mbf_compatibility)

// v1.1-like pitched sounds
extern int pitched_sounds;        // killough

extern int     default_translucency; // config file says           // phares
extern bool general_translucency; // true if translucency is ok // phares

extern int demo_insurance, default_demo_insurance;      // killough 4/5/98

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

extern int comp[COMP_TOTAL], default_comp[COMP_TOTAL];
extern int /*comperr[COMPERR_NUM], */default_comperr[COMPERR_NUM];

// -------------------------------------------
// Language.
extern  Language_t   language;

// -------------------------------------------
// Selected skill type, map etc.
//

// Defaults for menu, methinks.
extern  skill_t startskill;
extern  int     startepisode;
extern  int     startmap;

extern  bool autostart;

// Selected by user.
extern  skill_t gameskill;
extern  int     gameepisode;
extern  int     gamemap;

// Nightmare mode flag, single player.
extern  bool respawnmonsters;

// Flag: true only if started as net deathmatch.
// An enum might handle altdeath/cooperative better.
// Actually, this is an int now (altdeath)
extern int deathmatch;

// ------------------------------------------
// Internal parameters for sound rendering.
// These have been taken from the DOS version,
//  but are not (yet) supported with Linux
//  (e.g. no sound volume adjustment with menu.

// These are not used, but should be (menu).
// From m_menu.c:
//  Sound FX volume has default, 0 - 15
//  Music volume has default, 0 - 15
// These are multiplied by 8.
extern int snd_SfxVolume;      // maximum volume for sound
extern int snd_MusicVolume;    // maximum volume for music

// CPhipps - screen parameters
extern unsigned int desired_screenwidth, desired_screenheight;

// -------------------------
// Status flags for refresh.
//

enum automapmode_e {
  am_active = 1,  // currently shown
  am_overlay= 2,  // covers the screen, i.e. not overlay mode
  am_rotate = 4,  // rotates to the player facing direction
  am_follow = 8,  // keep the player centred
  am_grid   =16,  // show grid
};
extern enum automapmode_e automapmode; // Mode that the automap is in

enum menuactive_e {
  mnact_inactive, // no menu
  mnact_float, // doom-style large font menu, doesn't overlap anything
  mnact_full, // boom-style small font menu, may overlap status bar
};
extern enum menuactive_e menuactive; // Type of menu overlaid, if any

extern  bool paused;        // Game Pause?
extern  bool nodrawers;
extern  bool noblit;

// This one is related to the 3-screen display mode.
// ANG90 = left side, ANG270 = right
extern  int viewangleoffset;

// Player taking events, and displaying.
extern  int consoleplayer;
extern  int displayplayer;

// -------------------------------------
// Scores, rating.
// Statistics on a given map, for intermission.
//
extern  int totalkills, totallive;
extern  int totalitems;
extern  int totalsecret;
extern  int show_alive;

// Timer, for scores.
extern  int basetic;    /* killough 9/29/98: levelstarttic, adjusted */
extern  int leveltime;  // tics in game play for par

// --------------------------------------
// DEMO playback/recording related stuff.

extern  bool usergame;
extern  bool demoplayback;
extern  bool demorecording;
extern  int demover;

// Quit after playing a demo from cmdline.
extern  bool   singledemo;
// Print timing information after quitting.  killough
extern  bool   timingdemo;
// Run tick clock at fastest speed possible while playing demo.  killough
extern  bool   fastdemo;

//-----------------------------
// Internal parameters, fixed.
// These are set by the engine, and not changed
//  according to user inputs. Partly load from
//  WAD, partly set at startup time.

extern  int   gametic;

//e6y
extern  bool realframe;

// Bookkeeping on players - state.
extern  player_t  players[MAXPLAYERS];
extern  int       upmove;

// Alive? Disconnected?
extern  bool playeringame[MAXPLAYERS];
extern  bool realplayeringame[MAXPLAYERS];

extern  mapthing_t *deathmatchstarts;     // killough
extern  size_t     num_deathmatchstarts; // killough

extern  mapthing_t *deathmatch_p;

// Player spawn spots.
extern  mapthing_t playerstarts[];

// Intermission stats.
// Parameters for world map / intermission.
extern wbstartstruct_t wminfo;

//-----------------------------------------
// Internal parameters, used for engine.
//

// File handling stuff.
extern FILE *debugfile;

// if true, load all graphics at level load
extern int precache;


extern int mouseSensitivity_horiz; // killough
extern int mouseSensitivity_vert;

// debug flag to cancel adaptiveness
extern bool singletics;

extern int bodyqueslot;

// Needed to store the number of the dummy sky flat.
// Used for rendering, as well as tracking projectiles etc.

extern int skyflatnum;

extern int maketic;

// Networking and tick handling related.
#define BACKUPTICS              12

extern  ticcmd_t   netcmds[][BACKUPTICS];
extern  int        ticdup;

//-----------------------------------------------------------------------------

extern int allow_pushers;         // MT_PUSH Things    // phares 3/10/98
extern int default_allow_pushers;

extern int variable_friction;  // ice & mud            // phares 3/10/98
extern int default_variable_friction;

extern int monsters_remember;                          // killough 3/1/98
extern int default_monsters_remember;

extern int weapon_recoil;          // weapon recoil    // phares
extern int default_weapon_recoil;

extern int player_bobbing;  // whether player bobs or not   // phares 2/25/98
extern int default_player_bobbing;  // killough 3/1/98: make local to each game

extern int leave_weapons;         // leave picked-up weapons behind?
extern int default_leave_weapons; // CG 08/19/2014

#ifdef DOGS
extern int dogs, default_dogs;     // killough 7/19/98: Marine's best friend :)
extern int dog_jumping, default_dog_jumping;   // killough 10/98
#endif

/* killough 8/8/98: distance friendly monsters tend to stay from player */
extern int distfriend, default_distfriend;

/* killough 9/8/98: whether monsters are allowed to strafe or retreat */
extern int monster_backing, default_monster_backing;

/* killough 9/9/98: whether monsters intelligently avoid hazards */
extern int monster_avoid_hazards, default_monster_avoid_hazards;

/* killough 10/98: whether monsters are affected by friction */
extern int monster_friction, default_monster_friction;

/* killough 9/9/98: whether monsters help friends */
extern int help_friends, default_help_friends;

extern int flashing_hom; // killough 10/98

extern int doom_weapon_toggles;   // killough 10/98

/* killough 7/19/98: whether monsters should fight against each other */
extern int monster_infighting, default_monster_infighting;

extern int monkeys, default_monkeys;

extern int HelperThing;          // type of thing to use for helper

#endif

/* vi: set et ts=2 sw=2: */

