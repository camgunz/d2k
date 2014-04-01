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
 *    Network client. Passes information to/from server, staying
 *    synchronised.
 *    Contains the main wait loop, waiting for network input or
 *    time before doing the next tic.
 *    Rewritten for LxDoom, but based around bits of the old code.
 *
 *-----------------------------------------------------------------------------
 */

#include "z_zone.h"

#include "doomtype.h"
#include "doomstat.h"
#include "d_net.h"

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

static dboolean          isExtraDDisplay = false;
static net_state_e       net_state = NET_STATE_NONE;

int      maketic;
int      wanted_player_number = 0;

/*
static int               xtratics = 0;
int      ticdup = 1;
ticcmd_t netcmds[MAXPLAYERS][BACKUPTICS];
*/

void N_SetP2PState(p2p_state_e new_state) {
  if (new_state > p2p_state)
    p2p_state = new_state;
}

void N_InitNetGame(void) {
  int i = M_CheckParm("-net");

  if (i && i < myargc - 1)
    i++;

  if (!(netgame = have_peers = !!i)) {
    playeringame[consoleplayer = 0] = true;
    netgame = M_CheckParm("-solo-net");
  }

  if (have_peers) {
    N_Init();

    if (!N_ConnectToServer(myargv[i]))
      I_Error("Server aborted the game");

    N_ServiceNetworkTimeout(CONNECT_TIMEOUT * 1000);

    if (p2p_state != P2P_STATE_SETUP) 
      I_Error("Timed out waiting for setup information from server");

    atexit(N_Disconnect());
  }

  displayplayer = consoleplayer;
  if (!playeringame(consoleplayer))
    I_Error("D_InitNetGame: consoleplayer not in game");
}

void N_WaitForServer(void) {
  /*
   * CG: This should basically not exist.  In D_DoomLoop or whatever, the
   *     client needs to check its state, if it's SETUP and then it gets the
   *     GO packet, then it loads the game, otherwise it returns.
   */
  packet_header_t *packet = Z_Malloc(
    sizeof(packet_header_t) + 1, PU_STATIC, NULL
  );

  if (have_peers) {
    lprintf(LO_INFO,
      "D_CheckNetGame: waiting for server to signal game start\n"
    );
    do {
      while (!I_GetPacket(packet, sizeof(packet_header_t) + 1)) {
        packet_set(packet, PKT_GO, 0);
        *(byte *)(packet + 1) = consoleplayer;
        I_SendPacket(packet, sizeof(packet_header_t) + 1);
        I_uSleep(100000);
      }
    } while (packet->type != PKT_GO);
  }
  Z_Free(packet);
}

dboolean D_NetGetWad(const char* name) {
  /* CG: TODO: Do this when libcurl is added */
  return false;
}

/*
 * CG: TODO:
 *   - Fix the input system to avoid mouse deceleration; it almost
 *     certainly is a problem.
 *   - Fix choppiness between frames; rendering is probably only done at 35Hz
 *     if interpolation is disabled, which won't work for OpenGL.
 */

void NetUpdate(void) {
  static int lastmadetic;

  int newtics;
  ticcmd_t cmd;
  netticcmd_t ncmd;

  if (isExtraDDisplay)
    return;

  if (have_peers)
    N_ServiceNetwork();

  newtics = I_GetTime() - lastmadetic;

  //e6y    newtics = (newtics > 0 ? newtics : 0);
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
    M_CBufAppend(&D_ConsolePlayer()->commands, &ncmd);
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
      OBUF_FOR_EACH(&players, index, player_t *, player) {
        int command_count = M_CBufGetObjectCount(&player->commands);

        if (runtics == -1 || runtics < command_count)
          runtics = command_count;
      }
    }

    if (runtics)
      break;

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
        isExtraDDisplay = true;
        D_Display();
        isExtraDDisplay = false;
      }
    }
  }

  while (runtics--) {
    if (have_peers)
      CheckQueuedPackets();

    if (advancedemo)
      D_DoAdvanceDemo();

    M_Ticker();
    I_GetTime_SaveMS();
    if ((!netgame) || (have_peers && net_state = NET_STATE_GO)) {
      G_Ticker();
      P_Checksum(gametic);
      gametic++;
    }

    NetUpdate(); // Keep sending our tics to avoid stalling remote nodes
  }
}

static void D_QuitNetGame (void)
{
  byte buf[1 + sizeof(packet_header_t)];
  packet_header_t *packet = (void *)buf;
  int i;

  if (!have_peers)
    return;

  buf[sizeof(packet_header_t)] = consoleplayer;
  packet_set(packet, PKT_QUIT, gametic);

  for (i = 0; i < 4; i++) {
    I_SendPacket(packet, 1 + sizeof(packet_header_t));
    I_uSleep(10000);
  }
}

/* vi: set et ts=2 sw=2: */

