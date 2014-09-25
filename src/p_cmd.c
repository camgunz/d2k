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

static GArray *local_commands;
static int local_command_index = 0;

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

void P_InitLocalCommands(void) {
  if (local_commands != NULL && local_commands->len > 0) {
    g_array_remove_range(local_commands, 0, local_commands->len);
    return;
  }

  local_commands = g_array_sized_new(
    false, true, sizeof(netticcmd_t), BACKUPTICS
  );
}

void P_InitPlayerCommands(player_t *player) {
  if (player->commands != NULL && player->commands->len > 0) {
    g_array_remove_range(player->commands, 0, player->commands->len);
    return;
  }

  player->commands = g_array_sized_new(
    false, true, sizeof(netticcmd_t), BACKUPTICS
  );
}

unsigned int P_GetLocalCommandCount(void) {
  return local_commands->len;
}

unsigned int P_GetPlayerCommandCount(player_t *player) {
  return player->commands->len;
}

void P_UpdateConsoleplayerCommands(void) {
  GArray *player_commands = players[consoleplayer].commands;
  unsigned int local_command_count = P_GetLocalCommandCount();

  P_ClearPlayerCommands(&players[consoleplayer]);

  g_array_set_size(player_commands, local_command_count);
  for (unsigned int i = 0; i < local_command_count; i++) {
    netticcmd_t *local_ncmd = &g_array_index(local_commands, netticcmd_t, i);
    netticcmd_t *player_ncmd = &g_array_index(player_commands, netticcmd_t, i);

    if (local_ncmd->tic > gametic)
      break;

    memcpy(player_ncmd, local_ncmd, sizeof(netticcmd_t));
  }
}

void P_ClearPlayerCommands(player_t *player) {
  if (player->commands->len > 0)
    g_array_remove_range(player->commands, 0, player->commands->len);
}

void P_ClearLocalCommands(void) {
  if (local_commands->len > 0)
    g_array_remove_range(local_commands, 0, local_commands->len);
}

void P_RemoveSyncedCommands(void) {
  int command_index;
  int sync_tic;
  unsigned int i;

  if (!CLIENT)
    return;

  if (!CL_GetServerSync(&command_index, &sync_tic))
    return;

  for (i = 0; i < local_commands->len; i++) {
    netticcmd_t *ncmd = &g_array_index(local_commands, netticcmd_t, i);

    if (ncmd->index > command_index || ncmd->tic > sync_tic)
      break;
  }

  if (i > 0)
    g_array_remove_range(local_commands, 0, i);
}

void P_PrintPlayerCommands(GArray *commands) {
  D_Log(LOG_SYNC, "[ ");

  for (unsigned int i = 0; i < commands->len; i++) {
    netticcmd_t *ncmd = &g_array_index(commands, netticcmd_t, i);

    D_Log(LOG_SYNC, "%d/%d ", ncmd->index, ncmd->tic);
  }

  D_Log(LOG_SYNC, "]\n");
}

void P_BuildCommand(void) {
  netticcmd_t *ncmd = NULL;

  if (DELTASERVER)
    return;

  I_StartTic();
  G_BuildTiccmd(&players[consoleplayer]);
  ncmd = &g_array_index(
    players[consoleplayer].commands,
    netticcmd_t,
    players[consoleplayer].commands->len - 1
  );
  ncmd->index = local_command_index;
  if (DELTACLIENT) {
    D_Log(LOG_SYNC, "Built command { %d/%d %d %d %d %d %u %u }\n",
      ncmd->index,
      ncmd->tic,
      ncmd->cmd.forwardmove,
      ncmd->cmd.sidemove,
      ncmd->cmd.angleturn,
      ncmd->cmd.consistancy,
      ncmd->cmd.buttons,
      ncmd->cmd.chatchar
    );
  }
  local_command_index++;

  if (DELTACLIENT)
    g_array_append_val(local_commands, *ncmd);

  if (CLIENT)
    CL_MarkServerOutdated();

  P_PrintPlayerCommands(local_commands);
}

void P_RunAllPlayerCommands(player_t *player) {
  unsigned int command_count = P_GetPlayerCommandCount(player);

  for (unsigned int i = 0; i < command_count; i++) {
    netticcmd_t *ncmd = &g_array_index(player->commands, netticcmd_t, i);

    if (ncmd->index == 0 && ncmd->tic == 0)
      printf("Got blank command %u\n", i);

    D_Log(
      LOG_SYNC,
      "[%d: %td] P_RunAllPlayerCommands: After command %d"
      "{ %d/%d %d %d %d %d %u %u }\n",
      gametic, player - players,
      ncmd->index,
      i,
      ncmd->tic,
      ncmd->cmd.forwardmove,
      ncmd->cmd.sidemove,
      ncmd->cmd.angleturn,
      ncmd->cmd.consistancy,
      ncmd->cmd.buttons,
      ncmd->cmd.chatchar
    );

    memcpy(&player->cmd, &ncmd->cmd, sizeof(ticcmd_t));
    run_player_command(player);
    N_LogPlayerPosition(player);
    if (player->mo) {
      P_MobjThinker(player->mo);
      D_Log(LOG_SYNC, "After P_MobjThinker:\n");
      N_LogPlayerPosition(player);
    }
  }

  P_ClearPlayerCommands(player);
}

void P_RunBufferedCommands(player_t *player) {
  int saved_leveltime;
  unsigned int command_count = P_GetPlayerCommandCount(player);

  if (command_count == 0)
    return;

  for (unsigned int i = 0; i < command_count; i++) {
    netticcmd_t *ncmd = &g_array_index(player->commands, netticcmd_t, i);

    if (ncmd->tic > gametic)
      break;

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

    memcpy(&player->cmd, &ncmd->cmd, sizeof(ticcmd_t));
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
  }

  g_array_remove_range(player->commands, 0, command_count);
}

void P_RunNextCommand(player_t *player) {
  netticcmd_t *ncmd = NULL;

  if (P_GetPlayerCommandCount(player) == 0)
    return;

  ncmd = &g_array_index(player->commands, netticcmd_t, 0);
  memcpy(&player->cmd, &ncmd->cmd, sizeof(ticcmd_t));
  run_player_command(player);
  if (player->mo)
    P_MobjThinker(player->mo);
  g_array_remove_index(player->commands, 0);
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
  
  if (P_GetPlayerCommandCount(player) == 0 &&
      DELTACLIENT &&
      player != &players[consoleplayer] &&
      !player->mo) {
    P_PredictPlayerPosition(player);
    return;
  }

  if (DELTASERVER) {
    D_Log(LOG_SYNC, "(%d) Player %td has %d commands\n",
      gametic, player - players, P_GetPlayerCommandCount(player)
    );

    /*
     * CG: 08/13/2014: TODO:
     *     Fix skipping caused by running a shit-ton of commands in a single
     *     TIC.  This is gonna end up being some kind of vector function
     *     combined with a time limit which, if exceeded, disconnects the
     *     player.  For now though, just keep thinking on the player so their
     *     momentum keeps moving them.
     */
    P_RunBufferedCommands(player);

    return;
  }

  if (player == &players[consoleplayer])
    P_RunAllPlayerCommands(player);
  else
    P_RunNextCommand(player);
}

bool P_LoadCommandForTic(player_t *player, int tic) {
  unsigned int i;
  unsigned int command_count = P_GetPlayerCommandCount(player);

  for (i = 1; i <= command_count; i++) {
    netticcmd_t *ncmd = &g_array_index(player->commands, netticcmd_t, i - 1);

    if (ncmd->tic >= gametic)
      break;
  }

  if (i == command_count + 1)
    return false;

  g_array_remove_range(player->commands, 0, i);

  command_count = P_GetPlayerCommandCount(player);

  for (i = 0; i < command_count; i++) {
    netticcmd_t *ncmd = &g_array_index(player->commands, netticcmd_t, i);

    if (ncmd->tic == gametic) {
      memcpy(&player->cmd, &ncmd->cmd, sizeof(ticcmd_t));

      if (gamestate != GS_LEVEL || ncmd->tic <= gametic)
        g_array_remove_index(player->commands, i);

      return true;
    }
  }

  return false;
}

bool P_LoadLocalCommandForTic(int tic) {
  unsigned int command_count = P_GetLocalCommandCount();
  
  for (unsigned int i = 0; i < command_count; i++) {
    netticcmd_t *ncmd = &g_array_index(local_commands, netticcmd_t, i);

    if (ncmd->tic == gametic) {
      memcpy(&players[consoleplayer].cmd, &ncmd->cmd, sizeof(ticcmd_t));
      return true;
    }
  }

  return false;
}

void P_RemoveOldCommands(player_t *player, int tic) {
  unsigned int i;
  unsigned int command_count = P_GetPlayerCommandCount(player);

  if (command_count == 0)
    return;

  for (i = 1; i <= command_count; i++) {
    netticcmd_t *ncmd = &g_array_index(player->commands, netticcmd_t, i - 1);

    if (ncmd->tic >= tic)
      break;
  }

  if (i != 0)
    g_array_remove_range(player->commands, 0, i);
}

GArray* P_GetLocalCommands(void) {
  return local_commands;
}

netticcmd_t* P_GetNewBlankCommand(player_t *player) {
  unsigned int index = P_GetPlayerCommandCount(player);

  g_array_set_size(player->commands, index + 1);

  return &g_array_index(player->commands, netticcmd_t, index);
}

void P_EnsurePlayerCommandsSize(player_t *player, unsigned int min_size) {
  if (player->commands->len < min_size)
    g_array_set_size(player->commands, min_size);
}

/* vi: set et ts=2 sw=2: */

