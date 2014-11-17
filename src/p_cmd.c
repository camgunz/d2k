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

bool extrapolate_player_position(int playernum) {
  player_t *player = &players[playernum];

  if ((!DELTACLIENT) || (player == &players[consoleplayer]) || (!player->mo))
    return false;

  if (!cl_extrapolate_player_positions)
    return false;

#if EXTRAPOLATION == PMOBJTHINKER
  P_MobjThinker(player->mo);
  return false;
#elif EXTRAPOLATION == COPIED_COMMAND
  netticcmd_t *ncmd;

  if (player->missed_commands >= MISSED_COMMAND_MAX) {
    printf("(%d) %td over missed command limit\n", gametic, player - players);
    return false;
  }

  player->missed_commands++;

  ncmd = P_GetNewBlankCommand();

  ncmd->forward = player->cmd.forwardmove;
  ncmd->side    = player->cmd.sidemove;
  ncmd->angle   = player->cmd.angleturn;
  ncmd->buttons = 0;

  if (player != &players[0] &&
      ncmd->forward != 0 &&
      ncmd->side != 0 &&
      ncmd->angle != 0) {
    printf("(%d) Re-running command: {%d, %d, %d}\n",
      gametic,
      ncmd->forward,
      ncmd->side,
      ncmd->angle
    );
  }

  if (ncmd == NULL)
    puts("extrapolate_player_position: Pushing NULL netticcmd");

  g_queue_push_tail(player->commands, ncmd);

  return true;
#endif
  return false;
}

void run_queued_player_commands(int playernum) {
  player_t *player = &players[playernum];
  unsigned int command_limit;
  unsigned int command_count = g_queue_get_length(player->commands);
  unsigned int commands_run = 0;

  if (SERVER && playernum == consoleplayer)
    return;

  if (SERVER && sv_limit_player_commands)
    command_limit = SV_GetPlayerCommandLimit(playernum);

  if (CLIENT && playernum == consoleplayer)
    P_PrintCommands(player->commands);

  for (unsigned int i = 0; i < command_count; i++) {
    int saved_leveltime = leveltime;
    netticcmd_t *ncmd = g_queue_peek_nth(player->commands, i);

    if (ncmd->index <= player->latest_command_run_index)
      continue;

    if (CLIENT) {
      if (CL_Synchronizing()) {
        if (ncmd->server_tic == 0)
          continue;

        if (ncmd->server_tic != gametic)
          continue;
      }

      if (CL_RePredicting()) {
        if (ncmd->server_tic != 0)
          continue;

        if (ncmd->tic > gametic)
          continue;
      }

      if (CL_Predicting()) {
        if (ncmd->tic != gametic)
          continue;
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

    if (player->mo)
      P_MobjThinker(player->mo);

    N_LogPlayerPosition(player);

    CL_ShutdownCommandState();

    leveltime = saved_leveltime;

    player->latest_command_run_index = ncmd->index;

    if (CLIENT && !CL_Synchronizing())
      break;

    if (SERVER) {
      ncmd->server_tic = gametic;

      if (sv_limit_player_commands) {
        commands_run++;

        if (commands_run >= command_limit)
          break;
      }
    }
  }
}

void P_InitCommands(void) {
  if (blank_command_queue == NULL)
    blank_command_queue = g_queue_new();

  for (int i = 0; i < MAXPLAYERS; i++)
    players[i].commands = g_queue_new();
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
  netticcmd_t *run_ncmd;

  if (!CLIENT)
    return;

  run_ncmd = P_GetNewBlankCommand();

  memset(&cmd, 0, sizeof(ticcmd_t));
  G_BuildTiccmd(&cmd);

  run_ncmd->index   = CL_GetNextCommandIndex();
  run_ncmd->tic     = gametic;
  run_ncmd->forward = cmd.forwardmove;
  run_ncmd->side    = cmd.sidemove;
  run_ncmd->angle   = cmd.angleturn;
  run_ncmd->buttons = cmd.buttons;

  /*
  D_Log(LOG_SYNC, "(%d) P_BuildCommand: ", gametic);
  N_LogCommand(run_ncmd);
  */

  if (run_ncmd == NULL)
    puts("P_BuildCommand: Pushing NULL netticcmd");

  g_queue_push_tail(players[consoleplayer].commands, run_ncmd);

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
    int command_count = g_queue_get_length(players[playernum].commands);

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

void P_ClearPlayerCommands(int playernum) {
  D_Log(LOG_SYNC, "P_ClearPlayerCommands: Clearing commands for %d\n",
    playernum
  );
  while (!g_queue_is_empty(players[playernum].commands))
    P_RecycleCommand(g_queue_pop_head(players[playernum].commands));
}

void P_RemoveOldCommands(int playernum, int command_index) {
  GQueue *commands = players[playernum].commands;

  D_Log(LOG_SYNC, "(%d) P_RemoveOldCommands(%d)\n", gametic, command_index);

  while (!g_queue_is_empty(commands)) {
    netticcmd_t *ncmd = g_queue_peek_head(commands);

    if (ncmd->index > command_index)
      break;

    D_Log(LOG_SYNC, "P_RemoveOldCommands: Removing command %d/%d\n",
      ncmd->index,
      ncmd->tic
    );

    P_RecycleCommand(g_queue_pop_head(commands));
  }
}

void P_RemoveOldCommandsByTic(int playernum, int tic) {
  GQueue *commands = players[playernum].commands;

  D_Log(LOG_SYNC, "(%d) P_RemoveOldCommandsByTic(%d)\n", gametic, tic);

  while (!g_queue_is_empty(commands)) {
    netticcmd_t *ncmd = g_queue_peek_head(commands);

    if (ncmd->tic > tic)
      break;

    D_Log(LOG_SYNC, "P_RemoveOldCommandsByTic: Removing command %d/%d\n",
      ncmd->index,
      ncmd->tic
    );

    P_RecycleCommand(g_queue_pop_head(commands));
  }
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

