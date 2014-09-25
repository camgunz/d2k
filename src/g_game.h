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

#include "doomdef.h"
#include "d_event.h"
#include "d_ticcmd.h"

struct netticcmd_s;

//
// SAVE
//

// CPhipps - Make savedesciption visible in wider scope
#define SAVEDESCLEN 32

// Description to save in savegame
extern char savedescription[SAVEDESCLEN];

// killough 5/15/98: forced loadgames
void G_ForcedLoadGame(void);
void G_DoSaveGame(dboolean menu);
void G_DoLoadGame(void);
// Called by M_Responder.
void G_SaveGame(int slot, char *description);
// killough 5/15/98
void G_LoadGame(int slot, dboolean is_command);
/* killough 3/22/98: sets savegame filename */
int G_SaveGameName(char *name, size_t size, int slot, dboolean demoplayback);

//
// GAME
//

#define DEMOMARKER    0x80

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

dboolean G_Responder(event_t *ev);
dboolean G_CheckDemoStatus(void);
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
void G_ReloadDefaults(void);     // killough 3/1/98: loads game defaults
void G_SetFastParms(int);        // killough 4/10/98: sets -fast parameters
void G_DoNewGame(void);
void G_DoReborn(int playernum);
void G_DoPlayDemo(void);
void G_DoCompleted(void);
void G_ReadDemoContinueTiccmd (ticcmd_t* cmd);//e6y
void G_ReadDemoTiccmd(ticcmd_t *cmd);
void G_WriteDemoTiccmd(ticcmd_t *cmd);
void G_DoWorldDone(void);
void G_Compatibility(void);
void G_ReadOptions(byte game_options[]);
void G_WriteOptions(byte game_options[]);
void G_PlayerReborn(int player);
void G_RestartLevel(void); // CPhipps - menu involked level restart
void G_DoVictory(void);
void G_BuildTiccmd(void);         // CPhipps - move decl to header
void G_ChangedPlayerColour(int pn, int cl); // CPhipps - On-the-fly player colour changing
void G_MakeSpecialEvent(buttoncode_t bc, ...); /* cph - new event stuff */

extern dboolean forced_loadgame;
extern dboolean command_loadgame;

extern int totalleveltimes;

//e6y
extern dboolean democontinue;
extern char* demo_continue_name;
void G_CheckDemoContinue(void);
void G_SetSpeed(void);

//e6y
#define RDH_SAFE 0x00000001
#define RDH_SKIP_HEADER 0x00000002
const byte* G_ReadDemoHeaderEx(const byte* demo_p, size_t size, unsigned int params);
const byte* G_ReadDemoHeader(const byte* demo_p, size_t size);
void G_CalculateDemoParams(const byte *demo_p);

// killough 5/2/98: moved from m_misc.c:

/* CG: This is set to true when graphics have been initialized */
extern dboolean graphics_initialized;

extern int  defaultskill;      //jff 3/24/98 default skill
extern dboolean haswolflevels;  //jff 4/18/98 wolf levels present

extern GQueue *corpse_queue;
extern int corpse_queue_size;       // killough 2/8/98: adustable corpse limit

// killough 5/2/98: moved from d_deh.c:
// Par times (new item with BOOM) - from g_game.c
extern int pars[5][10];  // hardcoded array size
extern int cpars[];      // hardcoded array size

/* cph - compatibility level strings */
extern const char * comp_lev_str[];

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

#define singleplayer (!demorecording && !demoplayback && !democontinue && !netgame)
#define comperr(i) (default_comperr[i] && !demorecording && !demoplayback && !democontinue && !netgame)

#endif

/* vi: set et ts=2 sw=2: */

