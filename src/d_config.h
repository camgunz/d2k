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


#ifndef D_CONFIG_H__
#define D_CONFIG_H__

/*
 * Generally game options should be sync'd across peers and engine options
 * shouldn't.  But `max_player_corpse` is one of a few cases of an engine
 * option that should be sync'd.  So we can't overload like that.  Instead, I
 * think we need local/sync'd sections of the options.
 */

typedef struct local_config_s {
  int process_priority;
  bool try_to_reduce_cpu_cache_misses;
  int default_compatibility_level;
  int realtic_clock_rate;
  bool menu_background;
  bool flashing_hom;
  bool demo_insurance;
  bool level_precache;
  bool demo_smoothturns;
  int  default_translucency; // config file says           // phares
  bool general_translucency; // true if translucency is ok // phares
  // Defaults for menu, methinks.
  gameskill_e startskill;
  unsigned int startepisode;
  unsigned int startmap;
  bool    drawers;
  bool    blit;
  // if true, load all graphics at level load
  bool precache;
  bool flashing_hom; // killough 10/98
  bool doom_weapon_toggles;   // killough 10/98
  bool shorttics;
  int speed_step;
} local_config_t;

typedef struct sync_config_s {
  size_t max_player_corpse;
} sync_config_t;

#endif

/* vi: set et ts=2 sw=2: */
