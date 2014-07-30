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
/* vi: set et ts=2 sw=2:                                                     */
/*                                                                           */
/*****************************************************************************/

#ifndef S_ADVSOUND_H__
#define S_ADVSOUND_H__

#include "p_mobj.h"

#ifdef __GNUG__
#pragma interface
#endif

//
//MUSINFO lump
//

#define MAX_MUS_ENTRIES 64

typedef struct musinfo_s
{
  mobj_t *mapthing;
  mobj_t *lastmapthing;
  int tics;
  int current_item;
  int items[MAX_MUS_ENTRIES];
} musinfo_t;

extern musinfo_t musinfo;

void S_ParseMusInfo(const char *mapid);
void MusInfoThinker(mobj_t *thing);
void T_MAPMusic(void);

#endif

