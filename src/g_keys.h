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

#ifndef G_KEYS_H__
#define G_KEYS_H__

#define NUMKEYS 512

extern int key_right;
extern int key_left;
extern int key_up;
extern int key_down;
extern int key_mlook;
extern int key_menu_toggle;
extern int key_menu_right;                                  // phares 3/7/98
extern int key_menu_left;                                   //     |
extern int key_menu_up;                                     //     V
extern int key_menu_down;
extern int key_menu_backspace;                              //     ^
extern int key_menu_escape;                                 //     |
extern int key_menu_enter;                                  // phares 3/7/98
extern int key_strafeleft;
extern int key_straferight;
extern int key_flyup;
extern int key_flydown;
extern int key_fire;
extern int key_use;
extern int key_strafe;
extern int key_speed;
extern int key_escape;                                             // phares
extern int key_savegame;                                           //    |
extern int key_loadgame;                                           //    V
extern int key_autorun;
extern int key_reverse;
extern int key_zoomin;
extern int key_zoomout;
extern int key_chat;
extern int key_backspace;
extern int key_enter;
extern int key_help;
extern int key_soundvolume;
extern int key_hud;
extern int key_quicksave;
extern int key_endgame;
extern int key_messages;
extern int key_quickload;
extern int key_quit;
extern int key_gamma;
extern int key_spy;
extern int key_pause;
extern int key_setup;
extern int key_forward;
extern int key_leftturn;
extern int key_rightturn;
extern int key_backward;
extern int key_weapontoggle;
extern int key_weapon1;
extern int key_weapon2;
extern int key_weapon3;
extern int key_weapon4;
extern int key_weapon5;
extern int key_weapon6;
extern int key_weapon7;
extern int key_weapon8;
extern int key_weapon9;
extern int key_nextweapon;
extern int key_prevweapon;
extern int destination_key_green;
extern int destination_key_indigo;
extern int destination_key_brown;
extern int destination_key_red;
extern int key_map_right;
extern int key_map_left;
extern int key_map_up;
extern int key_map_down;
extern int key_map_zoomin;
extern int key_map_zoomout;
extern int key_map;
extern int key_map_gobig;
extern int key_map_follow;
extern int key_map_mark;                                           //    ^
extern int key_map_clear;                                          //    |
extern int key_map_grid;                                           // phares
extern int key_map_rotate;    // cph - map rotation
extern int key_map_overlay;   // cph - map overlay
extern int key_map_textured;  //e6y: textured automap
extern int key_screenshot;    // killough 2/22/98 -- add key for screenshot
extern int key_speed_up;
extern int key_speed_down;
extern int key_speed_default;
extern int key_level_restart;
extern int key_nextlevel;
extern int key_demo_jointogame;
extern int key_demo_endlevel;
extern int key_demo_skip;
extern int key_walkcamera;
extern int key_showalive;
extern int mousebfire;
extern int mousebstrafe;
extern int mousebforward;
extern int mousebbackward;
extern int mousebuse;
extern int joybfire;
extern int joybstrafe;
extern int joybstrafeleft;
extern int joybstraferight;
extern int joybuse;
extern int joybspeed;

extern bool gamekeydown[NUMKEYS];

#endif

/* vi: set et ts=2 sw=2: */

