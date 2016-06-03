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
#include "n_net.h"
#include "p_user.h"
#include "n_main.h"
#include "n_state.h"
#include "n_peer.h"
#include "n_proto.h"
#include "r_defs.h"
#include "cl_main.h"
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

static bool run_player_command(player_t *player) {
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

  // Determine if there's anything about the sector you're in that's
  // going to affect you, like painful floors.
  if (player->mo->subsector->sector->special)
    P_PlayerInSpecialSector(player);

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

  return true;
}

static void process_queued_command(gpointer data, gpointer user_data) {
  netticcmd_t *ncmd = (netticcmd_t *)data;
  player_t *player = (player_t *)user_data;
  int playernum = player - players;

  /*
  printf("(%d) Command: %d/%d/%d, %d/%d/%d/%d\n",
    gametic,
    ncmd->index,
    ncmd->tic,
    ncmd->server_tic,
    ncmd->forward,
    ncmd->side,
    ncmd->angle,
    ncmd->buttons
  );
  */

  if (ncmd->index <= player->cmdq.latest_command_run_index) {
    if (SERVER || playernum == consoleplayer) {
      return;
    }
  }

  if (player->cmdq.command_limit &&
      player->cmdq.commands_run_this_tic >= player->cmdq.command_limit) {
    return;
  }

  if (CLIENT) {
    if (CL_Synchronizing()) {
      D_Msg(MSG_CMD, "(%d) Running synchronized command %d/%d/%d for %d\n",
        gametic, ncmd->index, ncmd->tic, ncmd->server_tic, playernum
      );

      if (ncmd->server_tic == 0) {
        return;
      }

      if (ncmd->server_tic != gametic) {
        return;
      }
    }

    if (CL_RePredicting()) {
      D_Msg(MSG_CMD, "(%d) Running re-predicted command %d/%d/%d for %d\n",
        gametic, ncmd->index, ncmd->tic, ncmd->server_tic, playernum
      );

      if (ncmd->server_tic != 0) {
        return;
      }

      if (ncmd->tic > gametic) {
        return;
      }
    }

    if (CL_Predicting()) {
      D_Msg(MSG_CMD, "(%d) Running predicted command %d/%d/%d for %d\n",
        gametic, ncmd->index, ncmd->tic, ncmd->server_tic, playernum
      );

      if (ncmd->tic != gametic) {
        return;
      }
    }
  }
  else {
    D_Msg(MSG_CMD, "(%d) Running command %d/%d/%d for %d\n",
      gametic, ncmd->index, ncmd->tic, ncmd->server_tic, playernum
    );
  }

  CL_SetupCommandState(playernum, ncmd);

  player->cmd.forwardmove = ncmd->forward;
  player->cmd.sidemove    = ncmd->side;
  player->cmd.angleturn   = ncmd->angle;
  player->cmd.consistancy = 0;
  player->cmd.chatchar    = 0;
  player->cmd.buttons     = ncmd->buttons;

  leveltime = ncmd->tic;

  SV_UnlagSetTIC(ncmd->tic);

  N_LogCommand(ncmd);

  run_player_command(player);

  player->cmdq.commands_run_this_tic++;

  if (MULTINET && player->mo)
    P_MobjThinker(player->mo);

  N_LogPlayerPosition(player);

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

  if ((!CLIENT) || (playernum == consoleplayer) || (!player->mo))
    return false;

  if (!cl_extrapolate_player_positions)
    return false;

#if EXTRAPOLATION == PMOBJTHINKER
  P_MobjThinker(player->mo);
  return false;
#elif EXTRAPOLATION == COPIED_COMMAND
  netticcmd_t ncmd;

  if (player->cmdq.commands_missed >= MISSED_COMMAND_MAX) {
    printf("(%d) %td over missed command limit\n", gametic, player - players);
    return false;
  }

  player->cmdq.commands_missed++;

  ncmd.forward = player->cmd.forwardmove;
  ncmd.side    = player->cmd.sidemove;
  ncmd.angle   = player->cmd.angleturn;
  ncmd.buttons = 0;

  if (player != &players[0] && ncmd.forward != 0 &&
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

  if (SERVER && playernum == consoleplayer) {
    return;
  }

  if (CLIENT && playernum == consoleplayer) {
    P_PrintCommands(playernum);
  }

  if (CLIENT && !CL_Synchronizing()) {
    player->cmdq.command_limit = 1;
  }
  else if (SERVER && sv_limit_player_commands) {
    player->cmdq.command_limit = SV_GetPlayerCommandLimit(playernum);
  }
  else {
    player->cmdq.command_limit = 0;
  }

  player->cmdq.commands_run_this_tic = 0;

  P_ForEachCommand(playernum, process_queued_command, player);

  leveltime = saved_leveltime;
}

#if 0
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
#endif

static gint compare_commands(gconstpointer a, gconstpointer b) {
  netticcmd_t *ncmd_one = *(netticcmd_t **)a;
  netticcmd_t *ncmd_two = *(netticcmd_t **)b;

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

static void recycle_command(gpointer data) {
  netticcmd_t *ncmd = (netticcmd_t *)data;

  D_Msg(MSG_SYNC, "recycle_command: Recycling command %d/%d\n",
    ncmd->index,
    ncmd->tic
  );

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

void P_InitCommandQueues(void) {
  if (!blank_command_queue) {
    blank_command_queue = g_queue_new();
  }

  for (int i = 0; i < MAXPLAYERS; i++) {
    players[i].cmdq.commands = g_ptr_array_new_with_free_func(recycle_command);
    players[i].cmdq.updated = 0;
    players[i].cmdq.latest_synchronized_index = 0xFFFFFFFF;
    players[i].cmdq.commands_missed = 0;
    players[i].cmdq.command_limit = 0;
    players[i].cmdq.commands_run_this_tic = 0;
    players[i].cmdq.latest_command_run_index = 0;
  }
}

void P_ResetLatestSynchronizedCommandIndex(int playernum) {
  players[playernum].cmdq.updated = 0;
  players[playernum].cmdq.latest_synchronized_index = 0xFFFFFFFF;
}

bool P_LatestSynchronizedCommandIndexReady(int playernum) {
  command_queue_t *cmdq = &players[playernum].cmdq;

  NETPEER_FOR_EACH(iter) {
    if ((cmdq->updated & (playernum << iter.np->playernum)) == 0) {
      return false;
    }
  }

  return true;
}

unsigned int P_GetLatestSynchronizedCommandIndex(int playernum) {
  return players[playernum].cmdq.latest_synchronized_index;
}

bool P_HasCommands(int playernum) {
  if (!playeringame[playernum]) {
    return false;
  }

  return P_GetCommandCount(playernum) > 0;
}

unsigned int P_GetCommandCount(int playernum) {
  if (!playeringame[playernum]) {
    return 0;
  }

  return players[playernum].cmdq.commands->len;
}

netticcmd_t* P_GetCommand(int playernum, unsigned int index) {
  if (!playeringame[playernum]) {
    return NULL;
  }

  if (index >= P_GetCommandCount(playernum)) {
    return NULL;
  }

  return g_ptr_array_index(players[playernum].cmdq.commands, index);
}

void P_InsertCommandSorted(int playernum, netticcmd_t *tmp_ncmd) {
  unsigned int current_command_count;
  bool found_command = false;

  if (!playeringame[playernum]) {
    return;
  }

  current_command_count = P_GetCommandCount(playernum);

  for (unsigned int j = 0; j < current_command_count; j++) {
    netticcmd_t *ncmd = P_GetCommand(playernum, j);

    if (ncmd->index != tmp_ncmd->index) {
      continue;
    }

    found_command = true;

    if (ncmd->tic     != tmp_ncmd->tic     ||
        ncmd->forward != tmp_ncmd->forward ||
        ncmd->side    != tmp_ncmd->side    ||
        ncmd->angle   != tmp_ncmd->angle   ||
        ncmd->buttons != tmp_ncmd->buttons) {
      printf(
        "P_UnArchivePlayer(%d): command mismatch: "
        "{%d, %d, %d, %d, %d, %d, %u} != {%d, %d, %d, %d, %d, %d, %u}\n",
        playernum,
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
    P_AppendNewCommand(playernum, tmp_ncmd);
    g_ptr_array_sort(players[playernum].cmdq.commands, compare_commands);
  }
}

void P_QueuePlayerCommand(int playernum, netticcmd_t *ncmd) {
  GPtrArray *commands = players[playernum].cmdq.commands;
  netticcmd_t *new_ncmd = get_new_blank_command();

  memcpy(new_ncmd, ncmd, sizeof(netticcmd_t));

  for (size_t i = 0; i < commands->len; i++) {
    netticcmd_t *stored_ncmd = (netticcmd_t *)g_ptr_array_index(commands, i);

    if (stored_ncmd->index == ncmd->index) {
      return;
    }

    if (stored_ncmd->index > ncmd->index) {
      g_ptr_array_insert(commands, i, new_ncmd);
      return;
    }
  }

  g_ptr_array_add(commands, new_ncmd);
}

void P_AppendNewCommand(int playernum, netticcmd_t *tmp_ncmd) {
  netticcmd_t *ncmd;

  if (!playeringame[playernum]) {
    return;
  }

  ncmd = get_new_blank_command();

  memcpy(ncmd, tmp_ncmd, sizeof(netticcmd_t));

  g_ptr_array_add(players[playernum].cmdq.commands, ncmd);
}

netticcmd_t* P_GetEarliestCommand(int playernum) {
  if (!playeringame[playernum]) {
    return NULL;
  }

  return P_GetCommand(playernum, 0);
}

netticcmd_t* P_GetLatestCommand(int playernum) {
  if (!playeringame[playernum]) {
    return NULL;
  }

  return P_GetCommand(playernum, P_GetCommandCount(playernum) - 1);
}

int P_GetEarliestCommandIndex(int playernum) {
  netticcmd_t *ncmd = P_GetEarliestCommand(playernum);

  if (!ncmd) {
    return -1;
  }

  return ncmd->index;
}

int P_GetLatestCommandIndex(int playernum) {
  netticcmd_t *ncmd = P_GetLatestCommand(playernum);

  if (!ncmd) {
    return -1;
  }

  return ncmd->index;
}

void P_UpdateCommandServerTic(int playernum, uint32_t command_index,
                                             uint32_t server_tic) {
  GPtrArray *commands = players[playernum].cmdq.commands;

  for (unsigned int i = 0; i < commands->len; i++) {
    netticcmd_t *ncmd = (netticcmd_t *)g_ptr_array_index(commands, i);

    if (ncmd->index == command_index) {
      ncmd->server_tic = server_tic;
      D_Msg(MSG_CMD, "(%5d) Server ran %u at %u\n",
        gametic, ncmd->index, ncmd->server_tic
      );
      break;
    }
  }
}

void P_UpdateLatestSynchronizedCommandIndex(int originating_playernum,
                                            int receiving_playernum,
                                            unsigned int command_index) {
  player_t *player = &players[originating_playernum];

  if (command_index < player->cmdq.latest_synchronized_index) {
    printf(
      "(%5d) Player %d has commands from %d up until %d\n",
      gametic, receiving_playernum, originating_playernum, command_index
    );

    player->cmdq.latest_synchronized_index = command_index;
  }
  else {
    printf(
      "(%5d) Player %d has commands from %d up until %d\n",
      gametic, receiving_playernum, originating_playernum, command_index
    );
  }

  player->cmdq.updated &= (1 << receiving_playernum);
}

void P_ForEachCommand(int playernum, GFunc func, gpointer user_data) {
  if (playeringame[playernum])
    g_ptr_array_foreach(players[playernum].cmdq.commands, func, user_data);
}

void P_ClearPlayerCommands(int playernum) {
  unsigned int command_count;

  if (!playeringame[playernum]) {
    return;
  }

  D_Msg(MSG_CMD,
    "P_ClearPlayerCommands: Clearing commands for %d\n", playernum
  );

  command_count = P_GetCommandCount(playernum);

  if (command_count > 0) {
    g_ptr_array_remove_range(
      players[playernum].cmdq.commands, 0, command_count
    );
  }
}

void P_IgnorePlayerCommands(int playernum) {
  if (!playeringame[playernum]) {
    return;
  }

  P_ForEachCommand(playernum, ignore_command, NULL);
}

void P_TrimCommands(int playernum, TrimFunc should_trim, gpointer user_data) {
  GPtrArray *commands;
  uint32_t command_count = 0;
  uint32_t i = 0;

  if (!playeringame[playernum]) {
    return;
  }

  commands = players[playernum].cmdq.commands;
  command_count = commands->len;

  while (i < commands->len) {
    netticcmd_t *ncmd = g_ptr_array_index(commands, i);

    if (should_trim(ncmd, user_data)) {
      g_ptr_array_remove_index(commands, i);
    }
    else {
      i++;
    }
  }

  D_Msg(MSG_CMD, "(%5d) Kept %u/%u commands for %d\n",
    gametic, commands->len, command_count, playernum
  );
}

#if 0
void P_TrimCommandsByTic(int playernum, int tic) {
  D_Msg(MSG_SYNC, "(%d) P_TrimCommandsByTic(%d)\n", gametic, tic);

  P_TrimCommands(playernum, command_tic_is_old, GINT_TO_POINTER(tic));
}

void P_TrimCommandsByIndex(int playernum, int command_index) {
  D_Msg(MSG_SYNC, "(%d) P_TrimCommandsByIndex(%d)\n", gametic, command_index);

  P_TrimCommands(
    playernum, command_index_is_old, GINT_TO_POINTER(command_index)
  );
}
#endif

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
  D_Msg(MSG_CMD, "(%d) P_BuildCommand: ", gametic);
  N_LogCommand(run_ncmd);
  */

  P_AppendNewCommand(consoleplayer, &ncmd);

  if (CLIENT)
    CL_MarkServerOutdated();
}

bool P_RunPlayerCommands(int playernum) {
  player_t *player = &players[playernum];

  if (!MULTINET) {
    return run_player_command(player);
  }

  if (CLIENT && playernum != consoleplayer && playernum != 0) {
    unsigned int command_count = P_GetCommandCount(playernum);

    if (command_count == 0) {
      return extrapolate_player_position(playernum);
    }
#if EXTRAPOLATION == COPIED_COMMAND
    else {
      player->cmdq.commands_missed = 0;

      /*
      printf("(%d) Got (%d) command(s) for %d\n",
        gametic, command_count, playernum
      );
      */
    }
#endif
  }

  run_queued_player_commands(playernum);
  return player->playerstate != PST_DEAD;
}

void P_PrintCommands(int playernum) {
  netpeer_t *server = CL_GetServerPeer();

  if (!server) {
    return;
  }

#if LOG_COMMANDS
  D_Msg(MSG_CMD, "(%5d) %d: %u (%u) {",
    gametic,
    playernum,
    server->command_indices[playernum],
    players[playernum].cmdq.commands->len
  );
  P_ForEachCommand(playernum, log_command, NULL);
  D_Msg(MSG_CMD, " }\n");
#endif
}

/* vi: set et ts=2 sw=2: */

