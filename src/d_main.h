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


#ifndef D_MAIN_H__
#define D_MAIN_H__

struct event_s;
typedef struct event_s event_t;

/* CPhipps - removed wadfiles[] stuff to w_wad.h */

extern char *basesavegame;      // killough 2/16/98: savegame path

//jff 1/24/98 make command line copies of play modes available
extern bool clnomonsters; // checkparm of -nomonsters
extern bool clrespawnparm;  // checkparm of -respawn
extern bool clfastparm; // checkparm of -fast
//jff end of external declaration of command line playmode

extern bool nosfxparm;
extern bool nomusicparm;
extern int ffmap;

// Called by IO functions when input is detected.
void D_PostEvent(event_t* ev);

// Demo stuff
extern bool advancedemo;
void D_AdvanceDemo(void);
void D_DoAdvanceDemo (void);

//
// BASE LEVEL
//

void        D_Display(void);
void        D_PageTicker(void);
void        D_StartTitle(void);
bool        D_Wiping(void);
bool        D_Responder(event_t *ev);
void        D_DoomMain(void);
void        AddIWAD(const char *iwad);
void        D_SetIWAD(const char *iwad);
const char* D_GetIWAD(void);
const char* D_GetIWADPath(void);
void        D_ClearIWAD(void);
void        D_ClearResourceFiles(void);
void        D_ClearDEHFiles(void);
void        CheckIWAD(const char *iwadname, GameMode_t *gmode, bool *hassec);
void        IdentifyVersion(void);

/* cph - MBF-like wad/deh/bex autoload code */
/* proff 2001/7/1 - added prboom.wad as last entry so it's always loaded and
   doesn't overlap with the cfg settings */
#define MAXLOADFILES 3
extern const char *wad_file_names[MAXLOADFILES], *deh_file_names[MAXLOADFILES];

#endif

/* vi: set et ts=2 sw=2: */

