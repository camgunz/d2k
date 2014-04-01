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

#include "doomtype.h"
#include "doomstat.h"

#include "d_main.h"
#include "g_game.h"
#include "m_menu.h"
#include "m_utf.h"
#include "p_checksum.h"

#include "i_protocol.h"
#include "i_network.h"
#include "i_system.h"
#include "i_main.h"
#include "i_video.h"
#include "m_argv.h"
#include "r_fps.h"
#include "lprintf.h"
#include "e6y.h"

#include "n_net.h"
#include "n_main.h"

/*
 * CG: TODO:
 *   - Add WAD fetching (waiting on libcurl)
 */

static int maketic = 0;
static int lastmadetic = 0;
static dboolean is_extra_ddisplay = false;

/* CG: Client only */
static dboolean received_setup = false;
static auth_level_e authorization_level = AUTH_LEVEL_NONE;

void N_InitNetGame(void) {
  netgame   = false;
  solonet   = false;
  netserver = false;
  netsync   = NET_SYNC_TYPE_NONE:

  displayplayer = consoleplayer = 0;
  playeringame[consoleplayer] = true; // CG: FIXME

  if (M_CheckParm("-solo-net")) {
    netgame = true;
    solonet = true;
  }
  else if (M_CheckParm("-net")) {
    if (i < myargc - 1) {
      netgame = true;

      N_Init();

      if (!N_ConnectToServer(myargv[i + 1]))
        I_Error("Connection aborted");

      N_ServiceNetworkTimeout(CONNECT_TIMEOUT * 1000);

      if (!received_setup)
        I_Error("Timed out waiting for setup information from the server");

      atexit(N_Disconnect());
    }
  }
  else {
    if (M_CheckParm("-commandserve") && M_CheckParm("-deltaserve"))
      I_Error("Cannot specify both '-commandserve' and '-deltaserve'");
    else if (M_CheckParm("-commandserve"))
      netsync = NET_SYNC_TYPE_COMMAND;
    else if (M_CheckParm("-deltaserve"))
      netsync = NET_SYNC_TYPE_DELTA;

    if (CMDSYNC || DELTASYNC) {
      char *host = NULL;
      unsigned short port = DEFAULT_PORT;

      netgame = true;
      netserver = true;
      nodrawers = true;

      N_Init();

      if (i < myargc - 1) {
        size_t host_length = N_ParseAddressString(myargv[i + 1], &host, &port);

        if (host_length == 0)
          host = strdup("0.0.0.0");
      }

      if (!N_Listen(host, port))
        I_Error("Startup aborted");

      atexit(N_Disconnect());
    }
  }

}

dboolean N_GetWad(const char *name) {
  return false;
}

void N_Update(void) {
  int newtics;
  ticcmd_t cmd;
  netticcmd_t ncmd;

  if (is_extra_ddisplay)
    return;

  if (have_peers)
    N_ServiceNetwork();

  newtics = I_GetTime() - lastmadetic;

  lastmadetic += newtics;

  if (ffmap)
    newtics++;

  while (newtics--) {
    I_StartTic();
    if (maketic - gametic > BACKUPTICS / 2)
      break;

    // e6y
    // Eliminating the sudden jump of six frames(BACKUPTICS/2) 
    // after change of realtic_clock_rate.
    if (maketic - gametic &&
        gametic <= force_singletics_to &&
        realtic_clock_rate < 200) {
      break;
    }

    G_BuildTiccmd(&cmd);
    ncmd.tic         = maketic;
    ncmd.forwardmove = cmd.forwardmove
    ncmd.sidemove    = cmd.sidemove
    ncmd.angleturn   = cmd.angleturn
    ncmd.consistancy = cmd.consistancy
    ncmd.chatchar    = cmd.chatchar
    ncmd.buttons     = cmd.buttons
    M_CBufAppend(M_OBufGet(players, consoleplayer), &ncmd);
    maketic++;
  }
}

void TryRunTics (void) {
  int index = -1;
  player_t *player = NULL;
  int runtics = -1;
  int entertime = I_GetTime();

  while (true) {
    N_Update();

    if (have_peers) {
      while (M_OBufIter(players, &index, &player)) {
        int command_count = M_CBufGetObjectCount(&player->commands);

        if (runtics == -1 || runtics < command_count)
          runtics = command_count;
      }
    }
    else {
      runtics = maketic - gametic;
    }

    if (runtics) {
      while (runtics--) {
        if (advancedemo)
          D_DoAdvanceDemo();

        M_Ticker();
        I_GetTime_SaveMS();
        G_Ticker();
        P_Checksum(gametic);
        gametic++;

        N_Update(); // Keep sending our tics to avoid stalling remote nodes
      }
      break;
    }

    if (!movement_smooth || !window_focused) {
      if (have_peers)
        N_ServiceNetworkTimeout(ms_to_next_tick);
      else
        I_uSleep(ms_to_next_tick * 1000);
    }

    if (I_GetTime() - entertime > 10) {
      M_Ticker();
      return;
    }

    if (gametic > 0) {
      WasRenderedInTryRunTics = true;
      if (movement_smooth && gamestate == wipegamestate) {
        is_extra_ddisplay = true;
        D_Display();
        is_extra_ddisplay = false;
      }
    }
  }
}

void CL_SetReceivedSetup(dboolean new_received_setup) {
  received_setup = new_received_setup;
}

void CL_SetAuthorizationLevel(auth_level_e level) {
  if (level > authorization_level)
    authorization_level = level;
}

void CL_RemoveOldCommands(int tic) {
  int index = -1;
  netticcmd_t *ncmd = NULL;
  player_t *p = M_OBufGet(players, consoleplayer);

  while (M_CBufIter(&p->commands, &index, (void **)&ncmd)) {
    if (ncmd->tic <= tic) {
      M_CBufRemove(&p->commands, index);
      index--;
    }
  }
}

/* vi: set et ts=2 sw=2: */

