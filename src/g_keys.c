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

#include "z_zone.h"

#include "g_keys.h"

//
// controls (have defaults)
//

int key_right;
int key_left;
int key_up;
int key_down;
int key_mlook;
int key_menu_toggle;
int key_menu_right;                                      // phares 3/7/98
int key_menu_left;                                       //     |
int key_menu_up;                                         //     V
int key_menu_down;
int key_menu_backspace;                                  //     ^
int key_menu_escape;                                     //     |
int key_menu_enter;                                      // phares 3/7/98
int key_strafeleft;
int key_straferight;
int key_flyup;
int key_flydown;
int key_fire;
int key_use;
int key_strafe;
int key_speed;
int key_escape;                                         // phares 4/13/98
int key_savegame;                                               // phares
int key_loadgame;                                               //    |
int key_autorun;                                                //    V
int key_reverse;
int key_zoomin;
int key_zoomout;
int key_chat;
int key_backspace;
int key_enter;
int key_map_right;
int key_map_left;
int key_map_up;
int key_map_down;
int key_map_zoomin;
int key_map_zoomout;
int key_map;
int key_map_gobig;
int key_map_follow;
int key_map_mark;
int key_map_clear;
int key_map_grid;
int key_map_overlay; // cph - map overlay
int key_map_rotate;  // cph - map rotation
int key_map_textured;  // e6y - textured automap
int key_help;                                           // phares 4/13/98
int key_soundvolume;
int key_hud;
int key_quicksave;
int key_endgame;
int key_messages;
int key_quickload;
int key_quit;
int key_gamma;
int key_spy;
int key_pause;
int key_setup;
int key_forward;
int key_leftturn;
int key_rightturn;
int key_backward;
int destination_key_green;
int destination_key_indigo;
int destination_key_brown;
int destination_key_red;
int key_weapontoggle;
int key_weapon1;
int key_weapon2;
int key_weapon3;
int key_weapon4;
int key_weapon5;
int key_weapon6;
int key_weapon7;                                                //    ^
int key_weapon8;                                                //    |
int key_weapon9;                                                // phares
int key_nextweapon;
int key_prevweapon;
int key_screenshot;             // killough 2/22/98: screenshot key
int key_speed_up;
int key_speed_down;
int key_speed_default;
int key_level_restart;
int key_nextlevel;
int key_demo_jointogame;
int key_demo_endlevel;
int key_demo_skip;
int key_walkcamera;
int key_showalive;
int mousebfire;
int mousebstrafe;
int mousebforward;
int mousebbackward;
int mousebuse;
int joybfire;
int joybstrafe;
int joybstrafeleft;
int joybstraferight;
int joybuse;
int joybspeed;

bool gamekeydown[NUMKEYS];

/* vi: set et ts=2 sw=2: */

