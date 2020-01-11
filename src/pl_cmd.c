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

#include "enet/enet.h"

#include "doomdef.h"
#include "doomstat.h"
#include "d_event.h"
#include "e6y.h"
#include "g_game.h"
#include "i_video.h"
#include "n_main.h"
#include "p_user.h"
#include "r_defs.h"
#include "cl_main.h"
#include "cl_net.h"
#include "p_map.h"
#include "p_setup.h"
#include "p_mobj.h"
#include "p_spec.h"
#include "s_sound.h"
#include "sv_main.h"

#define NOEXTRAPOLATION 0
#define PMOBJTHINKER 1
#define COPIED_COMMAND 2
#define EXTRAPOLATION PMOBJTHINKER
#define MISSED_COMMAND_MAX 3
#define LOG_COMMANDS 0
#define PRINT_COMMANDS 1

void G_BuildTiccmd(ticcmd_t *cmd);

static GQueue *blank_command_queue;

#if LOG_COMMANDS
static void log_command(gpointer data, gpointer user_data) {
  netticcmd_t *ncmd = data;

  D_Msg(MSG_CMD, " %d/%d/%d", ncmd->index, ncmd->tic, ncmd->server_tic);
}
#endif

static void ignore_command(gpointer data, gpointer user_data) {
  netticcmd_t *ncmd = data;

  ncmd->server_tic = gametic;
}

static void run_queued_command(player_t *player, netticcmd_t *ncmd) {
  D_Msg(MSG_SYNC, "(%d) (%u) [%s] Running command %d/%d/%d\n",
    gametic,
    player->id,
    N_RunningStateName(),
    ncmd->index,
    ncmd->tic,
    ncmd->server_tic
  );

  CL_SetupCommandState(playernum, ncmd->index);

  player->cmd.forwardmove = ncmd->forward;
  player->cmd.sidemove    = ncmd->side;
  player->cmd.angleturn   = ncmd->angle;
  player->cmd.consistancy = 0;
  player->cmd.chatchar    = 0;
  player->cmd.buttons     = ncmd->buttons;

  leveltime = ncmd->tic;

  SV_UnlagSetTIC(ncmd->tic);

#if LOG_COMMANDS
  D_Msg(MSG_CMD, "(%d): {%d/%d/%d %d %d %d %u}\n",
    gametic,
    ncmd->index,
    ncmd->tic,
    ncmd->server_tic,
    ncmd->forward,
    ncmd->side,
    ncmd->angle,
    ncmd->buttons
  );
#endif

  PL_RunCommand(player);

  player->cmdq.commands_run_this_tic++;

  if (MULTINET && player->mo) {
    P_MobjThinker(player->mo);
  }

  D_Msg(MSG_SYNC,
    "(%d): %d: {%4d/%4d/%4d %4d/%4d/%4d %4d %4d/%4d/%4d/%4d %4d/%4u/%4u}\n",
    gametic,
    playernum,
    player->mo->x           >> FRACBITS,
    player->mo->y           >> FRACBITS,
    player->mo->z           >> FRACBITS,
    player->mo->momx        >> FRACBITS,
    player->mo->momy        >> FRACBITS,
    player->mo->momz        >> FRACBITS,
    player->mo->angle       /  ANG1,
    player->viewz           >> FRACBITS,
    player->viewheight      >> FRACBITS,
    player->deltaviewheight >> FRACBITS,
    player->bob             >> FRACBITS,
    player->prev_viewz      >> FRACBITS,
    player->prev_viewangle  /  ANG1,
    player->prev_viewpitch  /  ANG1
  );

  CL_ShutdownCommandState();

  player->cmdq.latest_command_run_index = ncmd->index;

  if (SERVER) {
    ncmd->server_tic = gametic;
  }
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
 *                A better solution is to predict a player's position based on
 *                their recent movement.  Essentially, this would mean saving
 *                their momx/y/z values and continuing to apply them inside
 *                command gaps.
 *
 *                Of course, this is a clientside enhancement only.  Were
 *                servers to apply something like this, it would irrevocably
 *                break clientside prediction.
 */

static bool extrapolate_player_position(player_t *player) {
  if (!CLIENT) {
    return false;
  }

  if (!PL_IsConsolePlayer(player)) {
    return false;
  }

  if (!player->mo) {
    return false;
  }

  if (!cl_extrapolate_player_positions) {
    return false;
  }

#if EXTRAPOLATION == PMOBJTHINKER
  P_MobjThinker(player->mo);
  return false;
#elif EXTRAPOLATION == COPIED_COMMAND
  netticcmd_t ncmd;

  if (player->cmdq.commands_missed >= MISSED_COMMAND_MAX) {
    D_Msg(MSG_WARN, "(%d) %td over missed command limit\n",
      gametic, player->id
    );
    return false;
  }

  player->cmdq.commands_missed++;

  ncmd.forward = player->cmd.forwardmove;
  ncmd.side    = player->cmd.sidemove;
  ncmd.angle   = player->cmd.angleturn;
  ncmd.buttons = 0;

  if (player->id != 1 && ncmd.forward != 0 && ncmd.side != 0 &&
                                              ncmd.angle != 0) {
    D_Msg(MSG_SYNC, "(%d) Re-running command: {%d, %d, %d}\n",
      gametic,
      ncmd.forward,
      ncmd.side,
      ncmd.angle
    );
  }

  PL_AppendNewCommand(player, &ncmd);

  return true;
#endif

  return false;
}

static void run_queued_player_commands(player_t *player) {
  int saved_leveltime = leveltime;

  if (SERVER && PL_IsConsolePlayer(player)) {
    return;
  }

  if (CLIENT && PL_IsConsolePlayer(player)) {
    PL_PrintCommands(playernum);
  }

  if (CLIENT && !CL_Synchronizing()) {
    player->cmdq.command_limit = 1;
  }
  else if (SERVER && sv_limit_player_commands) {
    player->cmdq.command_limit = SV_GetPlayerCommandLimit(player);
  }
  else {
    player->cmdq.command_limit = 0;
  }

  player->cmdq.commands_run_this_tic = 0;

  if (CLIENT && player->cmdq.commands->len >= 100) {
    I_Error("%zu commands for %d, exiting\n",
      player->cmdq.commands->len, player->id
    );
  }

  if (CLIENT && CL_Synchronizing()) {
    for (size_t i = 0; i < player->cmdq.commands->len; i++) {
      netticcmd_t *ncmd = g_ptr_array_index(player->cmdq.commands, i);

      if ((ncmd->index > player->cmdq.latest_command_run_index) &&
          (ncmd->server_tic == gametic)) {
        run_queued_command(player, ncmd);
      }
    }
  }
  else {
    for (size_t i = 0; i < player->cmdq.commands->len; i++) {
      netticcmd_t *ncmd = g_ptr_array_index(player->cmdq.commands, i);

      if (ncmd->index > player->cmdq.latest_command_run_index) {
        run_queued_command(player, ncmd);
        break;
      }
    }
  }

  leveltime = saved_leveltime;
}

static gint compare_commands(gconstpointer a, gconstpointer b) {
  netticcmd_t *ncmd_one = *(netticcmd_t **)a;
  netticcmd_t *ncmd_two = *(netticcmd_t **)b;

  if (ncmd_one->index < ncmd_two->index) {
    return -1;
  }

  if (ncmd_one->index > ncmd_two->index) {
    return 1;
  }

  if (ncmd_one->tic < ncmd_two->index) {
    return -1;
  }

  if (ncmd_one->tic > ncmd_two->index) {
    return 1;
  }

  if (ncmd_one->server_tic != 0 && ncmd_two->server_tic == 0) {
    return -1;
  }

  if (ncmd_one->server_tic == 0 && ncmd_two->server_tic != 0) {
    return 1;
  }

  if (ncmd_one->server_tic == 0 && ncmd_two->server_tic != 0) {
    return 1;
  }

  if (ncmd_one->server_tic < ncmd_two->server_tic) {
    return -1;
  }

  if (ncmd_one->server_tic > ncmd_two->server_tic) {
    return 1;
  }

  return 0;
}

static void recycle_command(gpointer data) {
  netticcmd_t *ncmd = (netticcmd_t *)data;

  memset(ncmd, 0, sizeof(netticcmd_t));

  g_queue_push_tail(blank_command_queue, ncmd);
}

static netticcmd_t* get_new_blank_command(void) {
  netticcmd_t *ncmd = g_queue_pop_head(blank_command_queue);

  if (!ncmd) {
    ncmd = calloc(1, sizeof(netticcmd_t));
  }

  if (!ncmd) {
    I_Error("get_new_blank_command: Error allocating new netticcmd");
  }

  return ncmd;
}

void PL_InitCommandQueues(void) {
  if (blank_command_queue) {
    return;
  }

  blank_command_queue = g_queue_new();
}

void PL_InitCommandQueue(player_t *player) {
  player->cmdq.commands = g_ptr_array_new_with_free_func(recycle_command);
  player->cmdq.commands_missed = 0;
  player->cmdq.command_limit = 0;
  player->cmdq.commands_run_this_tic = 0;
  player->cmdq.latest_command_run_index = 0;
}

size_t PL_GetCommandCount(player_t *player) {
  return player->cmdq.commands->len;
}

netticcmd_t* PL_GetCommand(player_t *player, size_t index) {
  if (index >= PL_GetCommandCount(player)) {
    return NULL;
  }

  return g_ptr_array_index(player->cmdq.commands, index);
}

void PL_QueueCommand(player_t *player, netticcmd_t *ncmd) {
  GPtrArray *commands = player->cmdq.commands;
  netticcmd_t *new_ncmd = get_new_blank_command();

  D_Msg(MSG_SYNC, "(%d) (%u) Queueing %d/%d/%d\n",
    gametic,
    player->id,
    ncmd->index,
    ncmd->tic,
    ncmd->server_tic
  );

  memcpy(new_ncmd, ncmd, sizeof(netticcmd_t));

  for (size_t i = 0; i < commands->len; i++) {
    netticcmd_t *stored_ncmd = (netticcmd_t *)g_ptr_array_index(commands, i);

    if (stored_ncmd->index == ncmd->index) {
      if ((stored_ncmd->server_tic == 0) && (ncmd->server_tic != 0)) {
        stored_ncmd->server_tic = ncmd->server_tic;
      }
      return;
    }

    if (stored_ncmd->index > ncmd->index) {
      g_ptr_array_insert(commands, i, new_ncmd);
      return;
    }
  }

  g_ptr_array_add(commands, new_ncmd);
}

void PL_AppendNewCommand(player_t *player, netticcmd_t *tmp_ncmd) {
  netticcmd_t *ncmd = get_new_blank_command();

  memcpy(ncmd, tmp_ncmd, sizeof(netticcmd_t));

  g_ptr_array_add(player->cmdq.commands, ncmd);
}

uint32_t PL_GetLatestCommandIndex(player_t *player) {
  netticcmd_t *ncmd = PL_GetCommand(player, PL_GetCommandCount(playernum) - 1);

  if (!ncmd) {
    return 0;
  }

  return ncmd->index;
}

void PL_UpdateCommandServerTic(player_t *player, uint32_t command_index,
                                                 uint32_t server_tic) {
  GPtrArray *commands = player->cmdq.commands;

  for (size_t i = 0; i < commands->len; i++) {
    netticcmd_t *ncmd = (netticcmd_t *)g_ptr_array_index(commands, i);

    if (ncmd->index == command_index) {
      ncmd->server_tic = server_tic;
      D_Msg(MSG_SYNC, "(%5d) Server ran %u at %u\n",
        gametic, ncmd->index, ncmd->server_tic
      );
      break;
    }
  }
}

void PL_ForEachCommand(player_t *player, GFunc func, gpointer user_data) {
  g_ptr_array_foreach(player->cmdq.commands, func, user_data);
}

void PL_ClearCommands(player_t *player) {
  unsigned int command_count;

  if (!playeringame[playernum]) {
    return;
  }

  D_Msg(MSG_CMD,
    "PL_ClearCommands: Clearing commands for %u\n", player->id
  );

  command_count = PL_GetCommandCount(playernum);

  if (command_count > 0) {
    g_ptr_array_remove_range(player->cmdq.commands, 0, command_count);
  }
}

void PL_ResetCommands(player_t *player) {
  command_queue_t *cmdq = cmdq = &player->cmdq;

  PL_ClearCommands(player);

  cmdq->commands_missed = 0;
  cmdq->command_limit = 0;
  cmdq->commands_run_this_tic = 0;
  cmdq->latest_command_run_index = 0;
}

void PL_IgnoreCommands(player_t *player) {
  PL_ForEachCommand(playernum, ignore_command, NULL);
}

void PL_TrimCommands(player_t *player, TrimFunc should_trim,
                                       gpointer user_data) {
  GPtrArray *commands;
  uint32_t command_count = 0;
  uint32_t i = 0;

  commands = player->cmdq.commands;
  command_count = commands->len;

  while (i < commands->len) {
    netticcmd_t *ncmd = g_ptr_array_index(commands, i);

    if (should_trim(ncmd, user_data)) {
      D_Msg(MSG_SYNC, "(%d) Removing command %d/%d/%d\n",
        gametic, ncmd->index, ncmd->tic, ncmd->server_tic
      );

      g_ptr_array_remove_index(commands, i);
    }
    else {
      i++;
    }
  }
}

void PL_BuildCommand(void) {
  ticcmd_t cmd;
  netticcmd_t ncmd;

  if (!CLIENT) {
    return;
  }

  memset(&cmd, 0, sizeof(ticcmd_t));
  G_BuildTiccmd(&cmd);

  ncmd.index      = CL_GetNextCommandIndex();
  ncmd.tic        = gametic;
  ncmd.server_tic = 0;
  ncmd.forward    = cmd.forwardmove;
  ncmd.side       = cmd.sidemove;
  ncmd.angle      = cmd.angleturn;
  ncmd.buttons    = cmd.buttons;

  PL_AppendNewCommand(PL_GetConsolePlayer(), &ncmd);

  if (CLIENT) {
    CL_MarkServerOutdated();
  }
}

bool PL_RunCommand(player_t *player) {
  ticcmd_t *cmd = &player->cmd;
  weapontype_t newweapon;

  /*
  if (CLIENT && player != &players[consoleplayer]) {
    D_Msg(MSG_CMD, "(%d) run_player_command: Running command for %td\n",
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

  if (player->playerstate == PST_DEAD) {
    P_DeathThink(player);
    return false;
  }

  if (player->jumpTics) {
    player->jumpTics--;
  }

  // Move around.
  // Reactiontime is used to prevent movement
  //  for a bit after a teleport.
  if (player->mo->reactiontime) {
    player->mo->reactiontime--;
  }
  else {
    P_MovePlayer(player);
  }

  P_SetPitch(player);

  P_CalcHeight(player); // Determines view height and bobbing

  // Determine if there's anything about the sector you're in that's
  // going to affect you, like painful floors.
  if (player->mo->subsector->sector->special) {
    P_PlayerInSpecialSector(player);
  }

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
      if (!prboom_comp[PC_ALLOW_SSG_DIRECT].state) {
        newweapon = (cmd->buttons & BT_WEAPONMASK_OLD) >> BT_WEAPONSHIFT;
      }

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

  return true;
}

bool PL_RunCommands(player_t *player) {
  player_t *player = &players[playernum];

  if (!MULTINET) {
    return PL_RunCommand(player);
  }

  if (CLIENT && !PL_IsConsolePlayer(player) && player->id != 1) {
    size_t command_count = PL_GetCommandCount(playernum);

    if (command_count == 0) {
      D_Msg(MSG_NET, "(%d) Extrapolating position for %d\n",
        gametic, playernum
      );
      return extrapolate_player_position(playernum);
    }
#if EXTRAPOLATION == COPIED_COMMAND
    else {
      player->cmdq.commands_missed = 0;
    }
#endif
  }

  run_queued_player_commands(player);

  return player->playerstate != PST_DEAD;
}

/* vi: set et ts=2 sw=2: */
