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
#include "g_game.h"
#include "g_state.h"
#include "n_main.h"
#include "pl_main.h"
#include "pl_cmd.h"
#include "pl_msg.h"
#include "p_setup.h"
#include "p_mobj.h"
#include "r_defs.h"
#include "r_state.h"
#include "r_fps.h"
#include "s_sound.h"
#include "cl_cmd.h"
#include "cl_net.h"

/*
 * Marked true when the client is loading a state
 * NOTE: Currently used to avoid stopping orphaned sounds
 */
static bool cl_loading_state = false;

/*
 * The TIC of the last state received from the server
 */
static int cl_state_tic = -1;

/*
 * Marked true when the client is synchronizing
 */
static bool cl_synchronizing = false;

/*
 * Marked true when the client is re-predicting
 */
static bool cl_repredicting = false;

/*
 * The TIC at which reprediction started.
 */
static int cl_repredicting_start_tic = 0;

/*
 * The TIC at which reprediction will finish.
 */
static int cl_repredicting_end_tic = 0;

/*
 * Set when the client receives an authorization message from the server
 * NOTE: Currently unimplemented.
 */
static auth_level_e cl_authorization_level = AUTH_LEVEL_NONE;

/*
 * Next gamestate to load after loading last state
 */
static gamestate_t cl_new_gamestate = GS_BAD;

/*
 * If set, calls G_InitNew next time the latest state is loaded
 */
static bool cl_needs_init_new = false;

static void cl_set_repredicting(int start_tic, int end_tic) {
  cl_repredicting_start_tic = start_tic;
  cl_repredicting_end_tic = end_tic;
  cl_repredicting = true;
}

static void cl_clear_repredicting(void) {
  cl_repredicting_start_tic = 0;
  cl_repredicting_end_tic = 0;
  cl_repredicting = false;
}

static bool cl_load_new_state(netpeer_t *server) {
  game_state_delta_t *delta = N_PeerGetSyncStateDelta(server);
  bool state_loaded;
  size_t index = 0;;
  player_t *player;

  if (!G_ApplyStateDelta(delta)) {
    PL_Echo(P_GetConsolePlayer(), "Error applying state delta");
    return false;
  }

  cl_loading_state = true;
  state_loaded = G_LoadLatestState(cl_needs_init_new);
  cl_loading_state = false;

  if (!state_loaded) {
    PL_Echo(P_GetConsolePlayer(), "Error loading latest state");
    return false;
  }

  cl_needs_init_new = false;

  cl_loading_state = true;
  state_loaded = G_LoadState(delta->from_tic, false);
  cl_loading_state = false;

  if (!state_loaded) {
    PL_Echo(P_GetConsolePlayer(), "Error loading previous state");
    return false;
  }

  while (P_PlayersIter(&index, &player)) {
    if (player->cmdq.commands->len) {
      D_Msg(MSG_SYNC, "(%d) (%zu) [ ", gametic, index);

      for (size_t i = 0; i < player->cmdq.commands->len; i++) {
        idxticcmd_t *icmd = g_ptr_array_index(player->cmdq.commands, i);

        D_Msg(MSG_SYNC, "%d/%d/%d ", icmd->index, icmd->tic, icmd->server_tic);
      }

      D_Msg(MSG_SYNC, "]\n");
    }
  }

  cl_synchronizing = true;

  D_Msg(MSG_SYNC, "Synchronizing %d => %d\n",
    gametic, N_PeerGetSyncTIC(server)
  );

  R_ResetViewInterpolation();

  for (int i = gametic; i <= N_PeerGetSyncTIC(server); i++) {
    N_RunTic();

    if (P_GetDisplayPlayer()->mo != NULL) {
      R_InterpolateView(P_GetDisplayPlayer());
      R_RestoreInterpolations();
    }
  }

  cl_synchronizing = false;

  if (gametic != N_PeerGetSyncTIC(server) + 1) {
    D_Msg(MSG_WARN, "Synchronization incomplete: %d, %d\n",
      gametic, N_PeerGetSyncTIC(server)
    );
  }

  D_Msg(MSG_CMD, "Ran sync'd commands, loading latest state\n");

  cl_loading_state = true;
  state_loaded = G_LoadLatestState(false);
  cl_loading_state = false;

  if (!state_loaded) {
    PL_Echo(P_GetConsolePlayer(), "Error loading latest state");
    return false;
  }

  R_UpdateInterpolations();

  G_RemoveOldStates(delta->from_tic);

  if (cl_new_gamestate != GS_BAD) {
    if ((cl_new_gamestate != GS_LEVEL) && (G_GetGameState() == GS_LEVEL)) {
      G_ExitLevel();
    }
    G_SetGameState(cl_new_gamestate);
    cl_new_gamestate = GS_BAD;
  }

  return true;
}

netpeer_t* CL_GetServerPeer(void) {
  return N_PeerGet(1);
}

void CL_CheckForStateUpdates(void) {
  netpeer_t *server;
  int saved_gametic = gametic;
  int saved_state_tic;
  int saved_state_delta_from_tic;
  int saved_state_delta_to_tic;

  if (!CLIENT) {
    return;
  }

  server = CL_GetServerPeer();

  if (!server) {
    PL_Echo(P_GetConsolePlayer(), "Server disconnected");
    N_Disconnect(DISCONNECT_REASON_LOST_PEER_CONNECTION);
    return;
  }

  if (!N_PeerSyncUpdated(server)) {
    return;
  }

  saved_state_tic = N_PeerGetSyncTIC(server);
  saved_state_delta_from_tic = N_PeerGetSyncStateDelta(server)->from_tic;
  saved_state_delta_to_tic = N_PeerGetSyncStateDelta(server)->to_tic;

  D_Msg(MSG_SYNC, "(%d) Loading new state [%d, %d => %d] (%d)\n",
    gametic,
    N_PeerGetSyncCommandIndex(server),
    saved_state_delta_from_tic,
    saved_state_delta_to_tic,
    P_GetConsolePlayer()->cmdq.latest_command_run_index
  );

  S_ResetSoundLog();

  if (!cl_load_new_state(server)) {
    N_PeerResetSyncStateDelta(
      server,
      saved_state_tic,
      saved_state_delta_from_tic,
      saved_state_delta_to_tic
    );
    return;
  }

  gametic++;

  CL_TrimSynchronizedCommands();

  D_Msg(MSG_SYNC,
    "(%d): %d: {%4d/%4d/%4d %4d/%4d/%4d %4d %4d/%4d/%4d/%4d %4d/%4u/%4u}\n", 
    gametic,
    consoleplayer,
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
      D_Msg(MSG_SYNC, "(%d) Sector %d: %d/%d\n",
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

  cl_state_tic = N_PeerGetSyncTIC(server);

  N_PeerSyncSetNotUpdated(server);
  N_PeerSyncSetOutdated(server);

  S_TrimSoundLog(
    N_PeerGetSyncStateDelta(server)->from_tic,
    N_PeerGetSyncCommandIndex(server)
  );
}

void CL_MarkServerOutdated(void) {
  netpeer_t *server;

  if (!CLIENT) {
    return;
  }
  
  server = CL_GetServerPeer();

  if (server) {
    N_PeerSyncSetOutdated(server);
  }

  N_UpdateSync();
}

bool CL_OccurredDuringRePrediction(int tic) {
  if (!cl_repredicting) {
    return false;
  }

  if (tic < cl_repredicting_start_tic) {
    return false;
  }

  if (tic > cl_repredicting_end_tic) {
    return false;
  }

  return true;
}

void CL_UpdateReceivedCommandIndex(unsigned int command_index) {
  netpeer_t *server = CL_GetServerPeer();

  if (!server) {
    return;
  }

  N_PeerUpdateSyncCommandIndex(server, command_index);
}

int CL_StateTIC(void) {
  netpeer_t *server = CL_GetServerPeer();

  if (!server) {
    return -1;
  }

  return cl_state_tic;
}

bool CL_ReceivedSetup(void) {
  netpeer_t *server = CL_GetServerPeer();

  if (!server) {
    return false;
  }

  return !(N_PeerSyncNeedsGameInfo(server) || N_PeerSyncNeedsGameState(server));
}

void CL_SetAuthorizationLevel(auth_level_e level) {
  if (level <= cl_authorization_level) {
    return;
  }

  cl_authorization_level = level;
}

void CL_SetNewGameState(gamestate_t new_gamestate) {
  switch (new_gamestate) {
    case GS_LEVEL:
    case GS_INTERMISSION:
    case GS_FINALE:
    case GS_DEMOSCREEN:
      cl_new_gamestate = new_gamestate;
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

  D_Msg(MSG_SYNC, "Re-predicting %u => %u (%d)\n",
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
  return cl_loading_state;
}

bool CL_Synchronizing(void) {
  return (CLIENT && cl_synchronizing);
}

bool CL_RePredicting(void) {
  return (CLIENT && cl_repredicting);
}

void CL_SetNeedsInitNew(void) {
  cl_needs_init_new = true;
}

void CL_ResetSync(void) {
  size_t index = 0;
  player_t *player = NULL;
  netpeer_t *server = CL_GetServerPeer();

  cl_state_tic = -1;
  cl_repredicting_start_tic = 0;
  cl_repredicting_end_tic = 0;

  while (P_PlayersIter(&index, &player)) {
    PL_ResetCommands(player);
  }

  G_ClearStates();

  if (server) {
    N_PeerResetSync(server);
    N_PeerSyncSetHasGameInfo(server);
  }

  cl_needs_init_new = true;
}

/* vi: set et ts=2 sw=2: */
