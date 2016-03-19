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
#include "g_save.h"
#include "n_net.h"
#include "n_main.h"
#include "n_state.h"
#include "n_peer.h"
#include "n_proto.h"
#include "p_cmd.h"
#include "p_user.h"

#define MAX_COMMAND_COUNT (TICRATE / 4)

static game_state_t *unlag_game_state = NULL;
static int           unlag_tic;

int sv_limit_player_commands = 0;

static void sv_remove_old_commands(void) {
  int oldest_sync_tic = INT_MAX;

  NETPEER_FOR_EACH(iter) {
    oldest_sync_tic = MIN(oldest_sync_tic, iter.np->sync.tic);
  }

  if (oldest_sync_tic == INT_MAX)
    return;

  for (int i = 0; i < MAXPLAYERS; i++)
    P_TrimCommandsByTic(i, oldest_sync_tic);
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
  sv_remove_old_states();
}

/*
 * CG: This code originates from Odamex, released under the GPLv2 or any later
 *     version.
 */
unsigned int SV_GetPlayerCommandLimit(int playernum) {
  static int catchup_playernum = 0;

  player_t *player = &players[playernum];
  unsigned int command_count = P_GetCommandCount(playernum);

  if (player->mo == NULL)
    return 0;
  
  if (command_count == 0)
    return 0;

  /* Process all queued ticcmds */
  if (!sv_limit_player_commands)
    return MAX_COMMAND_COUNT;

  if (player->playerstate == PST_DEAD)
    return MAX_COMMAND_COUNT;

  /* Check for non-moving player */
  if (player->mo->momx == 0 && player->mo->momy == 0 && player->mo->momz == 0)
    return 2;

  if (command_count <= 2)
    return 1;

  /*
   * Process an extra ticcmd once every 2 seconds to reduce the queue size. Use
   * player id to stagger the timing to prevent everyone from running an extra
   * ticcmd at the same time.
   */
  if ((gametic % (2 * TICRATE)) == 0) {
    /* CG: Find a suitable player */
    int last_playernum = catchup_playernum;

    while (1) {
      catchup_playernum++;

      if (catchup_playernum == last_playernum)
        break;

      if (catchup_playernum == MAXPLAYERS)
        catchup_playernum = 0;

      if (catchup_playernum == consoleplayer)
        continue;

      if (!playeringame[catchup_playernum])
        continue;

      return 2;
    }
  }

  /*
   * The player experienced a large latency spike, so try to catch up by
   * processing more than one ticcmd at the expense of appearing perfectly
   * smooth
   */
  if (command_count > MAX_COMMAND_COUNT)
    return 2;

  return 1; /* always run at least 1 ticcmd if possible */
}

void SV_UnlagSetTIC(int tic) {
  unlag_tic = tic;
}

bool SV_UnlagStart(void) {
  if (!unlag_game_state) {
    unlag_game_state = malloc(sizeof(game_state_t));
    if (!unlag_game_state) {
      I_Error("Error allocating unlagged game state");
    }

    unlag_game_state->data = M_PBufNewWithCapacity(16384);
  }
  else {
    M_PBufClear(unlag_game_state->data);
  }

  for (int i = 0; i < MAXPLAYERS; i++) {
    if (!playeringame[i]) {
      continue;
    }

    players[i].saved_momx        = players[i].mo->momx;
    players[i].saved_momy        = players[i].mo->momy;
    players[i].saved_momz        = players[i].mo->momz;
    players[i].saved_damagecount = players[i].damagecount;
  }

  unlag_game_state->tic = gametic;
  G_WriteSaveData(unlag_game_state->data);

  return N_LoadState(unlag_tic, false);
}

void SV_UnlagEnd(void) {
  for (int i = 0; i < MAXPLAYERS; i++) {
    if (!playeringame[i]) {
      continue;
    }

    /* [CG] These now become deltas instead of base values */
    players[i].saved_momx        -= players[i].mo->momx;
    players[i].saved_momy        -= players[i].mo->momy;
    players[i].saved_momz        -= players[i].mo->momz;
    players[i].saved_damagecount -= players[i].damagecount;
  }

  M_PBufSeek(unlag_game_state->data, 0);
  G_ReadSaveData(unlag_game_state->data, true, false);

  for (int i = 0; i < MAXPLAYERS; i++) {
    if (!playeringame[i]) {
      continue;
    }

    players[i].mo->momx    += players[i].saved_momx;
    players[i].mo->momy    += players[i].saved_momy;
    players[i].mo->momz    += players[i].saved_momz;
    players[i].damagecount += players[i].saved_damagecount;
  }
}

/* vi: set et ts=2 sw=2: */

