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

static int               remotetic;  /* Tic expected from the remote */
static int               remotesend; /* Tic expected by the remote   */
static ticcmd_t         *localcmds;
static unsigned          numqueuedpackets;
static packet_header_t **queuedpacket;
static int               xtratics = 0;
static dboolean          isExtraDDisplay = false;
static net_state_e       net_state = NET_STATE_NONE;

int      maketic;
int      ticdup = 1;
int      wanted_player_number = 0;
ticcmd_t netcmds[MAXPLAYERS][BACKUPTICS];

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

void CL_SetRemoteTic(int tic) {
  remotetic = tic;
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
  int sendtics;
  size_t pkt_size;

  if (isExtraDDisplay)
    return;

  if (have_peers)
    N_ServiceNetwork();

  // Build new ticcmds
  newtics = I_GetTime() - lastmadetic;

  //e6y    newtics = (newtics > 0 ? newtics : 0);
  lastmadetic += newtics;

  if (ffmap)
    newtics++;

  while (newtics--) {
    I_StartTic();
    if (maketic - gametic > BACKUPTICS / 2)
      break;

    /* Next command expected by the server:  remotesend                     */
    /* Last command acknowledged from the server: remotetic                 */
    /* Current game tic:                     gametic                        */
    /* Number of tics to send to the server: maketic - remotesend           */
    /* Number of tics to make commands for:  maketic - remotesend           */
    
    // e6y
    // Eliminating the sudden jump of six frames(BACKUPTICS/2) 
    // after change of realtic_clock_rate.
    if (maketic - gametic &&
        gametic <= force_singletics_to &&
        realtic_clock_rate < 200) {
      break;
    }

    G_BuildTiccmd(&localcmds[maketic % BACKUPTICS]);
    maketic++;
  }

  if (have_peers && maketic > remotesend) { // Send the tics to the server
    int sendtics;
    remotesend -= xtratics;

    if (remotesend < 0)
      remotesend = 0;

    sendtics = maketic - remotesend;

    {
      size_t pkt_size =
        sizeof(packet_header_t) + 2 + sendtics * sizeof(ticcmd_t);
      packet_header_t *packet = Z_Malloc(pkt_size, PU_STATIC, NULL);

      packet_set(packet, PKT_TICC, maketic - sendtics);
      *(byte *)(packet + 1) = sendtics;
      *(((byte *)(packet + 1)) + 1) = consoleplayer;
      {
        void *tic = ((byte *)(packet + 1)) + 2;
        while (sendtics--) {
          TicToRaw(tic, &localcmds[remotesend++ % BACKUPTICS]);
          tic = (byte *)tic + sizeof(ticcmd_t);
        }
      }
      I_SendPacket(packet, pkt_size);
      Z_Free(packet);
    }
  }
}

/* cph - data passed to this must be in the Doom (little-) endian */
void D_NetSendMisc(netmisctype_t type, size_t len, void* data)
{
  if (have_peers) {
    size_t size = sizeof(packet_header_t) + 3 * sizeof(int) + len;
    packet_header_t *packet = Z_Malloc(size, PU_STATIC, NULL);
    int *p = (void *)(packet + 1);

    packet_set(packet, PKT_EXTRA, gametic);
    *p++ = LittleLong(type);
    *p++ = LittleLong(consoleplayer);
    *p++ = LittleLong(len);
    memcpy(p, data, len);
    I_SendPacket(packet, size);

    Z_Free(packet);
  }
}

static void CheckQueuedPackets(void)
{
  int i;

  for (i = 0; (unsigned)i < numqueuedpackets; i++) {
    if (doom_ntohl(queuedpacket[i]->tic) <= gametic) {
      switch (queuedpacket[i]->type) {
      case PKT_QUIT: // Player quit the game
      {
        int pn = *(byte *)(queuedpacket[i] + 1);
        playeringame[pn] = false;
        doom_printf("Player %d left the game\n", pn);
      }
      break;
      case PKT_EXTRA:
      {
        int *p = (int *)(queuedpacket[i] + 1);
        size_t len = LittleLong(*(p + 2));
        switch (LittleLong(*p)) {
        case nm_plcolour:
          G_ChangedPlayerColour(LittleLong(*(p + 1)), LittleLong(*(p + 3)));
          break;
        case nm_savegamename:
          if (len < SAVEDESCLEN) {
            memcpy(savedescription, p+3, len);
            // Force terminating 0 in case
            savedescription[len] = 0;
          }
          break;
        }
      }
        break;
      default: // Should not be queued
        break;
      }
    }
  }

  { // Requeue remaining packets
    int newnum = 0;
    packet_header_t **newqueue = NULL;

    for (i = 0; (unsigned)i < numqueuedpackets; i++) {
      if (doom_ntohl(queuedpacket[i]->tic) > gametic) {
        newqueue = Z_Realloc(
          newqueue, ++newnum * sizeof *newqueue, PU_STATIC, NULL
        );
        newqueue[newnum-1] = queuedpacket[i];
      }
      else {
        Z_Free(queuedpacket[i]);
      }
    }

    Z_Free(queuedpacket);
    numqueuedpackets = newnum; queuedpacket = newqueue;
  }
}

void TryRunTics (void) {
  int runtics;
  int entertime = I_GetTime();

  // Wait for tics to run
  while (true) {
    NetUpdate();
    runtics = (have_peers ? remotetic : maketic) - gametic;

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

    //if ((displaytime) < (tic_vars.next-SDL_GetTicks()))
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
    if ((!netgame) || (using_p2p_netcode && p2p_state == P2P_STATE_GO)) {
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

