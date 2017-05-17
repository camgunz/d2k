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


#ifndef G_GAME_H__
#define G_GAME_H__

enum buttoncode_e;
typedef enum buttoncode_e buttoncode_t;

struct event_s;
typedef struct event_s event_t;

struct mapthing_s;
typedef struct mapthing_s mapthing_t;

// CPhipps - Make savedesciption visible in wider scope
#define SAVEDESCLEN 32

#define DEMOMARKER 0x80

// killough 2/28/98: A ridiculously large number
// of players, the most you'll ever need in a demo
// or savegame. This is used to prevent problems, in
// case more players in a game are supported later.
#define MIN_MAXPLAYERS 32

// CG 04/06/2014: The old number of options is 14
#define OLD_GAME_OPTION_SIZE 14
// killough 5/2/98: number of bytes reserved for saving options
#define GAME_OPTION_SIZE 64

#define MAX_NAME_LENGTH 255

//e6y
#define RDH_SAFE        0x00000001
#define RDH_SKIP_HEADER 0x00000002

typedef enum {
  doom_12_compatibility,   /* Doom v1.2 */
  doom_1666_compatibility, /* Doom v1.666 */
  doom2_19_compatibility,  /* Doom & Doom 2 v1.9 */
  ultdoom_compatibility,   /* Ultimate Doom and Doom95 */
  finaldoom_compatibility,     /* Final Doom */
  dosdoom_compatibility,     /* DosDoom 0.47 */
  tasdoom_compatibility,     /* TASDoom */
  boom_compatibility_compatibility,      /* Boom's compatibility mode */
  boom_201_compatibility,                /* Boom v2.01 */
  boom_202_compatibility,                /* Boom v2.02 */
  lxdoom_1_compatibility,                /* LxDoom v1.3.2+ */
  mbf_compatibility,                     /* MBF */
  prboom_1_compatibility,                /* PrBoom 2.03beta? */
  prboom_2_compatibility,                /* PrBoom 2.1.0-2.1.1 */
  prboom_3_compatibility,                /* PrBoom 2.2.x */
  prboom_4_compatibility,                /* PrBoom 2.3.x */
  prboom_5_compatibility,                /* PrBoom 2.4.0 */
  prboom_6_compatibility,                /* Latest PrBoom */
  d2k_0_compatibility,                   /* D2K compatibility, v0 */
  MAX_COMPATIBILITY_LEVEL,               /* Must be last entry */
  /* Aliases follow */
  boom_compatibility = boom_201_compatibility, /* Alias used by G_Compatibility */
  best_compatibility = prboom_6_compatibility,
  d2k_compatibility  = d2k_0_compatibility,
} complevel_t_e;

typedef int complevel_t;

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

#define comperr(i) (default_comperr[i] && !demorecording && \
                                          !demoplayback && \
                                          !democontinue && \
                                          !netgame)
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

// -------------------------
// Status flags for refresh.
//

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

// The next map
extern int next_map;

//-----------------------------
// Internal parameters, fixed.
// These are set by the engine, and not changed
//  according to user inputs. Partly load from
//  WAD, partly set at startup time.

extern  int   gametic;

//e6y
extern  bool realframe;

extern  int       upmove;

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

// extern  ticcmd_t   netcmds[][BACKUPTICS];
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

// Description to save in savegame
extern char savedescription[SAVEDESCLEN];

// killough 5/2/98: moved from m_misc.c:

extern bool forced_loadgame;
extern bool command_loadgame;
extern int  totalleveltimes;

//e6y
extern bool  democontinue;
extern char *demo_continue_name;

/* CG: This is set to true when graphics have been initialized */
extern bool graphics_initialized;

extern int  defaultskill;   //jff 3/24/98 default skill
extern bool haswolflevels;  //jff 4/18/98 wolf levels present

extern GQueue *corpse_queue;
extern int     corpse_queue_size; // killough 2/8/98: adustable corpse limit

// killough 5/2/98: moved from d_deh.c:
// Par times (new item with BOOM) - from g_game.c
extern int pars[5][10];  // hardcoded array size
extern int cpars[];      // hardcoded array size

/* cph - compatibility level strings */
extern const char *comp_lev_str[];

// e6y
// There is a new command-line switch "-shorttics".
// This makes it possible to practice routes and tricks
// (e.g. glides, where this makes a significant difference)
// with the same mouse behaviour as when recording,
// but without having to be recording every time.
extern int shorttics;

//e6y: for r_demo.c
extern int longtics;
extern int bytes_per_tic;

extern int speed_step;
extern int autorun;           // always running?                   // phares

extern time_t level_start_time; // CG

extern gamestate_t gamestate;
extern gamestate_t prevgamestate;
extern gamestate_t oldgamestate;
extern gamestate_t wipegamestate; // wipegamestate can be set to -1
                                  //  to force a wipe on the next draw

void G_CheckDemoContinue(void);
void G_SetSpeed(void);

// killough 5/15/98: forced loadgames
void G_ForcedLoadGame(void);
void G_DoSaveGame(bool menu);
void G_DoLoadGame(void);

// Called by M_Responder.
void G_SaveGame(int slot, char *description);

// killough 5/15/98
void G_LoadGame(int slot, bool is_command);

/* killough 3/22/98: sets savegame filename */
int G_SaveGameName(char *name, size_t size, int slot, bool demoplayback);

bool G_Responder(event_t *ev);
bool G_CheckDemoStatus(void);
void G_ClearCorpses(void);
void G_DeathMatchSpawnPlayer(int playernum);
void G_InitNew(skill_t skill, int episode, int map);
void G_DeferedInitNew(skill_t skill, int episode, int map);
void G_DeferedPlayDemo(const char *demo); // CPhipps - const
void G_BeginRecording(void);

// CPhipps - const on these string params
void G_RecordDemo(const char *name);          // Only called by startup code.

void G_ExitLevel(void);
void G_SecretExitLevel(void);
void G_WorldDone(void);
void G_EndGame(void); /* cph - make m_menu.c call a G_* function for this */
void G_Ticker(void);
void G_Drawer(void);
void G_ReloadDefaults(void);     // killough 3/1/98: loads game defaults
void G_SetFastParms(int);        // killough 4/10/98: sets -fast parameters
void G_DoNewGame(void);
void G_DoReborn(int playernum);
void G_DoPlayDemo(void);
void G_DoCompleted(void);
void G_DoWorldDone(void);
void G_DoLoadLevel(void);
void G_Compatibility(void);
void G_ReadOptions(unsigned char game_options[]);
void G_WriteOptions(unsigned char game_options[]);
void G_PlayerReborn(int player);
void G_RestartLevel(void); // CPhipps - menu involked level restart
void G_DoVictory(void);
void G_ChangedPlayerColour(int pn, int cl); // CPhipps - On-the-fly player colour changing
void G_MakeSpecialEvent(buttoncode_t bc, ...); /* cph - new event stuff */
void G_SetGameState(gamestate_t new_gamestate);
void G_SetPrevGameState(gamestate_t new_prevgamestate);
void G_SetOldGameState(gamestate_t new_oldgamestate);
void G_SetWipeGameState(gamestate_t new_wipegamestate);
void G_ResetGameState(void);

gamestate_t  G_GetGameState(void);

gameaction_t G_GetGameAction(void);
void         G_SetGameAction(gameaction_t new_gameaction);

const unsigned char* G_ReadDemoHeaderEx(const unsigned char *demo_p,
                                        size_t size,
                                        unsigned int params);
const unsigned char* G_ReadDemoHeader(const unsigned char *demo_p,
                                      size_t size);
void G_CalculateDemoParams(const unsigned char *demo_p);

#endif

/* vi: set et ts=2 sw=2: */
