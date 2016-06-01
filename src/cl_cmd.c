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

#include "doomdef.h"
#include "doomstat.h"
#include "d_event.h"
#include "n_net.h"
#include "n_state.h"
#include "n_peer.h"
#include "cl_cmd.h"
#include "p_user.h"
#include "p_setup.h"
#include "g_game.h"

#if 0
static bool command_is_synchronized(gpointer data, gpointer user_data) {
  netticcmd_t *ncmd = (netticcmd_t *)data;

  return ncmd->server_tic != 0;
}
#endif

static void count_command(gpointer data, gpointer user_data) {
  netpeer_t *server = CL_GetServerPeer();
  netticcmd_t *ncmd = (netticcmd_t *)data;
  unsigned int *command_count = (unsigned int *)user_data;

  if (server == NULL)
    return;

  if (ncmd->server_tic == 0) {
    (*command_count)++;
  }
}

void CL_TrimSynchronizedCommands(int playernum) {
  printf(
    "(%5d) Trimming synchronized commands (actually, just skipping this)\n",
    gametic
  );
  // P_TrimCommands(playernum, command_is_synchronized, NULL);
}

unsigned int CL_GetUnsynchronizedCommandCount(int playernum) {
  netpeer_t *server;
  unsigned int command_count = 0;
  
  if (!CLIENT)
    return 0;
  
  if (!playeringame[playernum])
    return 0;

  server = CL_GetServerPeer();

  if (server == NULL)
    return 0;

  P_ForEachCommand(playernum, count_command, &command_count);

  return command_count;
}


/* vi: set et ts=2 sw=2: */

