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

#ifndef WI_STUFF_H__
#define WI_STUFF_H__

//
// INTERMISSION
// Structure passed e.g. to WI_Start(wb)
//
typedef struct {
  bool in;                    // whether the player is in game
  int  skills;                // Player stats, kills, collected items etc.
  int  sitems;
  int  ssecret;
  int  stime;
  int  frags[VANILLA_MAXPLAYERS];
  int  score;                 // current score on entry, modified on return
} wbplayerstruct_t;

typedef struct {
  int              epsd;      // episode # (0-2)
  bool             didsecret; // if true, splash the secret level
  int              last;      // previous and next levels, origin 0
  int              next;
  int              maxkills;
  int              maxitems;
  int              maxsecret;
  int              maxfrags;
  int              partime;   // the par time
  int              pnum;      // index of this player in game
  wbplayerstruct_t plyr[VANILLA_MAXPLAYERS];
  int              totaltimes; // CPhipps - total game time for completed
                               // levels so far
} wbstartstruct_t;

// States for the intermission

typedef enum {
  NoState = -1,
  StatCount,
  ShowNextLoc
} stateenum_t;

// Called by main loop, animate the intermission.
void WI_Ticker(void);

// Called by main loop,
// draws the intermission directly into the screen buffer.
void WI_Drawer(void);

// Setup for an intermission screen.
void WI_Start(int par_time);

// Release intermission screen memory
void WI_End(void);

#endif // ifndef WI_STUFF_H__

/* vi: set et ts=2 sw=2: */
