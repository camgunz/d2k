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
  gameskill_none = -1, //jff 3/24/98 create unpicked skill setting
  gameskill_baby = 0,
  gameskill_easy,
  gameskill_medium,
  gameskill_hard,
  gameskill_nightmare
} gameskill_e;

// The current state of the game: whether we are playing, gazing
// at the intermission screen, the game final animation, or a demo.

typedef enum {
  gamestate_bad = -1,
  gamestate_level = 0,
  gamestate_intermission,
  gamestate_finale,
  gamestate_demo_screen
} gamestate_e;

typedef enum {
  gameaction_nothing,
  gameaction_load_level,
  gameaction_new_game,
  gameaction_load_game,
  gameaction_save_game,
  gameaction_play_demo,
  gameaction_completed,
  gameaction_victory,
  gameaction_world_done,
} gameaction_e;

typedef struct game_s {
  game_options_t options;
  uint32_t tic;
  uint32_t basetic;
  uint32_t leveltime;
  uint32_t starttime;
  gameaction_e action;
  gameskill_e skill;
  unsigned int episode;
  unsigned int map;
  bool paused;
  gamestate_e state;
  gamestate_e prev_state;
  gamestate_e old_state;
  gamestate_e wipe_state;

  //  to force a wipe on the next draw
} game_t;

void G_GameInit(game_t *game);

extern gamestate_e gamestate;
extern gamestate_e prevgamestate;
extern gamestate_e oldgamestate;
extern gamestate_e wipegamestate; // wipegamestate can be set to -1

// -----------------------------------------------------
// Game Mode - identify IWAD as shareware, retail etc.
//

extern const char *doomverstr;

extern bool autostart;

extern bool bfgedition;

// -------------------------------------
// Scores, rating.
// Statistics on a given map, for intermission.
//
extern int totalkills;
extern int totallive;
extern int totalitems;
extern int totalsecret;
extern int show_alive;

// --------------------------------------
// DEMO playback/recording related stuff.

// The next map
extern int next_map;

//-----------------------------
// Internal parameters, fixed.
// These are set by the engine, and not changed
//  according to user inputs. Partly load from
//  WAD, partly set at startup time.

//e6y
extern bool realframe;

extern int upmove;

//-----------------------------------------
// Internal parameters, used for engine.
//

// File handling stuff.
extern FILE *debugfile;

// debug flag to cancel adaptiveness
extern bool singletics;

extern int HelperThing;          // type of thing to use for helper

// killough 5/2/98: moved from m_misc.c:

extern bool forced_loadgame;
extern bool command_loadgame;
extern int  totalleveltimes;

/* CG: This is set to true when graphics have been initialized */
extern bool graphics_initialized;

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

extern bool autorun;           // always running?                   // phares

extern time_t level_start_time; // CG



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
void G_InitNew(gameskill_e skill, int episode, int map);
void G_DeferredInitNew(gameskill_e skill, int episode, int map);

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
void G_MakeSpecialEvent(buttoncode_t bc, ...); /* cph - new event stuff */
void G_SetGameState(gamestate_e new_gamestate);
void G_SetPrevGameState(gamestate_e new_prevgamestate);
void G_SetOldGameState(gamestate_e new_oldgamestate);
void G_SetWipeGameState(gamestate_e new_wipegamestate);
void G_ResetGameState(void);

gamestate_e G_GetGameState(void);

gameaction_e G_GetGameAction(void);
void         G_SetGameAction(gameaction_e new_gameaction);

#endif

/* vi: set et ts=2 sw=2: */
