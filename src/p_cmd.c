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
#include "d_event.h"
#include "d_log.h"
#include "d_player.h"
#include "d_ticcmd.h"
#include "e6y.h"
#include "g_game.h"
#include "i_video.h"
#include "n_net.h"
#include "n_main.h"
#include "n_state.h"
#include "n_proto.h"
#include "p_cmd.h"
#include "p_map.h"
#include "p_mobj.h"
#include "p_pspr.h"
#include "p_user.h"

static GQueue *blank_command_queue;
static GQueue *local_commands;
static int local_command_index = 0;

static void print_command(gpointer data, gpointer user_data) {
  netticcmd_t *ncmd = (netticcmd_t *)data;

  D_Log(LOG_SYNC, "%d/%d ", ncmd->index, ncmd->tic);
}

static void recycle_command(netticcmd_t *ncmd) {
  memset(ncmd, 0, sizeof(netticcmd_t));

  g_queue_push_tail(blank_command_queue, ncmd);
}

static netticcmd_t* get_new_blank_command(void) {
  netticcmd_t *ncmd = g_queue_pop_head(blank_command_queue);

  if (ncmd == NULL)
    ncmd = calloc(1, sizeof(netticcmd_t));

  return ncmd;
}

static netticcmd_t* get_new_indexed_local_command(void) {
  netticcmd_t *ncmd = get_new_blank_command();

  ncmd->index = local_command_index;
  local_command_index++;
  g_queue_push_tail(local_commands, ncmd);

  return ncmd;
}

static netticcmd_t* get_new_blank_player_command(player_t *player) {
  netticcmd_t *ncmd = get_new_blank_command();

  g_queue_push_tail(player->commands, ncmd);

  return ncmd;
}

static void copy_local_command_to_player_command_buffer(gpointer data,
                                                        gpointer user_data) {
  player_t *player = (player_t *)user_data;
  netticcmd_t *local_ncmd = (netticcmd_t *)data;
  netticcmd_t *player_ncmd;
  
  if (local_ncmd->tic > gametic)
    return;

  player_ncmd = get_new_blank_command();
  memcpy(player_ncmd, local_ncmd, sizeof(netticcmd_t));
  g_queue_push_tail(player->commands, player_ncmd);
}

static void run_player_command(player_t *player) {
  ticcmd_t *cmd = &player->cmd;
  weapontype_t newweapon;

  // chain saw run forward
  if (player->mo->flags & MF_JUSTATTACKED) {
    cmd->angleturn = 0;
    cmd->forwardmove = 0xc800 / 512;
    cmd->sidemove = 0;
    player->mo->flags &= ~MF_JUSTATTACKED;
  }

  if (player->playerstate == PST_DEAD)
    return;

  if (player->jumpTics)
    player->jumpTics--;

  // Move around.
  // Reactiontime is used to prevent movement
  //  for a bit after a teleport.
  if (player->mo->reactiontime)
    player->mo->reactiontime--;
  else
    P_MovePlayer(player);

  P_SetPitch(player);

  P_CalcHeight(player); // Determines view height and bobbing

  // Check for weapon change.
  if (cmd->buttons & BT_CHANGE) {
    // The actual changing of the weapon is done
    //  when the weapon psprite can do it
    //  (read: not in the middle of an attack).

    newweapon = (cmd->buttons & BT_WEAPONMASK) >> BT_WEAPONSHIFT;

    // killough 3/22/98: For demo compatibility we must perform the fist
    // and SSG weapons switches here, rather than in G_BuildTiccmd(). For
    // other games which rely on user preferences, we must use the latter.

    // compatibility mode -- required for old demos -- killough
    if (demo_compatibility) {
      //e6y
      if (!prboom_comp[PC_ALLOW_SSG_DIRECT].state)
        newweapon = (cmd->buttons & BT_WEAPONMASK_OLD) >> BT_WEAPONSHIFT;

      if (newweapon == wp_fist && player->weaponowned[wp_chainsaw] && (
            player->readyweapon != wp_chainsaw ||
            !player->powers[pw_strength])) {
        newweapon = wp_chainsaw;
      }

      if (gamemode == commercial && newweapon == wp_shotgun &&
                                    player->weaponowned[wp_supershotgun] &&
                                    player->readyweapon != wp_supershotgun) {
        newweapon = wp_supershotgun;
      }
    }

    // killough 2/8/98, 3/22/98 -- end of weapon selection changes

    // Do not go to plasma or BFG in shareware, even if cheated.
    if (player->weaponowned[newweapon] &&
        newweapon != player->readyweapon && (
          (newweapon != wp_plasma && newweapon != wp_bfg) ||
          (gamemode != shareware))) {
      player->pendingweapon = newweapon;
    }
  }

  // check for use

  if (cmd->buttons & BT_USE) {
    if (!player->usedown) {
      P_UseLines(player);
      player->usedown = true;
    }
  }
  else {
    player->usedown = false;
  }

  // cycle psprites

  P_MovePsprites(player);
}

void P_InitCommands(void) {
  if (blank_command_queue == NULL)
    blank_command_queue = g_queue_new();

  for (int i = 0; i < MAXPLAYERS; i++)
    P_InitPlayerCommands(&players[i]);

  P_InitLocalCommands();
}

void P_InitLocalCommands(void) {
  if (local_commands == NULL)
    local_commands = g_queue_new();
  else
    P_ClearLocalCommands();
}

void P_InitPlayerCommands(player_t *player) {
  if (player->commands == NULL)
    player->commands = g_queue_new();
  else
    P_ClearPlayerCommands(player);
}

unsigned int P_GetLocalCommandCount(void) {
  return g_queue_get_length(local_commands);
}

unsigned int P_GetPlayerCommandCount(player_t *player) {
  return g_queue_get_length(player->commands);
}

void P_UpdateConsoleplayerCommands(void) {
  player_t *player = &players[consoleplayer];

  P_ClearPlayerCommands(player);

  g_queue_foreach(
    local_commands, copy_local_command_to_player_command_buffer, player
  );
}

void P_ClearPlayerCommands(player_t *player) {
  while (!g_queue_is_empty(player->commands))
    recycle_command(g_queue_pop_head(player->commands));
}

void P_ClearLocalCommands(void) {
  while (!g_queue_is_empty(local_commands))
    recycle_command(g_queue_pop_head(local_commands));
}

void P_RemoveSyncedCommands(void) {
  int command_index;
  int sync_tic;

  if (!CLIENT)
    return;

  if (!CL_GetServerSync(&command_index, &sync_tic))
    return;

  while (!g_queue_is_empty(local_commands)) {
    netticcmd_t *ncmd = g_queue_peek_head(local_commands);

    if (ncmd->index <= command_index && ncmd->tic <= sync_tic)
      recycle_command(g_queue_pop_head(local_commands));
    else
      break;
  }
}

void P_PrintPlayerCommands(GQueue *commands) {
  D_Log(LOG_SYNC, "[ ");
  g_queue_foreach(commands, print_command, NULL);
  D_Log(LOG_SYNC, "]\n");
}

void P_BuildCommand(void) {
  player_t *player = &players[consoleplayer];
  netticcmd_t *local_ncmd;
  netticcmd_t *player_ncmd;

  if (DELTASERVER)
    return;

  I_StartTic();
  G_BuildTiccmd();

  local_ncmd = g_queue_peek_tail(local_commands);

  if (DELTACLIENT) {
    D_Log(LOG_SYNC, "Built command { %d/%d %d %d %d %d %u %u }\n",
      local_ncmd->index,
      local_ncmd->tic,
      local_ncmd->cmd.forwardmove,
      local_ncmd->cmd.sidemove,
      local_ncmd->cmd.angleturn,
      local_ncmd->cmd.consistancy,
      local_ncmd->cmd.buttons,
      local_ncmd->cmd.chatchar
    );
  }

  player_ncmd = get_new_blank_player_command(player);
  memcpy(player_ncmd, local_ncmd, sizeof(netticcmd_t));

  if (CLIENT)
    CL_MarkServerOutdated();

  P_PrintPlayerCommands(local_commands);
}

void P_RunAllPlayerCommands(player_t *player) {
  while (!g_queue_is_empty(player->commands)) {
    netticcmd_t *ncmd = g_queue_pop_head(player->commands);
    bool command_is_blank = (ncmd->index == 0 && ncmd->tic == 0);

    if (command_is_blank) {
      D_Log(LOG_SYNC, "(%d) Got blank command (%d, %d)\n",
        gametic, ncmd->index, ncmd->tic
      );
    }

    memcpy(&player->cmd, &ncmd->cmd, sizeof(ticcmd_t));
    D_Log(
      LOG_SYNC,
      "[%d: %td] P_RunAllPlayerCommands: After command "
      "{ %d/%d %d %d %d %d %u %u }\n",
      gametic, player - players,
      ncmd->index,
      ncmd->tic,
      ncmd->cmd.forwardmove,
      ncmd->cmd.sidemove,
      ncmd->cmd.angleturn,
      ncmd->cmd.consistancy,
      ncmd->cmd.buttons,
      ncmd->cmd.chatchar
    );
    run_player_command(player);
    N_LogPlayerPosition(player);

    if (player->mo) {
      P_MobjThinker(player->mo);
      D_Log(LOG_SYNC, "After P_MobjThinker:\n");
      N_LogPlayerPosition(player);
    }

    recycle_command(ncmd);
  }
}

void P_RunBufferedCommands(player_t *player) {
  int saved_leveltime;
  unsigned int command_count = P_GetPlayerCommandCount(player);

  if (command_count == 0) {
    D_Log(LOG_SYNC, "(%d) No commands for %td\n", gametic, player - players);
    return;
  }

  D_Log(LOG_SYNC, "(%d) Running %u commands for %td\n",
    gametic, command_count, player - players
  );

  while (!g_queue_is_empty(player->commands)) {
    netticcmd_t *ncmd = g_queue_peek_head(player->commands);

    if (ncmd->tic > gametic)
      break;

    ncmd = g_queue_pop_head(player->commands);

    memcpy(&player->cmd, &ncmd->cmd, sizeof(ticcmd_t));
    D_Log(
      LOG_SYNC,
      "[%d: %td] P_RunBufferedCommands: After command "
      "{ %d/%d %d %d %d %d %u %u }\n",
      gametic, player - players,
      ncmd->index,
      ncmd->tic,
      ncmd->cmd.forwardmove,
      ncmd->cmd.sidemove,
      ncmd->cmd.angleturn,
      ncmd->cmd.consistancy,
      ncmd->cmd.buttons,
      ncmd->cmd.chatchar
    );
    saved_leveltime = leveltime;
    leveltime = ncmd->tic;
    run_player_command(player);
    N_LogPlayerPosition(player);

    if (player != &players[consoleplayer] && player->mo) {
      P_MobjThinker(player->mo);
      D_Log(LOG_SYNC, "After P_MobjThinker:\n");
      N_LogPlayerPosition(player);
    }
    leveltime = saved_leveltime;

    recycle_command(ncmd);
  }
}

void P_RunNextCommand(player_t *player) {
  netticcmd_t *ncmd = g_queue_pop_head(player->commands);

  if (ncmd == NULL)
    return;

  memcpy(&player->cmd, &ncmd->cmd, sizeof(ticcmd_t));
  run_player_command(player);
  if (player->mo)
    P_MobjThinker(player->mo);

  recycle_command(ncmd);
}

void P_PredictPlayerPosition(player_t *player) {
  if ((!DELTACLIENT) || (player != &players[consoleplayer]) || (!player->mo))
    return;

  P_MobjThinker(player->mo);

  if (player != &players[0]) {
    D_Log(LOG_SYNC, "After P_MobjThinker:\n");
    N_LogPlayerPosition(player);
  }
}

void P_RunPlayerCommands(player_t *player) {
  if (!(DELTACLIENT || DELTASERVER)) {
    run_player_command(player);
    return;
  }
  
  /*
   * CG: 08/13/2014: TODO: Fix skipping caused by running a shit-ton of
   *                       commands in a single TIC.  This is gonna end up
   *                       being some kind of vector function combined with a
   *                       time limit which, if exceeded, disconnects the
   *                       player.  For now though, just keep thinking on the
   *                       player so their momentum keeps moving them.
   */

  if (P_GetPlayerCommandCount(player) == 0 &&
      DELTACLIENT &&
      player != &players[consoleplayer] &&
      player->mo) {
    P_PredictPlayerPosition(player);
    return;
  }

  if (DELTASERVER) {
    D_Log(LOG_SYNC, "(%d) Player %td has %d commands\n",
      gametic, player - players, P_GetPlayerCommandCount(player)
    );

    P_RunBufferedCommands(player);
    return;
  }

  if (player == &players[consoleplayer])
    P_RunAllPlayerCommands(player);
  else
    P_RunNextCommand(player);
}

bool P_LoadCommandForTic(player_t *player, int tic) {
  netticcmd_t *ncmd;

  while (!g_queue_is_empty(player->commands)) {
    ncmd = g_queue_peek_head(player->commands);

    if (ncmd->tic < gametic)
      recycle_command(g_queue_pop_head(player->commands));
    else
      break;
  }

  if (g_queue_is_empty(player->commands))
    return false;

  while (!g_queue_is_empty(player->commands)) {
    ncmd = g_queue_peek_head(player->commands);

    if (ncmd->tic == gametic) {
      memcpy(&player->cmd, &ncmd->cmd, sizeof(ticcmd_t));
      recycle_command(g_queue_pop_head(player->commands));
      return true;
    }
  }

  return false;
}

bool P_LoadLocalCommandForTic(int tic) {
  unsigned int command_count = g_queue_get_length(local_commands);
  
  for (unsigned int i = 0; i < command_count; i++) {
    netticcmd_t *ncmd = g_queue_peek_nth(local_commands, i);

    if (!ncmd)
      return false;

    if (ncmd->tic == tic) {
      memcpy(&players[consoleplayer].cmd, &ncmd->cmd, sizeof(ticcmd_t));
      return true;
    }
  }

  return false;
}

void P_RemoveOldCommands(player_t *player, int tic) {
  while (!g_queue_is_empty(player->commands)) {
    netticcmd_t *ncmd = g_queue_peek_head(player->commands);

    if (ncmd->tic < tic)
      recycle_command(g_queue_pop_head(player->commands));
    else
      break;
  }
}

GQueue* P_GetLocalCommands(void) {
  return local_commands;
}

netticcmd_t* P_GetNewIndexedLocalCommand(void) {
  return get_new_indexed_local_command();
}

netticcmd_t* P_GetNewBlankPlayerCommand(player_t *player) {
  return get_new_blank_player_command(player);
}

/* vi: set et ts=2 sw=2: */

