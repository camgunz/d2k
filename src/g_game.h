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

struct player_s;
typedef struct player_s player_t;

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

// -------------------------------------------
// Selected skill type, map etc.
//

typedef enum {
  sk_none = -1, //jff 3/24/98 create unpicked skill setting
  sk_baby = 0,
  sk_easy,
  sk_medium,
  sk_hard,
  sk_nightmare
} skill_t;

// The current state of the game: whether we are playing, gazing
// at the intermission screen, the game final animation, or a demo.

typedef enum {
  GS_BAD = -1,
  GS_LEVEL = 0,
  GS_INTERMISSION,
  GS_FINALE,
  GS_DEMOSCREEN
} gamestate_t;

typedef enum {
  ga_nothing,
  ga_loadlevel,
  ga_newgame,
  ga_loadgame,
  ga_savegame,
  ga_playdemo,
  ga_completed,
  ga_victory,
  ga_worlddone,
} gameaction_t;


// ------------------------
// Command line parameters.
//

extern bool nomonsters; // checkparm of -nomonsters
extern bool respawnparm;  // checkparm of -respawn
extern bool fastparm; // checkparm of -fast
extern bool devparm;  // DEBUG: launched with -devparm

// -----------------------------------------------------
// Game Mode - identify IWAD as shareware, retail etc.
//

extern const char *doomverstr;

extern int  default_translucency; // config file says           // phares
extern bool general_translucency; // true if translucency is ok // phares

// Defaults for menu, methinks.
extern skill_t startskill;
extern int     startepisode;
extern int     startmap;

extern bool autostart;

extern gameaction_t gameaction;

// Selected by user.
extern skill_t gameskill;
extern int     gameepisode;
extern int     gamemap;

// Nightmare mode flag, single player.
extern bool respawnmonsters;

// Flag: true only if started as net deathmatch.
// An enum might handle altdeath/cooperative better.
// Actually, this is an int now (altdeath)
extern int deathmatch;

extern bool bfgedition;

// -------------------------
// Status flags for refresh.
//

extern bool paused;        // Game Pause?
extern bool nodrawers;
extern bool noblit;

// This one is related to the 3-screen display mode.
// ANG90 = left side, ANG270 = right
extern int viewangleoffset;

// -------------------------------------
// Scores, rating.
// Statistics on a given map, for intermission.
//
extern int totalkills;
extern int totallive;
extern int totalitems;
extern int totalsecret;
extern int show_alive;

// Timer, for scores.
extern int basetic;    /* killough 9/29/98: levelstarttic, adjusted */
extern int leveltime;  // tics in game play for par

extern int  starttime;

// --------------------------------------
// DEMO playback/recording related stuff.

// The next map
extern int next_map;

//-----------------------------
// Internal parameters, fixed.
// These are set by the engine, and not changed
//  according to user inputs. Partly load from
//  WAD, partly set at startup time.

extern int gametic;

//e6y
extern bool realframe;

extern int upmove;

//-----------------------------------------
// Internal parameters, used for engine.
//

// File handling stuff.
extern FILE *debugfile;

// if true, load all graphics at level load
extern int precache;

// debug flag to cancel adaptiveness
extern bool singletics;

extern int bodyqueslot;

// Needed to store the number of the dummy sky flat.
// Used for rendering, as well as tracking projectiles etc.

extern int skyflatnum;

extern int maketic;

// Networking and tick handling related.
#define BACKUPTICS 12

// extern ticcmd_t   netcmds[][BACKUPTICS];
extern int ticdup;

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
extern int dogs; // killough 7/19/98: Marine's best friend :)
extern int default_dogs;
extern int dog_jumping; // killough 10/98
extern int default_dog_jumping;
#endif

/* killough 8/8/98: distance friendly monsters tend to stay from player */
extern int distfriend;
extern int default_distfriend;

/* killough 9/8/98: whether monsters are allowed to strafe or retreat */
extern int monster_backing;
extern int default_monster_backing;

/* killough 9/9/98: whether monsters intelligently avoid hazards */
extern int monster_avoid_hazards;
extern int default_monster_avoid_hazards;

/* killough 10/98: whether monsters are affected by friction */
extern int monster_friction;
extern int default_monster_friction;

/* killough 9/9/98: whether monsters help friends */
extern int help_friends;
extern int default_help_friends;

extern int flashing_hom; // killough 10/98

extern int doom_weapon_toggles;   // killough 10/98

/* killough 7/19/98: whether monsters should fight against each other */
extern int monster_infighting;
extern int default_monster_infighting;

extern int monkeys;
extern int default_monkeys;

extern int HelperThing;          // type of thing to use for helper

// killough 5/2/98: moved from m_misc.c:

extern bool forced_loadgame;
extern bool command_loadgame;
extern int  totalleveltimes;

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

// e6y
// There is a new command-line switch "-shorttics".
// This makes it possible to practice routes and tricks
// (e.g. glides, where this makes a significant difference)
// with the same mouse behaviour as when recording,
// but without having to be recording every time.
extern int shorttics;

extern int speed_step;
extern int autorun;           // always running?                   // phares

extern time_t level_start_time; // CG

extern gamestate_t gamestate;
extern gamestate_t prevgamestate;
extern gamestate_t oldgamestate;
extern gamestate_t wipegamestate; // wipegamestate can be set to -1
                                  //  to force a wipe on the next draw

void G_SkipDemoStart(void);
void G_SkipDemoStop(void);
void G_SkipDemoCheck(void);
int  G_ReloadLevel(void);
int  G_GotoNextLevel(void);

// killough 5/15/98: forced loadgames
void G_ForcedLoadGame(void);
void G_DoSaveGame(bool menu);
void G_DoLoadGame(void);

bool G_Responder(event_t *ev);
void G_DeathMatchSpawnPlayer(player_t *player);
void G_InitNew(skill_t skill, int episode, int map);
void G_DeferedInitNew(skill_t skill, int episode, int map);

// CPhipps - const on these string params

void G_ExitLevel(void);
void G_SecretExitLevel(void);
void G_WorldDone(void);
void G_EndGame(void); /* cph - make m_menu.c call a G_* function for this */
void G_Ticker(void);
void G_Drawer(void);
void G_ReloadDefaults(void);     // killough 3/1/98: loads game defaults
void G_SetFastParms(int);        // killough 4/10/98: sets -fast parameters
void G_DoNewGame(void);
void G_DoReborn(player_t *player);
void G_DoPlayDemo(void);
void G_DoCompleted(void);
void G_DoWorldDone(void);
void G_DoLoadLevel(void);
void G_PlayerReborn(player_t *player);
void G_RestartLevel(void); // CPhipps - menu involked level restart
void G_DoVictory(void);
void G_ChangedPlayerColour(int pn, int cl); // CPhipps - On-the-fly player colour changing
void G_MakeSpecialEvent(buttoncode_t bc, ...); /* cph - new event stuff */
void G_SetGameState(gamestate_t new_gamestate);
void G_SetPrevGameState(gamestate_t new_prevgamestate);
void G_SetOldGameState(gamestate_t new_oldgamestate);
void G_SetWipeGameState(gamestate_t new_wipegamestate);
void G_ResetGameState(void);

gamestate_t G_GetGameState(void);

gameaction_t G_GetGameAction(void);
void         G_SetGameAction(gameaction_t new_gameaction);

#endif

/* vi: set et ts=2 sw=2: */
