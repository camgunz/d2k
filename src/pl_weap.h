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


#ifndef PL_WEAP_H__
#define PL_WEAP_H__

typedef enum {
  WSOP_NONE   = 0,
  WSOP_WEAPON = 1,
  WSOP_AMMO   = 2,
  WSOP_MAX    = 4,
} wsop_e;

extern int weapon_preferences[2][NUMWEAPONS + 1]; /* killough 5/2/98 */

int  P_WeaponPreferred(int w1, int w2);
int  P_SwitchWeapon(player_t *player);
bool P_CheckAmmo(player_t *player);
void P_DropWeapon(player_t *player);

#endif

/* vi: set et ts=2 sw=2: */
