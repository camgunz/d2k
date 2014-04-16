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
#include <msgpack.h>

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
#include "m_cbuf.h"
#include "m_menu.h"
#include "m_utf.h"
#include "p_checksum.h"
#include "r_fps.h"
#include "e6y.h"

#include "n_net.h"
#include "n_main.h"
#include "n_state.h"
#include "n_peer.h"
#include "n_proto.h"

/*
 * CG: TODO:
 *   - Add WAD fetching (waiting on libcurl)
 */

static dboolean is_extra_ddisplay = false;

/* CG: Client only */
static dboolean received_setup = false;
static auth_level_e authorization_level = AUTH_LEVEL_NONE;

void N_InitNetGame(void) {
  int i;

  netgame   = false;
  solonet   = false;
  netserver = false;
  netsync   = NET_SYNC_TYPE_NONE;

  displayplayer = consoleplayer = 0;
  playeringame[consoleplayer] = true;

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

      if (N_GetPeer(0) == NULL)
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

  if (!CLIENT) {
    M_CBufInitWithCapacity(
      &players[consoleplayer].commands, sizeof(netticcmd_t), BACKUPTICS
    );
  }
}

dboolean N_GetWad(const char *name) {
  return false;
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

void CL_RemoveOldStates(void) {
  netpeer_t *server = N_GetPeer(0);

  if (server != NULL)
    N_RemoveOldStates(server->delta.from_tic);
}

void SV_RemoveOldStates(void) {
  int command_tic = INT_MAX;

  for (int i = 0; i < N_GetPeerCount(); i++) {
    netpeer_t *np = N_GetPeer(i);

    if (np != NULL)
      command_tic = MIN(command_tic, np->command_tic);
  }

  N_RemoveOldStates(command_tic);
}

void N_TryRunTics(void) {
  static int commands_last_built_time = 0;

  int sleep_time = ms_to_next_tick;
  int current_time = I_GetTime();
  int delta_time = current_time - commands_last_built_time;
  int commands_to_build = delta_time;
  int menu_renderer_calls = commands_to_build * 3;
  int tics_to_run = INT_MAX;

  if (commands_to_build > 0)
    commands_last_built_time = current_time;

  if (ffmap)
    commands_to_build++;

  if (MULTINET && CMDSYNC) {
    for (int i = 0; i < MAXPLAYERS; i++) {
      if (playeringame[i]) {
        int command_count = M_CBufGetObjectCount(&players[i].commands);

        tics_to_run = MIN(tics_to_run, command_count);
      }
    }
  }
  else {
    tics_to_run = commands_to_build;
  }

  if (menu_renderer_calls > tics_to_run)
    menu_renderer_calls -= tics_to_run;
  else
    menu_renderer_calls = 0;

  if (commands_to_build > 0) {
    while (commands_to_build--) {
      I_StartTic();
      M_CBufConsolidate(&players[consoleplayer].commands);
      G_BuildTiccmd(
        M_CBufGetFirstFreeOrNewSlot(&players[consoleplayer].commands)
      );
      N_Update();
    }
  }

  if (tics_to_run > 0) {
    while (tics_to_run--) {
      if (advancedemo)
        D_DoAdvanceDemo();

      M_Ticker();
      I_GetTime_SaveMS();
      G_Ticker();
      P_Checksum(gametic);
      gametic++;

      if (DELTASERVER) {
        printf("Saving state at %d.\n", gametic);
        N_SaveState();
      }
    }

    if (CMDSERVER) {
      for (int i = 0; i < N_GetPeerCount(); i++) {
        netpeer_t *np = N_GetPeer(i);

        if (np != NULL)
          np->command_tic = gametic;
      }
    }
  }

  if (SERVER) {
    for (int i = 0; i < N_GetPeerCount(); i++) {
      netpeer_t *np = N_GetPeer(i);

      if (np != NULL && np->needs_setup != 0 && gametic > np->needs_setup)
        SV_SendSetup(np->playernum);
    }
  }

  if (menu_renderer_calls > 0) {
    while (menu_renderer_calls--) {
      M_Ticker();
    }
  }

  if (MULTINET) {
    N_ServiceNetwork();

    if (N_GetPeerCount() == 0) {
      I_uSleep(sleep_time * 1000);
      N_RemoveOldStates(gametic);
    }
  }
  else {
    if (movement_smooth && window_focused)
      sleep_time = 0;

#ifdef GL_DOOM
    if (V_GetMode() == VID_MODEGL)
      sleep_time = 0;
#endif

    if (sleep_time > 0)
      I_uSleep(sleep_time * 1000);
  }

#ifdef GL_DOOM
  if (V_GetMode() == VID_MODEGL) {
    WasRenderedInTryRunTics = true;
    is_extra_ddisplay = true;
    D_Display();
    is_extra_ddisplay = false;
  }
#else
  if (gametic > 0) {
    WasRenderedInTryRunTics = true;
    if (movement_smooth && gamestate == wipegamestate) {
      is_extra_ddisplay = true;
      D_Display();
      is_extra_ddisplay = false;
    }
  }
#endif
}

/* vi: set et ts=2 sw=2: */

