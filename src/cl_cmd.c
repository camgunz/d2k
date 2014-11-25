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
#include "n_state.h"
#include "n_peer.h"
#include "cl_cmd.h"
#include "p_cmd.h"

void CL_ClearSynchronizedCommands(int playernum) {
  if (!playeringame[playernum])
    return;

  while (P_HasCommands(playernum)) {
    bool found_old_command = false;
    unsigned int command_count = P_GetCommandCount(playernum);

    for (unsigned int i = 0; i < command_count; i++) {
      netticcmd_t *ncmd = P_GetCommand(playernum, i);

      if (ncmd->server_tic == 0)
        continue;

      D_Log(LOG_SYNC,
        "CL_ClearSynchronizedCommands: Removing old command %d/%d\n",
        ncmd->index,
        ncmd->tic
      );
      P_RecycleCommand(P_PopCommand(playernum, i));
      found_old_command = true;
      break;
    }

    if (!found_old_command)
      break;
  }
}

unsigned int CL_GetUnsynchronizedCommandCount(int playernum) {
  netpeer_t *server;
  unsigned int command_count = 0;
  unsigned int queue_length;
  
  if (!CLIENT)
    return 0;
  
  if (!playeringame[playernum])
    return 0;

  server = CL_GetServerPeer();

  if (server == NULL)
    return 0;

  queue_length = P_GetCommandCount(playernum);

  for (unsigned int i = 0; i < queue_length; i++) {
    netticcmd_t *ncmd = P_GetCommand(playernum, i);

    if (ncmd->index > server->sync.command_index)
      command_count++;
  }

  return command_count;
}


/* vi: set et ts=2 sw=2: */

