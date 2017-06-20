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


#ifndef G_OPT_H__
#define G_OPT_H__

typedef struct game_options_s {
  bool monsters;
  bool monsters_respawn;
  bool monsters_fast;
  bool dev_mode;
  int  deathmatch;
  bool allow_pushers;
  bool variable_friction;
  bool monsters_remember;
  bool weapon_recoil;
  bool player_bobbing;
  bool leave_weapons;
  int  dogs;
  bool dog_jumping;
  int  distfriend;
  bool monster_backing;
  bool monster_avoid_hazards;
  bool monster_friction;
  bool help_friends;
  bool monster_infighting;
  bool monsters_climb;
  int  viewangleoffset;
} game_options_t;

#endif

/* vi: set et ts=2 sw=2: */