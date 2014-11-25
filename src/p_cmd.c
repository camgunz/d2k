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
#include "lprintf.h"
#include "n_net.h"
#include "n_main.h"
#include "n_state.h"
#include "n_proto.h"
#include "cl_main.h"
#include "p_cmd.h"
#include "p_map.h"
#include "p_mobj.h"
#include "p_pspr.h"
#include "p_user.h"
#include "s_sound.h"
#include "sv_main.h"

#define NOEXTRAPOLATION 0
#define PMOBJTHINKER 1
#define COPIED_COMMAND 2
#define EXTRAPOLATION PMOBJTHINKER
#define MISSED_COMMAND_MAX 3

static GQueue *blank_command_queue;

static void print_command(gpointer data, gpointer user_data) {
  netticcmd_t *ncmd = data;

  D_Log(LOG_SYNC, " %d/%d/%d", ncmd->index, ncmd->tic, ncmd->server_tic);
}

static void run_player_command(player_t *player) {
  ticcmd_t *cmd = &player->cmd;
  weapontype_t newweapon;

  /*
  if (CLIENT && player != &players[consoleplayer]) {
    D_Log(LOG_SYNC, "(%d) run_player_command: Running command for %td\n",
      gametic,
      player - players
    );
  }
  */

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

      if (newweapon == wp_fist && player->weaponowned[wp_chainsaw] &&
          (player->readyweapon != wp_chainsaw ||
           !player->powers[pw_strength])) {
        newweapon = wp_chainsaw;
      }

      if (gamemode == commercial &&
          newweapon == wp_shotgun &&
          player->weaponowned[wp_supershotgun] &&
          player->readyweapon != wp_supershotgun) {
        newweapon = wp_supershotgun;
      }
    }

    // killough 2/8/98, 3/22/98 -- end of weapon selection changes

    // Do not go to plasma or BFG in shareware, even if cheated.
    if (player->weaponowned[newweapon] &&
        newweapon != player->readyweapon &&
        ((newweapon != wp_plasma && newweapon != wp_bfg) ||
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

static void process_queued_command(gpointer data, gpointer user_data) {
  netticcmd_t *ncmd = (netticcmd_t *)data;
  player_t *player = (player_t *)user_data;
  int playernum = player - players;

  if (ncmd->index <= player->latest_command_run_index)
    return;

  if (player->command_limit &&
      player->commands_run_this_tic >= player->command_limit) {
    return;
  }

  if (CLIENT) {
    if (CL_Synchronizing()) {
      if (ncmd->server_tic == 0)
        return;

      if (ncmd->server_tic != gametic)
        return;
    }

    if (CL_RePredicting()) {
      if (ncmd->server_tic != 0)
        return;

      if (ncmd->tic > gametic)
        return;
    }

    if (CL_Predicting()) {
      if (ncmd->tic != gametic)
        return;
    }
  }

  CL_SetupCommandState(playernum, ncmd);

  player->cmd.forwardmove = ncmd->forward;
  player->cmd.sidemove    = ncmd->side;
  player->cmd.angleturn   = ncmd->angle;
  player->cmd.consistancy = 0;
  player->cmd.chatchar    = 0;
  player->cmd.buttons     = ncmd->buttons;

  leveltime = ncmd->tic;

  N_LogCommand(ncmd);

  run_player_command(player);

  player->commands_run_this_tic++;

  if (player->mo)
    P_MobjThinker(player->mo);

  N_LogPlayerPosition(player);

  CL_ShutdownCommandState();

  player->latest_command_run_index = ncmd->index;

  if (SERVER)
    ncmd->server_tic = gametic;
}

/*
 * CG 09/28/2014: It is not guaranteed that servers or players will receive
 *                player commands on a timely basis, rather, it is likely
 *                commands will be spread out and then later bunched together.
 *                My current understanding of the issue leads me to believe
 *                that the only remedy is to predict the player's ultimate
 *                position in the absence of commands, in hopes that when their
 *                commands are received and subsequently run, the predicted
 *                position will be close to the ultimate position.
 *
 *                For now, this prediction consists of simply running
 *                P_MobjThinker on the player, continuing to simulate their
 *                momentum (and other attributes).  This will work for very
 *                small gaps (1-5 TICs).
 *
 *                A better solution is to predict a player's position base on
 *                their recent movement.  Essentially, this would mean saving
 *                their momx/y/z values and continuing to apply them inside
 *                command gaps.
 *
 *                Of course, this is a clientside enhancement only.  Were
 *                servers to apply something like this, it would irrevocably
 *                break clientside prediction.
 */

static bool extrapolate_player_position(int playernum) {
  player_t *player = &players[playernum];

  if ((!DELTACLIENT) || (playernum == consoleplayer) || (!player->mo))
    return false;

  if (!cl_extrapolate_player_positions)
    return false;

#if EXTRAPOLATION == PMOBJTHINKER
  P_MobjThinker(player->mo);
  return false;
#elif EXTRAPOLATION == COPIED_COMMAND
  netticcmd_t ncmd;

  if (player->missed_commands >= MISSED_COMMAND_MAX) {
    printf("(%d) %td over missed command limit\n", gametic, player - players);
    return false;
  }

  player->missed_commands++;

  ncmd.forward = player->cmd.forwardmove;
  ncmd.side    = player->cmd.sidemove;
  ncmd.angle   = player->cmd.angleturn;
  ncmd.buttons = 0;

  if (player != &players[0] &&
      ncmd.forward != 0 &&
      ncmd.side != 0 &&
      ncmd.angle != 0) {
    printf("(%d) Re-running command: {%d, %d, %d}\n",
      gametic,
      ncmd.forward,
      ncmd.side,
      ncmd.angle
    );
  }

  P_AppendNewCommand(playernum, &ncmd);

  return true;
#endif
  return false;
}

static void run_queued_player_commands(int playernum) {
  int saved_leveltime = leveltime;
  player_t *player = &players[playernum];

  if (SERVER && playernum == consoleplayer)
    return;

  if (CLIENT && playernum == consoleplayer)
    P_PrintCommands(player->commands);

  if (CLIENT && !CL_Synchronizing())
    player->command_limit = 1;
  else if (SERVER && sv_limit_player_commands)
    player->command_limit = SV_GetPlayerCommandLimit(playernum);
  else
    player->command_limit = 0;

  player->commands_run_this_tic = 0;

  P_ForEachCommand(playernum, process_queued_command, player);

  leveltime = saved_leveltime;
}

static bool command_index_is_old(gpointer data, gpointer user_data) {
  netticcmd_t *ncmd = (netticcmd_t *)data;
  int command_index = GPOINTER_TO_INT(user_data);

  return ncmd->index <= command_index;
}

static bool command_tic_is_old(gpointer data, gpointer user_data) {
  netticcmd_t *ncmd = (netticcmd_t *)data;
  int tic = GPOINTER_TO_INT(user_data);

  return ncmd->tic <= tic;
}

static gint compare_commands(gconstpointer a, gconstpointer b,
                             gpointer user_data) {
  netticcmd_t *ncmd_one = (netticcmd_t *)a;
  netticcmd_t *ncmd_two = (netticcmd_t *)b;

  if (ncmd_one->index < ncmd_two->index)
    return -1;

  if (ncmd_one->index > ncmd_two->index)
    return 1;

  if (ncmd_one->tic < ncmd_two->index)
    return -1;

  if (ncmd_one->tic > ncmd_two->index)
    return 1;

  if (ncmd_one->server_tic != 0 && ncmd_two->server_tic == 0)
    return -1;

  if (ncmd_one->server_tic == 0 && ncmd_two->server_tic != 0)
    return 1;

  if (ncmd_one->server_tic == 0 && ncmd_two->server_tic != 0)
    return 1;

  if (ncmd_one->server_tic < ncmd_two->server_tic)
    return -1;

  if (ncmd_one->server_tic > ncmd_two->server_tic)
    return 1;

  return 0;
}

void P_InitCommands(void) {
  if (blank_command_queue == NULL)
    blank_command_queue = g_queue_new();

  for (int i = 0; i < MAXPLAYERS; i++)
    players[i].commands = g_queue_new();
}

bool P_HasCommands(int playernum) {
  if (!playeringame[playernum])
    return false;

  return !g_queue_is_empty(players[playernum].commands);
}

unsigned int P_GetCommandCount(int playernum) {
  if (!playeringame[playernum])
    return 0;

  return g_queue_get_length(players[playernum].commands);
}

netticcmd_t* P_GetCommand(int playernum, unsigned int index) {
  if (!playeringame[playernum])
    return NULL;

  return g_queue_peek_nth(players[playernum].commands, index);
}

netticcmd_t* P_PopCommand(int playernum, unsigned int index) {
  if (!playeringame[playernum])
    return NULL;

  return g_queue_pop_nth(players[playernum].commands, index);
}

void P_InsertCommandSorted(int playernum, netticcmd_t *tmp_ncmd) {
  unsigned int current_command_count;
  bool found_command = false;
  
  if (!playeringame[playernum])
    return;

  current_command_count = P_GetCommandCount(playernum);

  for (unsigned int j = 0; j < current_command_count; j++) {
    netticcmd_t *ncmd = g_queue_peek_nth(players[playernum].commands, j);

    if (ncmd->index != tmp_ncmd->index)
      continue;

    found_command = true;

    if (ncmd->tic     != tmp_ncmd->tic     ||
        ncmd->forward != tmp_ncmd->forward ||
        ncmd->side    != tmp_ncmd->side    ||
        ncmd->angle   != tmp_ncmd->angle   ||
        ncmd->buttons != tmp_ncmd->buttons) {
      P_Printf(consoleplayer,
        "P_UnArchivePlayer: command mismatch: "
        "{%d, %d, %d, %d, %d, %d, %u} != {%d, %d, %d, %d, %d, %d, %u}\n",
        ncmd->index,
        ncmd->tic,
        ncmd->server_tic,
        ncmd->forward,
        ncmd->side,
        ncmd->angle,
        ncmd->buttons,
        tmp_ncmd->index,
        tmp_ncmd->tic,
        tmp_ncmd->server_tic,
        tmp_ncmd->forward,
        tmp_ncmd->side,
        tmp_ncmd->angle,
        tmp_ncmd->buttons
      );
    }

    ncmd->server_tic = tmp_ncmd->server_tic;
  }

  if (!found_command) {
    netticcmd_t *ncmd = P_GetNewBlankCommand();

    memcpy(ncmd, tmp_ncmd, sizeof(netticcmd_t));
    g_queue_insert_sorted(
      players[playernum].commands, ncmd, compare_commands, NULL
    );
  }
}

void P_AppendNewCommand(int playernum, netticcmd_t *tmp_ncmd) {
  netticcmd_t *ncmd;

  if (!playeringame[playernum])
    return;
  
  ncmd = P_GetNewBlankCommand();

  memcpy(ncmd, tmp_ncmd, sizeof(netticcmd_t));

  g_queue_push_tail(players[playernum].commands, ncmd);
}

int P_GetLatestCommandIndex(int playernum) {
  netticcmd_t *ncmd;

  if (!playeringame[playernum])
    return -1;
  
  ncmd = g_queue_peek_tail(players[playernum].commands);

  if (ncmd == NULL)
    return -1;

  return ncmd->index;
}

void P_ForEachCommand(int playernum, GFunc func, gpointer user_data) {
  if (playeringame[playernum])
    g_queue_foreach(players[playernum].commands, func, user_data);
}

void P_ClearPlayerCommands(int playernum) {
  D_Log(LOG_SYNC, "P_ClearPlayerCommands: Clearing commands for %d\n",
    playernum
  );
  while (P_HasCommands(playernum))
    P_RecycleCommand(P_PopCommand(playernum, 0));
}

void P_TrimCommands(int playernum, TrimFunc should_trim, gpointer user_data) {
  while (P_HasCommands(playernum)) {
    netticcmd_t *ncmd = P_GetCommand(playernum, 0);

    if (!should_trim(ncmd, user_data))
      break;

    D_Log(LOG_SYNC, "P_TrimCommands: Removing command %d/%d\n",
      ncmd->index,
      ncmd->tic
    );

    P_RecycleCommand(P_PopCommand(playernum, 0));
  }
}

void P_RemoveOldCommands(int playernum, int command_index) {
  D_Log(LOG_SYNC, "(%d) P_RemoveOldCommands(%d)\n", gametic, command_index);

  P_TrimCommands(
    playernum, command_index_is_old, GINT_TO_POINTER(command_index)
  );
}

void P_RemoveOldCommandsByTic(int playernum, int tic) {
  D_Log(LOG_SYNC, "(%d) P_RemoveOldCommandsByTic(%d)\n", gametic, tic);

  P_TrimCommands(playernum, command_tic_is_old, GINT_TO_POINTER(tic));
}

netticcmd_t* P_GetNewBlankCommand(void) {
  netticcmd_t *ncmd = g_queue_pop_head(blank_command_queue);

  if (ncmd == NULL)
    ncmd = calloc(1, sizeof(netticcmd_t));

  if (ncmd == NULL)
    I_Error("P_GetNewBlankCommand: Error allocating new netticcmd");

  return ncmd;
}

void P_BuildCommand(void) {
  ticcmd_t cmd;
  netticcmd_t ncmd;

  if (!CLIENT)
    return;

  memset(&cmd, 0, sizeof(ticcmd_t));
  G_BuildTiccmd(&cmd);

  ncmd.index      = CL_GetNextCommandIndex();
  ncmd.tic        = gametic;
  ncmd.server_tic = 0;
  ncmd.forward    = cmd.forwardmove;
  ncmd.side       = cmd.sidemove;
  ncmd.angle      = cmd.angleturn;
  ncmd.buttons    = cmd.buttons;

  /*
  D_Log(LOG_SYNC, "(%d) P_BuildCommand: ", gametic);
  N_LogCommand(run_ncmd);
  */

  P_AppendNewCommand(consoleplayer, &ncmd);

  if (CLIENT)
    CL_MarkServerOutdated();
}

void P_RunPlayerCommands(int playernum) {
  player_t *player = &players[playernum];

  if (!(DELTACLIENT || DELTASERVER)) {
    run_player_command(player);
    return;
  }
  
  if (DELTACLIENT && playernum != consoleplayer && playernum != 0) {
    unsigned int command_count = P_GetCommandCount(playernum);

    if (command_count == 0) {
      if (!extrapolate_player_position(playernum))
        return;
    }
#if EXTRAPOLATION == COPIED_COMMAND
    else {
      player->missed_commands = 0;

      /*
      printf("(%d) Got (%d) command(s) for %d\n",
        gametic, command_count, playernum
      );
      */
    }
#endif
  }

  run_queued_player_commands(playernum);
}

void P_RecycleCommand(netticcmd_t *ncmd) {
  D_Log(LOG_SYNC, "P_RecycleCommand: Recycling command %d/%d\n",
    ncmd->index,
    ncmd->tic
  );

  memset(ncmd, 0, sizeof(netticcmd_t));

  g_queue_push_tail(blank_command_queue, ncmd);
}

void P_PrintCommands(GQueue *commands) {
  D_Log(LOG_SYNC, "{");
  g_queue_foreach(commands, print_command, NULL);
  D_Log(LOG_SYNC, " }\n");
}

/* vi: set et ts=2 sw=2: */

