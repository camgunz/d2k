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

#include "g_game.h"
#include "g_state.h"
#include "pl_cmd.h"
#include "pl_weap.h"
#include "pl_main.h"
#include "n_main.h"
#include "n_msg.h"
#include "cl_cmd.h"
#include "cl_net.h"

static bool command_is_synchronized(gpointer data, gpointer user_data) {
  idxticcmd_t *icmd = (idxticcmd_t *)data;
  int state_tic = GPOINTER_TO_INT(user_data);

  if (icmd->server_tic == 0) {
    return false;
  }

  if (icmd->server_tic >= state_tic) {
    return false;
  }

  return true;
}

static void count_command(gpointer data, gpointer user_data) {
  int state_tic = G_GetStateFromTic();
  idxticcmd_t *icmd = (idxticcmd_t *)data;
  unsigned int *command_count = (unsigned int *)user_data;

  if (icmd->index < state_tic) {
    (*command_count)++;
  }
}

void CL_TrimSynchronizedCommands(void) {
  int state_tic = G_GetStateFromTic();

  N_MsgCmdLocalDebug("(%5d) Trimming synchronized commands\n", gametic);

  PLAYERS_FOR_EACH(iter) {
    PL_TrimCommands(
      iter.player,
      command_is_synchronized,
      GINT_TO_POINTER(state_tic)
    );
  }
}

size_t CL_GetUnsynchronizedCommandCount(player_t *player) {
  netpeer_t *server;
  size_t command_count = 0;
  
  if (!CLIENT) {
    return 0;
  }
  
  server = CL_GetServerPeer();

  if (!server) {
    return 0;
  }

  PL_ForEachCommand(player, count_command, &command_count);

  return command_count;
}


/* vi: set et ts=2 sw=2: */

