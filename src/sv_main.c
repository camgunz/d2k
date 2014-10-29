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

#include <enet/enet.h>

#include "doomstat.h"
#include "n_net.h"
#include "n_main.h"
#include "n_state.h"
#include "n_peer.h"
#include "p_cmd.h"

static void sv_remove_old_commands(void) {
  NETPEER_FOR_EACH(iter) {
    for (int i = 0; i < MAXPLAYERS; i++) {
      if (i != iter.np->playernum) {
        P_RemoveOldCommands(
          iter.np->sync.commands[i].received,
          iter.np->sync.commands[i].sync_queue
        );
      }
    }
  }
}

static void sv_remove_old_states(void) {
  int oldest_gametic = gametic;

  NETPEER_FOR_EACH(iter) {
    netpeer_t *np = iter.np;

    if (np->sync.tic > 0)
      oldest_gametic = MIN(oldest_gametic, np->sync.tic);
  }

  N_RemoveOldStates(oldest_gametic);
}

void SV_CleanupOldCommandsAndStates(void) {
  if (!SERVER)
    return;

  sv_remove_old_commands();
  if (DELTASERVER)
    sv_remove_old_states();
}

