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

#include "i_net.h"
#include "g_game.h"
#include "g_state.h"
#include "p_defs.h"
#include "p_mobj.h"
#include "pl_cmd.h"
#include "pl_main.h"
#include "r_fps.h"
#include "s_sound.h"

#include "n_main.h"
#include "n_msg.h"
#include "n_peer.h"
#include "n_proto.h"
#include "cl_main.h"

#define MAX_SETUP_REQUEST_ATTEMPTS 10

typedef struct local_client_state_s {
  net_peer_t   *local_peer;
  net_peer_t   *server_peer;
  uint32_t      local_command_index;
  uint32_t      current_command_index;
  bool          running_consoleplayer_commands;
  bool          running_nonconsoleplayer_commands;
  bool          running_thinkers;
  bool          loading_state;
  bool          synchronizing;
  bool          repredicting;
  int           state_tic;
  int           reprediction_start_tic;
  int           reprediction_end_tic;
  auth_level_e  authorization_level;
  gamestate_e   new_gamestate;
  bool          needs_init_new;
} local_client_state_t;

static local_client_state_t local_client;

int cl_extrapolate_player_positions = false;

static void cl_set_repredicting(int start_tic, int end_tic) {
  local_client.reprediction_start_tic = start_tic;
  local_client.reprediction_end_tic = end_tic;
  local_client.repredicting = true;
}

static void cl_clear_repredicting(void) {
  local_client.reprediction_start_tic = 0;
  local_client.reprediction_end_tic = 0;
  local_client.repredicting = false;
}

static bool cl_load_new_state() {
  net_peer_t *server_peer = CL_GetServerPeer();
  netsync_t *sync = &server_peer->as.server.link.sync;
  game_state_delta_t *delta = N_SyncGetStateDelta(sync);
  bool state_loaded;

  if (!G_ApplyStateDelta(delta)) {
    D_MsgLocalError("Error applying state delta");
    return false;
  }

  local_client.loading_state = true;
  state_loaded = G_LoadLatestState(local_client.needs_init_new);
  local_client.loading_state = false;

  if (!state_loaded) {
    D_MsgLocalError("Error loading latest state");
    return false;
  }

  local_client.needs_init_new = false;

  local_client.loading_state = true;
  state_loaded = G_LoadState(delta->from_tic, false);
  local_client.loading_state = false;

  if (!state_loaded) {
    D_MsgLocalError("Error loading previous state");
    return false;
  }

  PLAYERS_FOR_EACH(iter) {
    if (iter.player->cmdq.commands->len) {
      N_MsgSyncLocalDebug("(%d) (%u) [ ", gametic, iter.player->id);

      for (size_t i = 0; i < iter.player->cmdq.commands->len; i++) {
        idxticcmd_t *icmd = g_ptr_array_index(iter.player->cmdq.commands, i);

        N_MsgSyncLocalDebug("%d/%d/%d ", icmd->index, icmd->tic, icmd->server_tic);
      }

      N_MsgSyncLocalDebug("]\n");
    }
  }

  local_client.synchronizing = true;

  N_MsgSyncLocalDebug("Synchronizing %d => %d\n", gametic, N_SyncGetTIC(sync));

  R_ResetViewInterpolation();

  for (int i = gametic; i <= N_SyncGetTIC(sync); i++) {
    N_RunTic();

    if (P_GetDisplayPlayer()->mo != NULL) {
      R_InterpolateView(P_GetDisplayPlayer());
      R_RestoreInterpolations();
    }
  }

  local_client.synchronizing = false;

  if (gametic != N_SyncGetTIC(sync) + 1) {
    D_MsgLocalWarn("Synchronization incomplete: %d, %d\n",
      gametic, N_SyncGetTIC(sync)
    );
  }

  N_MsgCmdLocalDebug("Ran sync'd commands, loading latest state\n");

  local_client.loading_state = true;
  state_loaded = G_LoadLatestState(false);
  local_client.loading_state = false;

  if (!state_loaded) {
    D_MsgLocalError("Error loading latest state");
    return false;
  }

  R_UpdateInterpolations();

  G_RemoveOldStates(delta->from_tic);

  if (local_client.new_gamestate != GS_BAD) {
    if ((local_client.new_gamestate != GS_LEVEL) &&
        (G_GetGameState() == GS_LEVEL)) {
      G_ExitLevel();
    }
    G_SetGameState(local_client.new_gamestate);
    local_client.new_gamestate = GS_BAD;
  }

  return true;
}

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

void CL_Init(void) {
  /*
   * The current command index, used for generating new commands
   */
  local_client.local_command_index = 1;

  /*
   * The index of the currently running command
   */
  local_client.current_command_index = 1;

  /*
   * Marked true when the client is running consoleplayer's commands
   */
  local_client.running_consoleplayer_commands = false;

  /*
   * Marked true when the client is running commands for a non-consoleplayer
   * player
   */
  local_client.running_nonconsoleplayer_commands = false;

  /*
   * Marked true when the client is running thinkers and specials
   */
  local_client.running_thinkers = false;

  /*
   * Marked true when the client is loading a state
   * NOTE: Currently used to avoid stopping orphaned sounds
   */
  local_client.loading_state = false;

  /*
   * The TIC of the last state received from the server
   */
  local_client.state_tic = -1;

  /*
   * Marked true when the client is synchronizing
   */
  local_client.synchronizing = false;

  /*
   * Marked true when the client is re-predicting
   */
  local_client.repredicting = false;

  /*
   * The TIC at which reprediction started.
   */
  local_client.reprediction_start_tic = 0;

  /*
   * The TIC at which reprediction will finish.
   */
  local_client.reprediction_end_tic = 0;

  /*
   * Set when the client receives an authorization message from the server
   * NOTE: Currently unimplemented.
   */
  local_client.authorization_level = AUTH_LEVEL_NONE;

  /*
   * Next gamestate to load after loading last state
   */
  local_client.new_gamestate = GS_BAD;

  /*
   * If set, calls G_InitNew next time the latest state is loaded
   */
  local_client.needs_init_new = false;

  /*
   * The server, if connected
   */
  local_client.server_peer = NULL;

  /*
   * The local peer, if connected
   */
  local_client.local_peer = NULL;

}

void CL_Reset(void) {
  local_client.local_command_index = 1;
  local_client.current_command_index = 1;
}

void CL_Connect(const char *address) {
  size_t host_length = 0;
  char *host = NULL;
  uint16_t port = 0;
  time_t start_request_time;

  host_length = I_NetParseAddressString(address, &host, &port);

  if (!host_length) {
    D_MsgLocalWarn(
      "CL_Connect: Invalid host (address: %s); using default host\n",
      address
    );

    host = strdup("0.0.0.0");
  }

  if (port == 0) {
    port = NET_DEFAULT_PORT;
  }

  D_MsgLocalInfo(
    "CL_Connect: Connecting to server %s:%u...\n", host, port
  );

  for (size_t i = 0; i < MAX_SETUP_REQUEST_ATTEMPTS; i++) {
    base_netpeer_t *base_server_peer = N_Connect(host, port);

    if (!base_server_peer) {
      D_MsgLocalError("CL_Connect: Connection refused\n");
      free(host);
      return;
    }

    D_MsgLocalInfo("CL_Connect: Connected!\n");

    local_client.server_peer = N_PeersAdd(base_server_peer, PEER_TYPE_SERVER);

    G_ReloadDefaults();

    D_MsgLocalInfo("CL_Connect: Requesting setup information...\n");

    start_request_time = time(NULL);

    while (true) {
      if (!I_NetConnected()) {
        break;
      }

      CL_SendSetupRequest();
      I_NetServiceNetwork();

      if (CL_ReceivedSetup()) {
        break;
      }

      if (difftime(time(NULL), start_request_time) > 10.0) {
        break;
      }
    }

    if (CL_ReceivedSetup()) {
      break;
    }
  }

  if (!CL_ReceivedSetup()) {
    D_MsgLocalError("CL_Connect: Timed out waiting for setup information\n");
  }
  else {
    D_MsgLocalInfo("CL_Connect: Setup information received!\n");
  }
}

void CL_Disconnect(void) {
  NET_PEERS_FOR_EACH(iter) {
    N_PeersIterateRemove(&iter);
  }

  I_NetDisconnect(DISCONNECT_REASON_MANUAL);
}

void CL_AbortNetgame(void) {
  /* [CG] [TODO] Go to fullscreen console or something */
  I_NetReset();
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
  size_t command_count;

  PL_ForEachCommand(player, count_command, &command_count);

  return command_count;
}

bool CL_RunningConsoleplayerCommands(void) {
  return local_client.running_consoleplayer_commands;
}

bool CL_RunningNonConsoleplayerCommands(void) {
  return local_client.running_nonconsoleplayer_commands;
}

bool CL_Predicting(void) {
  return !(CL_Synchronizing() || CL_RePredicting());
}

void CL_SetRunningThinkers(bool running) {
  local_client.running_thinkers = running;
}

bool CL_RunningThinkers(void) {
  return local_client.running_thinkers;
}

void CL_SetupCommandState(player_t *player, uint32_t command_index) {
  if (PL_IsConsolePlayer(player)) {
    local_client.running_consoleplayer_commands = true;
    local_client.running_nonconsoleplayer_commands = false;
  }
  else {
    local_client.running_consoleplayer_commands = false;
    local_client.running_nonconsoleplayer_commands = true;
  }

  if (local_client.running_consoleplayer_commands) {
    local_client.current_command_index = command_index;
  }
}

void CL_ShutdownCommandState(void) {
  local_client.running_consoleplayer_commands = false;
  local_client.running_nonconsoleplayer_commands = false;
}

uint32_t CL_GetCurrentCommandIndex(void) {
  return local_client.current_command_index;
}

uint32_t CL_GetNextCommandIndex(void) {
  uint32_t out = local_client.local_command_index;

  local_client.local_command_index++;

  return out;
}

net_peer_t* CL_GetServerPeer(void) {
  return local_client.server_peer;
}

void CL_CheckForStateUpdates(void) {
  net_peer_t *server_peer = CL_GetServerPeer();
  int saved_gametic = gametic;
  int saved_state_tic = N_SyncGetTIC(&server_peer->as.server.link.sync);
  int saved_state_delta_from_tic = N_SyncGetStateDelta(
    &server_peer->as.server.link.sync
  )->from_tic;
  int saved_state_delta_to_tic = N_SyncGetStateDelta(
    &server_peer->as.server.link.sync
  )->to_tic;

  if (!N_SyncUpdated(&server_peer->as.server.link.sync)) {
    return;
  }

  N_MsgSyncLocalDebug("(%d) Loading new state [%d, %d => %d] (%d)\n",
    gametic,
    N_SyncGetCommandIndex(&server_peer->as.server.link.sync),
    saved_state_delta_from_tic,
    saved_state_delta_to_tic,
    P_GetConsolePlayer()->cmdq.latest_command_run_index
  );

  S_ResetSoundLog();

  if (!cl_load_new_state()) {
    N_SyncResetStateDelta(
      &server_peer->as.server.link.sync,
      saved_state_tic,
      saved_state_delta_from_tic,
      saved_state_delta_to_tic
    );
    return;
  }

  gametic++;

  CL_TrimSynchronizedCommands();

  N_MsgSyncLocalDebug(
    "(%d): %u: {%4d/%4d/%4d %4d/%4d/%4d %4d %4d/%4d/%4d/%4d %4d/%4u/%4u}\n", 
    gametic,
    P_GetConsolePlayer()->id,
    P_GetConsolePlayer()->mo->x           >> FRACBITS,
    P_GetConsolePlayer()->mo->y           >> FRACBITS,
    P_GetConsolePlayer()->mo->z           >> FRACBITS,
    P_GetConsolePlayer()->mo->momx        >> FRACBITS,
    P_GetConsolePlayer()->mo->momy        >> FRACBITS,
    P_GetConsolePlayer()->mo->momz        >> FRACBITS,
    P_GetConsolePlayer()->mo->angle       /  ANG1,
    P_GetConsolePlayer()->viewz           >> FRACBITS,
    P_GetConsolePlayer()->viewheight      >> FRACBITS,
    P_GetConsolePlayer()->deltaviewheight >> FRACBITS,
    P_GetConsolePlayer()->bob             >> FRACBITS,
    P_GetConsolePlayer()->prev_viewz      >> FRACBITS,
    P_GetConsolePlayer()->prev_viewangle  /  ANG1,
    P_GetConsolePlayer()->prev_viewpitch  /  ANG1
  );

#ifdef LOG_SECTOR
  if (LOG_SECTOR < numsectors) {
    if (sectors[LOG_SECTOR].floorheight != (168 << FRACBITS) &&
        sectors[LOG_SECTOR].floorheight != (40 << FRACBITS)) {
      N_MsgSyncLocalDebug("(%d) Sector %d: %d/%d\n",
        gametic,
        LOG_SECTOR,
        sectors[LOG_SECTOR].floorheight >> FRACBITS,
        sectors[LOG_SECTOR].ceilingheight >> FRACBITS
      );
    }
  }
#endif

  if (G_GetGameState() == GS_LEVEL) {
    CL_RePredict(saved_gametic);
  }

  local_client.state_tic = N_SyncGetTIC(&server_peer->as.server.link.sync);

  N_SyncSetNotUpdated(&server_peer->as.server.link.sync);
  N_SyncSetOutdated(&server_peer->as.server.link.sync);

  S_TrimSoundLog(
    N_SyncGetStateDelta(&server_peer->as.server.link.sync)->from_tic,
    N_SyncGetCommandIndex(&server_peer->as.server.link.sync)
  );
}

void CL_MarkServerOutdated(void) {
  net_peer_t *server_peer = CL_GetServerPeer();

  N_SyncSetOutdated(&server_peer->as.server.link.sync);

  N_UpdateSync();
}

bool CL_OccurredDuringRePrediction(int tic) {
  if (!local_client.repredicting) {
    return false;
  }

  if (tic < local_client.reprediction_start_tic) {
    return false;
  }

  if (tic > local_client.reprediction_end_tic) {
    return false;
  }

  return true;
}

void CL_UpdateReceivedCommandIndex(uint32_t command_index) {
  net_peer_t *server_peer = CL_GetServerPeer();
  N_SyncUpdateCommandIndex(&server_peer->as.server.link.sync, command_index);
}

int CL_StateTIC(void) {
  return local_client.state_tic;
}

bool CL_ReceivedSetup(void) {
  net_peer_t *server_peer = CL_GetServerPeer();

  return !(N_SyncNeedsGameInfo(&server_peer->as.server.link.sync) ||
           N_SyncNeedsGameState(&server_peer->as.server.link.sync));
}

void CL_SetAuthorizationLevel(auth_level_e level) {
  if (level <= local_client.authorization_level) {
    return;
  }

  local_client.authorization_level = level;
}

void CL_SetNewGameState(gamestate_e new_gamestate) {
  switch (new_gamestate) {
    case GS_LEVEL:
    case GS_INTERMISSION:
    case GS_FINALE:
    case GS_DEMOSCREEN:
      local_client.new_gamestate = new_gamestate;
    break;
    default:
    break;
  }
}

void CL_RePredict(int saved_gametic) {
  player_t *player = P_GetConsolePlayer();
  uint32_t latest_command_index = PL_GetLatestCommandIndex(player);
  
  if (gametic == -1) {
    return;
  }

  if (latest_command_index == 0) {
    return;
  }

  cl_set_repredicting(
    player->cmdq.latest_command_run_index, latest_command_index
  );

  N_MsgSyncLocalDebug("Re-predicting %u => %u (%d)\n",
    player->cmdq.latest_command_run_index,
    latest_command_index,
    latest_command_index - player->cmdq.latest_command_run_index
  );

  while (player->cmdq.latest_command_run_index < latest_command_index) {
    N_RunTic();

    if (P_GetDisplayPlayer()->mo != NULL) {
      R_InterpolateView(P_GetDisplayPlayer());
      R_RestoreInterpolations();
    }
  }

  cl_clear_repredicting();
}

bool CL_LoadingState(void) {
  return local_client.loading_state;
}

bool CL_Synchronizing(void) {
  return local_client.synchronizing;
}

bool CL_RePredicting(void) {
  return local_client.repredicting;
}

void CL_SetNeedsInitNew(void) {
  local_client.needs_init_new = true;
}

void CL_ResetSync(void) {
  net_peer_t *server_peer = CL_GetServerPeer();

  local_client.state_tic = -1;
  local_client.reprediction_start_tic = 0;
  local_client.reprediction_end_tic = 0;

  PLAYERS_FOR_EACH(iter) {
    PL_ResetCommands(iter.player);
  }

  G_ClearStates();

  if (server_peer) {
    N_SyncClear(&server_peer->as.server.link.sync);
    N_SyncSetHasGameInfo(&server_peer->as.server.link.sync);
  }

  local_client.needs_init_new = true;
}

/* vi: set et ts=2 sw=2: */
