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

typedef struct config_s {
    int process_priority;
    bool try_to_reduce_cpu_cache_misses;
    int default_compatibility_level;
    int realtic_clock_rate;
    bool menu_background;
    int max_player_corpse;
    bool flashing_hom;
    int demo_insurance;
    bool level_precache;
    bool demo_smoothturns;
    demo

} config_t;

#endif

/* vi: set et ts=2 sw=2: */
