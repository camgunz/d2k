/* Emacs style mode select   -*- C++ -*-
 *-----------------------------------------------------------------------------
 *
 *
 *  PrBoom: a Doom port merged with LxDoom and LSDLDoom
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 1999 by
 *  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
 *  Copyright (C) 1999-2000 by
 *  Jess Haas, Nicolas Kalkhof, Colin Phipps, Florian Schulze
 *  Copyright 2005, 2006 by
 *  Florian Schulze, Colin Phipps, Neil Stevens, Andrey Budko
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 *  02111-1307, USA.
 *
 * DESCRIPTION:
 *
 *
 *-----------------------------------------------------------------------------
 */

#include "z_zone.h"

#include <enet/enet.h>
#include "cmp.h"

#include "doomstat.h"
#include "protocol.h"

#include "d_main.h"
#include "g_game.h"
#include "i_network.h"
#include "i_system.h"
#include "i_main.h"
#include "i_video.h"
#include "lprintf.h"
#include "m_argv.h"
#include "m_delta.h"
#include "m_menu.h"
#include "m_pbuf.h"
#include "m_utf.h"
#include "p_checksum.h"
#include "p_user.h"
#include "r_fps.h"
#include "e6y.h"

#include "n_net.h"
#include "n_main.h"
#include "n_state.h"
#include "n_peer.h"
#include "n_proto.h"

/* CG: TODO: Add WAD fetching (waiting on libcurl) */

static dboolean is_extra_ddisplay = false;

/* CG: Client only */
static dboolean received_setup = false;
static auth_level_e authorization_level = AUTH_LEVEL_NONE;

static void build_command(void) {
  cbuf_t *commands = P_GetPlayerCommands(consoleplayer);
  netticcmd_t *ncmd = M_CBufGetFirstFreeOrNewSlot(commands);

  I_StartTic();
  M_CBufConsolidate(commands);
  G_BuildTiccmd(ncmd);

  M_CBufAppend(&players[consoleplayer].commands, ncmd);
}

static void rerun_local_commands(void) {
  if (!DELTACLIENT)
    return;

  printf("(%d) Re-running local commands.\n", gametic);
  CL_RunLocalCommands();
}

static void run_tic(void) {
  if (advancedemo)
    D_DoAdvanceDemo();

  M_Ticker();
  I_GetTime_SaveMS();
  G_Ticker();
  P_Checksum(gametic);
  gametic++;
}

static int run_tics(int tic_count) {
  int out = tic_count;

  while (tic_count--) {
    build_command();
    run_tic();
  }

  return out;
}

static int run_commandsync_tics(int command_count) {
  int tic_count = command_count;

  for (int i = 0; i < command_count; i++)
    build_command();

  for (int i = 0; i < MAXPLAYERS; i++) {
    if (playeringame[i]) {
      tic_count = MIN(tic_count, M_CBufGetObjectCount(P_GetPlayerCommands(i)));
    }
  }

  if (tic_count)
    run_tics(tic_count);

  if (CMDCLIENT && (command_count > 0)) {
    netpeer_t *server = N_PeerGet(0);

    if (server != NULL)
      server->needs_sync_update = true;
  }

  return tic_count;
}

static int run_deltasync_tics(int tic_count) {
  int out = tic_count;
  dboolean clients_need_updating = DELTASERVER && (tic_count > 0);
  dboolean server_needs_updating = DELTACLIENT && (tic_count > 0);

  while (tic_count--) {
    build_command();
    run_tic();

    if (DELTASERVER)
      N_SaveState();
  }

  if (server_needs_updating) {
    netpeer_t *server = N_PeerGet(0);

    if (server != NULL)
      server->needs_sync_update = true;
  }

  if (clients_need_updating) {
    for (int i = 0; i < N_PeerGetCount(); i++) {
      netpeer_t *client = N_PeerGet(i);

      if (client != NULL)
        client->needs_sync_update = true;
    }
  }

  return out;
}

static int process_tics(int tics_elapsed) {
  /*
  printf("(%d) Running %d TICs.\n", gametic, tics_elapsed);
  */

  if (tics_elapsed <= 0)
    return 0;

  if (!MULTINET)
    return run_tics(tics_elapsed);

  if (CMDSYNC)
    return run_commandsync_tics(tics_elapsed);

  if (DELTASYNC)
    return run_deltasync_tics(tics_elapsed);

  return 0;
}

static void setup_new_peers(void) {
  if (!SERVER)
    return;

  for (int i = 0; i < N_PeerGetCount(); i++) {
    netpeer_t *np = N_PeerGet(i);

    if (np != NULL && np->needs_setup != 0 && gametic > np->needs_setup)
      SV_SendSetup(np->playernum);
  }
}

static void render_menu(int menu_renderer_calls) {
  while (menu_renderer_calls--)
    M_Ticker();
}

static void service_network(void) {
  if (MULTINET) {
    if (CLIENT)
      printf("(%d) Updating sync & servicing network.\n", gametic);

    N_UpdateSync();
    N_ServiceNetwork();

    if (DELTASERVER && N_PeerGetCount() == 0)
      N_RemoveOldStates(gametic);
  }
}

static void render_extra_frame(void) {
  dboolean should_render = false;

  WasRenderedInTryRunTics = true;

  should_render = movement_smooth && gamestate == wipegamestate;

#ifdef GL_DOOM
  if (V_GetMode() == VID_MODEGL)
    should_render = true;
#endif

  if (should_render) {
    is_extra_ddisplay = true;
    D_Display();
    is_extra_ddisplay = false;
  }
}

void N_PrintPlayerCommands(cbuf_t *commands) {
  printf("[ ");

  CBUF_FOR_EACH(commands, entry)
    printf("%d ", ((netticcmd_t *)entry.obj)->tic);

  printf("]\n");
}

void N_InitNetGame(void) {
  int i;

  netgame   = false;
  solonet   = false;
  netserver = false;
  netsync   = NET_SYNC_TYPE_NONE;

  displayplayer = consoleplayer = 0;
  playeringame[consoleplayer] = true;

  M_InitDeltas();

  if ((i = M_CheckParm("-solo-net"))) {
    netgame = true;
    solonet = true;
  }
  else if ((i = M_CheckParm("-net"))) {
    if (i < myargc - 1) {
      char *host = NULL;
      unsigned short port = 0;

      netgame = true;

      N_Init();

      N_ParseAddressString(myargv[i + 1], &host, &port);

      if (N_Connect(host, port))
        printf("N_InitNetGame: Connected to server %s:%u.\n", host, port);
      else
        I_Error("N_InitNetGame: Connection aborted");


      N_ServiceNetworkTimeout(CONNECT_TIMEOUT * 1000);

      if (N_PeerGet(0) == NULL)
        I_Error("N_InitNetGame: Timed out connecting to server");

      N_ServiceNetworkTimeout(CONNECT_TIMEOUT * 1000);

      if (received_setup)
        doom_printf("N_InitNetGame: Setup information received.\n");
      else
        I_Error("N_InitNetGame: Timed out waiting for setup information.");

      atexit(N_Disconnect);
    }
  }
  else {
    if (M_CheckParm("-commandserve") && M_CheckParm("-deltaserve"))
      I_Error("Cannot specify both '-commandserve' and '-deltaserve'");
    else if ((i = M_CheckParm("-commandserve")))
      netsync = NET_SYNC_TYPE_COMMAND;
    else if ((i = M_CheckParm("-deltaserve")))
      netsync = NET_SYNC_TYPE_DELTA;

    if (CMDSYNC || DELTASYNC) {
      char *host = NULL;
      unsigned short port = DEFAULT_PORT;

      netgame = true;
      netserver = true;

      nodrawers   = true;
      nosfxparm   = true;
      nomusicparm = true;

      N_Init();

      if (i < myargc - 1) {
        size_t host_length = N_ParseAddressString(myargv[i + 1], &host, &port);

        if (host_length == 0)
          host = strdup("0.0.0.0");
      }
      else {
        host = strdup("0.0.0.0");
      }

      if (N_Listen(host, port))
        doom_printf("N_InitNetGame: Listening on %s:%u.\n", host, port);
      else
        I_Error("Startup aborted");

      atexit(N_Disconnect);
    }
  }

  for (int i = 0; i < MAXPLAYERS; i++) {
    M_CBufInitWithCapacity(
      P_GetPlayerCommands(i), sizeof(netticcmd_t), BACKUPTICS
    );
  }

  M_CBufInitWithCapacity(
    &players[consoleplayer].commands, sizeof(netticcmd_t), BACKUPTICS
  );
}

void N_Update(void) {
  if (is_extra_ddisplay)
    return;

  if (MULTINET)
    N_ServiceNetwork();

#if 0
    // e6y
    // Eliminating the sudden jump of six frames(BACKUPTICS/2) 
    // after change of realtic_clock_rate.
    if (maketic - gametic &&
        gametic <= force_singletics_to &&
        realtic_clock_rate < 200) {
      break;
    }
#endif
}

dboolean N_GetWad(const char *name) {
  return false;
}

dboolean CL_ReceivedSetup(void) {
  return received_setup;
}

void CL_SetReceivedSetup(dboolean new_received_setup) {
  received_setup = new_received_setup;
}

void CL_SetAuthorizationLevel(auth_level_e level) {
  if (level > authorization_level)
    authorization_level = level;
}

void CL_LoadState(void) {
  netpeer_t *server = N_PeerGet(0);
  game_state_delta_t *delta = NULL;
  cbuf_t *local_commands = P_GetPlayerCommands(consoleplayer);
  
  if (server == NULL)
    return;

  delta = &server->delta;
  printf("(%d) Applying delta from %d to %d.\n",
    gametic, delta->from_tic, delta->to_tic
  );

  if (!N_ApplyStateDelta(delta)) {
    doom_printf("Error applying state delta.\n");
    return;
  }

  if (!N_LoadLatestState(false)) {
    doom_printf("Error loading state.\n");
    return;
  }

  server->state_tic = delta->to_tic;

  CBUF_FOR_EACH(local_commands, entry) {
    netticcmd_t *ncmd = (netticcmd_t *)entry.obj;

    if (ncmd->tic <= server->command_tic) {
      printf("(%d) Removing old local command (%d <= %d).\n",
        gametic, ncmd->tic, server->command_tic
      );
      M_CBufRemove(local_commands, entry.index);
      entry.index--;
    }
  }
}

void CL_RunLocalCommands(void) {
  cbuf_t *player_commands = &players[consoleplayer].commands;
  cbuf_t *local_commands = P_GetPlayerCommands(consoleplayer);

#if 0
  printf("Player commands: ");
  N_PrintPlayerCommands(player_commands);
  printf("Local commands: ");
  N_PrintPlayerCommands(local_commands);
#endif

  if (M_CBufGetObjectCount(local_commands) == 0)
    return;

  CBUF_FOR_EACH(local_commands, entry) {
    netticcmd_t *ncmd = (netticcmd_t *)entry.obj;

    if (ncmd->tic < gametic)
      continue;

    printf("(%d) Running local command [%d]\n", gametic, ncmd->tic);

    M_CBufAppend(player_commands, ncmd);
    is_extra_ddisplay = true;
    run_tic();
    is_extra_ddisplay = false;
  }

  CL_RemoveOldStates();
}

void CL_RemoveOldStates(void) {
  netpeer_t *server = N_PeerGet(0);

  if (server != NULL)
    N_RemoveOldStates(server->delta.from_tic);
}


void SV_RemoveOldCommands(void) {
  int oldest_state_tic = gametic;

  for (int i = 0; i < N_PeerGetCount(); i++) {
    netpeer_t *client = N_PeerGet(i);

    if (client != NULL && client->state_tic)
      oldest_state_tic = MIN(oldest_state_tic, client->state_tic);
  }

  for (int i = 0; i < N_PeerGetCount(); i++) {
    cbuf_t *commands = NULL;
    netpeer_t *client = N_PeerGet(i);

    if (client == NULL)
      continue;

    commands = P_GetPlayerCommands(client->playernum);

    CBUF_FOR_EACH(commands, entry) {
      netticcmd_t *ncmd = (netticcmd_t *)entry.obj;

      if (ncmd->tic < oldest_state_tic) {
        printf("(%d: %d) Removing old command %d.\n",
          gametic, client->playernum, ncmd->tic
        );
        M_CBufRemove(commands, entry.index);
        entry.index--;
      }
    }
  }
}

void SV_RemoveOldStates(void) {
  int oldest_state_tic = gametic;

  for (int i = 0; i < N_PeerGetCount(); i++) {
    netpeer_t *np = N_PeerGet(i);

    if (np != NULL)
      oldest_state_tic = MIN(oldest_state_tic, np->state_tic);
  }

  N_RemoveOldStates(oldest_state_tic);
}

void N_TryRunTics(void) {
  static int tics_built = 0;

  int tics_elapsed = I_GetTime() - tics_built;
  int menu_renderer_calls = tics_elapsed * 3;

  int tics_run = 0;
  dboolean render_fast = false;
  
  render_fast = movement_smooth && (!window_focused) && (gametic > 0);

#ifdef GL_DOOM
  if (V_GetMode() == VID_MODEGL)
    render_fast = true;
#endif

  if ((!MULTINET) && (!render_fast) && (tics_elapsed <= 0))
    I_uSleep(ms_to_next_tick * 1000);

  if (CLIENT && N_PeerGet(0) != NULL) {
    netpeer_t *s = N_PeerGet(0);

    printf("\n=== Starting new loop (%d/%d)\n", s->state_tic, s->command_tic);
  }

  if (tics_elapsed) {
    tics_built += tics_elapsed;

    if (ffmap)
      tics_elapsed++;

    rerun_local_commands();

    tics_run = process_tics(tics_elapsed);

    render_menu(MAX(menu_renderer_calls - tics_run, 0));

    setup_new_peers();
  }

  service_network();

  if (CLIENT && gametic > 0 && N_PeerGet(0) == NULL)
    I_Error("Server disconnected.\n");

  if (render_fast)
    render_extra_frame();
}

/* vi: set et ts=2 sw=2: */

