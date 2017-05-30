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


#ifndef DOOMSTAT_H__
#define DOOMSTAT_H__

#define VANILLA_MAXPLAYERS 4
#define PLAYER_CAP 2000
#define TICRATE 35

#define MULTINET     (netgame && (!solonet) && (!netdemo))
#define CLIENT       (MULTINET && (!netserver))
#define SERVER       (MULTINET && netserver)
#define SINGLEPLAYER (!demorecording && !demoplayback && !democontinue && \
                      !netgame)

// Game mode handling - identify IWAD version
//  to handle IWAD dependend animations etc.
typedef enum {
  shareware,    // DOOM 1 shareware, E1, M9
  registered,   // DOOM 1 registered, E3, M27
  commercial,   // DOOM 2 retail, E1 M34  (DOOM 2 german edition not handled)
  retail,       // DOOM 1 retail, E4, M36
  indetermined  // Well, no IWAD found.
} game_mode_e;

// Mission packs - might be useful for TC stuff?
typedef enum {
  doom,         // DOOM 1
  doom2,        // DOOM 2
  pack_tnt,     // TNT mission pack
  pack_plut,    // Plutonia pack
  pack_nerve,   // No Rest For The Living
  hacx,         // HACX - Twitch 'n Kill
  chex,         // Chex Quest
  none
} game_mission_e;

extern bool           usergame;
extern bool           solonet;
extern bool           netgame;
extern bool           netserver;
extern bool           netdemo;
extern bool           demorecording;
extern bool           demoplayback;
extern bool           democontinue;
extern game_mode_e    gamemode;
extern game_mission_e gamemission;

#endif

/* vi: set et ts=2 sw=2: */
