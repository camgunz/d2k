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

#include "doomdef.h"
#include "doomstat.h"
#include "d_main.h"
#include "p_setup.h"
#include "p_mobj.h"
#include "m_misc.h"
#include "sounds.h"
#include "s_sound.h"
#include "i_sound.h"
#include "r_defs.h"
#include "sc_man.h"
#include "w_wad.h"
#include "p_user.h"
#include "g_game.h"
#include "s_advsound.h"

#define TIDNUM(x) ((int)(x->iden_nums & 0xFFFF))		// thing identifier

musinfo_t musinfo;

//
// S_ParseMusInfo
// Parses MUSINFO lump.
//
void S_ParseMusInfo(const char *mapid) {
  memset(&musinfo, 0, sizeof(musinfo));
  musinfo.current_item = -1;

  S_music[NUMMUSIC].lumpnum = -1;

  if (W_CheckNumForName("MUSINFO") != -1) {
    int num, lumpnum;
    int inMap = false;

    SC_OpenLump("MUSINFO");

    while (SC_GetString()) {
      if (inMap || SC_Compare(mapid)) {
        if (!inMap) {
          SC_GetString();
          inMap = true;
        }

        if (sc_String[0] == 'E' || sc_String[0] == 'e' ||
            sc_String[0] == 'M' || sc_String[0] == 'm') {
          break;
        }

        // Check number in range
        if (M_StrToInt(sc_String, &num) && num > 0 && num < MAX_MUS_ENTRIES) {
          if (SC_GetString()) {
            lumpnum = W_CheckNumForName(sc_String);

            if (lumpnum >= 0) {
              musinfo.items[num] = lumpnum;
            }
            else {
              D_Msg(MSG_ERROR,
                "S_ParseMusInfo: Unknown MUS lump %s", sc_String
              );
            }
          }
        }
        else {
          D_Msg(MSG_ERROR,
            "S_ParseMusInfo: Number not in range 1 to %d", MAX_MUS_ENTRIES
          );
        }
      }
    }

    SC_Close();
  }
}

void MusInfoThinker(mobj_t *thing) {
  if (musinfo.mapthing != thing &&
      thing->subsector->sector == players[displayplayer].mo->subsector->sector)
  {
    musinfo.lastmapthing = musinfo.mapthing;
    musinfo.mapthing = thing;
    musinfo.tics = 30;
  }
}

void T_MAPMusic(void)
{
  if (musinfo.tics < 0 || !musinfo.mapthing)
    return;

  if (musinfo.tics > 0) {
    musinfo.tics--;
  }
  else {
    if (!musinfo.tics && musinfo.lastmapthing != musinfo.mapthing) {
      int arraypt = TIDNUM(musinfo.mapthing);

      if (arraypt >= 0 && arraypt < MAX_MUS_ENTRIES) {
        int lumpnum = musinfo.items[arraypt];

        if (lumpnum >= 0 && lumpnum < numlumps)
          S_ChangeMusInfoMusic(lumpnum, true);
      }

      musinfo.tics = -1;
    }
  }
}

/* vi: set et ts=2 sw=2: */

