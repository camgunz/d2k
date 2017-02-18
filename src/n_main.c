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

#include <enet/enet.h>

#include "doomdef.h"
#include "doomstat.h"

#include "d_event.h"
#include "c_eci.h"
#include "c_main.h"
#include "d_main.h"
#include "d_msg.h"
#include "g_game.h"
#include "r_defs.h"
#include "i_system.h"
#include "i_input.h"
#include "i_main.h"
#include "i_video.h"
#include "m_argv.h"
#include "m_delta.h"
#include "p_user.h"
#include "m_menu.h"
#include "n_main.h"
#include "p_setup.h"
#include "p_mobj.h"
#include "g_state.h"
#include "cl_cmd.h"
#include "cl_main.h"
#include "cl_net.h"
#include "sv_main.h"
#include "p_checksum.h"
#include "r_fps.h"
#include "s_sound.h"
#include "x_main.h"
#include "e6y.h"
#include "v_video.h"
#include "hu_lib.h"
#include "hu_stuff.h"

#define DEBUG_NET 0
#define DEBUG_SYNC 0
#define DEBUG_SYNC_STDERR 0
#define DEBUG_SAVE 0
#define DEBUG_CMD 0
// #define LOG_SECTOR 43

#define SERVER_NO_PEER_SLEEP_TIMEOUT 20
#define SERVER_SLEEP_TIMEOUT 1
#define MAX_SETUP_REQUEST_ATTEMPTS 10

void G_BuildTiccmd(ticcmd_t *cmd);

static int run_tics(int tic_count) {
  int saved_tic_count = tic_count;

  while (tic_count--) {
    if (MULTINET) {
      if (!D_Wiping()) {
        if (G_GetGameState() == GS_LEVEL) {
          P_BuildCommand();
        }
        else {
          I_InputHandle();
        }
      }
    }
    else {
      memset(&players[consoleplayer].cmd, 0, sizeof(ticcmd_t));
      G_BuildTiccmd(&players[consoleplayer].cmd);
    }
    N_RunTic();
  }

  return saved_tic_count;
}

static int process_tics(int tics_elapsed) {
  if (tics_elapsed <= 0)
    return 0;

  return run_tics(tics_elapsed);
}

static bool should_render(void) {
  if (nodrawers)
    return false;

  if (!window_focused)
    return false;

  if (gametic <= 0)
    return false;

#ifdef GL_DOOM
  if (V_GetMode() == VID_MODEGL)
    return true;
#endif

  if (!movement_smooth)
    return false;

  return true;
}

void N_InitNetGame(void) {
  int i;
  time_t start_request_time;

  netgame   = false;
  solonet   = false;
  netserver = false;

  displayplayer = consoleplayer = 0;
  playeringame[consoleplayer] = true;

  M_InitDeltas();

  for (i = 0; i < MAXPLAYERS; i++) {
    char *name;
    size_t name_length;

    players[i].name = NULL;
    name_length = snprintf(NULL, 0, "Player %d", i);
    name = calloc(name_length + 1, sizeof(char));
    snprintf(name, name_length + 1, "Player %d", i);

    P_SetName(i, name);

    P_InitPlayerMessages(i);
  }

  P_InitCommandQueues();

  if ((i = M_CheckParm("-solo-net"))) {
    netgame = true;
    solonet = true;
  }
  else if ((i = M_CheckParm("-connect"))) {
    char *host = NULL;
    unsigned short port = 0;

    if (i >= myargc) {
      I_Error("-connect requires an address");
    }

    netgame = true;

    N_Init();

    /* [CG] [FIXME] Should these use the ~/.d2k path? */

    if (DEBUG_NET && CLIENT) {
      if (!D_MsgActivateWithPath(MSG_NET, "client-net.log")) {
        I_Error("Error activating client-net.log: %s", strerror(errno));
      }
    }

    if (DEBUG_SAVE && CLIENT) {
      if (!D_MsgActivateWithPath(MSG_SAVE, "client-save.log")) {
        I_Error("Error activating client-save.log: %s", strerror(errno));
      }
    }

    if (DEBUG_SYNC && CLIENT) {
      if (!D_MsgActivateWithPath(MSG_SYNC, "client-sync.log")) {
        I_Error("Error activating client-sync.log: %s", strerror(errno));
      }
    }
#if DEBUG_SYNC_STDERR
    else if (CLIENT) {
      if (!D_MsgActivateWithFile(MSG_SYNC, stderr)) {
        I_Error("Error logging sync to stderr: %s", strerror(errno));
      }
    }
#endif

    if (DEBUG_CMD && CLIENT) {
      if (!D_MsgActivateWithPath(MSG_CMD, "client-cmd.log")) {
        I_Error("Error activating client-cmd.log: %s", strerror(errno));
      }
    }

    CL_Init();

    if (i < (myargc - 1)) {
      N_ParseAddressString(myargv[i + 1], &host, &port);
    }
    else {
      host = strdup("0.0.0.0");
      port = DEFAULT_PORT;
    }

    D_Msg(MSG_INFO,
      "N_InitNetGame: Connecting to server %s:%u...\n", host, port
    );

    for (int i = 0; i < MAX_SETUP_REQUEST_ATTEMPTS; i++) {
      if (!N_Connect(host, port)) {
        I_Error("N_InitNetGame: Connection refused");
      }

      D_Msg(MSG_INFO, "N_InitNetGame: Connected!\n");

      if (!CL_GetServerPeer()) {
        I_Error("N_InitNetGame: Server peer was NULL");
      }

      G_ReloadDefaults();

      D_Msg(MSG_INFO, "N_InitNetGame: Requesting setup information...\n");

      start_request_time = time(NULL);

      while (N_Connected()) {
        CL_SendSetupRequest();
        N_ServiceNetwork();

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
      /*
       * [CG] [FIXME] This should just cancel the connection and print to the
       *              console.
       */
      I_Error("N_InitNetGame: Timed out waiting for setup information");
    }


    D_Msg(MSG_INFO, "N_InitNetGame: Setup information received!\n");
  }
  else {
    if ((i = M_CheckParm("-serve"))) {
      netgame = true;
      netserver = true;
    }

    if (MULTINET) {
      char *host = NULL;
      unsigned short port = DEFAULT_PORT;

      nodrawers   = true;
      nosfxparm   = true;
      nomusicparm = true;

      N_Init();

      if (i < (myargc - 1)) {
        size_t host_length = N_ParseAddressString(myargv[i + 1], &host, &port);

        if (host_length == 0) {
          host = strdup("0.0.0.0");
        }
      }
      else {
        host = strdup("0.0.0.0");
      }

      if (!N_Listen(host, port)) {
        I_Error("Error listning on %s:%d\n", host, port);
      }

      D_Msg(MSG_INFO, "N_InitNetGame: Listening on %s:%u.\n", host, port);

      if (DEBUG_NET && SERVER) {
        if (!D_MsgActivateWithPath(MSG_NET, "server-net.log")) {
          I_Error("Error activating server-net.log: %s", strerror(errno));
        }
      }

      if (DEBUG_SAVE && SERVER) {
        if (!D_MsgActivateWithPath(MSG_SAVE, "server-save.log")) {
          I_Error("Error activating server-save.log: %s", strerror(errno));
        }
      }

      if (DEBUG_SYNC && SERVER) {
        if (!D_MsgActivateWithPath(MSG_SYNC, "server-sync.log")) {
          I_Error("Error activating server-sync.log: %s", strerror(errno));
        }
      }
#if DEBUG_SYNC_STDERR
      else if (SERVER) {
        if (!D_MsgActivateWithFile(MSG_SYNC, stderr)) {
          I_Error("Error logging sync to stderr: %s", strerror(errno));
        }
      }
#endif

      if (DEBUG_CMD && SERVER) {
        if (!D_MsgActivateWithPath(MSG_CMD, "server-cmd.log")) {
          I_Error("Error activating server-cmd.log: %s", strerror(errno));
        }
      }
    }
  }
}

void N_RunTic(void) {
  if (advancedemo) {
    D_DoAdvanceDemo();
  }

  M_Ticker();

  if ((!CL_RePredicting()) && (!CL_Synchronizing())) {
    I_GetTime_SaveMS();
  }

  G_Ticker();

#ifdef LOG_SECTOR
  if ((LOG_SECTOR < numsectors) &&
      (!CL_RePredicting()) &&
      (!CL_Synchronizing()) &&
      (sectors[LOG_SECTOR].floorheight != (168 << FRACBITS)) &&
      (sectors[LOG_SECTOR].floorheight != (40 << FRACBITS))) {
    D_Msg(MSG_SYNC, "(%d) Sector %d: %d/%d\n",
      gametic,
      LOG_SECTOR,
      sectors[LOG_SECTOR].floorheight >> FRACBITS,
      sectors[LOG_SECTOR].ceilingheight >> FRACBITS
    );
  }
#endif

  P_Checksum(gametic);

  if ((G_GetGameState() == GS_LEVEL) && SERVER && (gametic > 0)) {
    NETPEER_FOR_EACH(iter) {
      if (N_PeerSynchronized(iter.np)) {
        N_PeerSyncSetOutdated(iter.np);
      }
    }

    if (N_PeerGetCount() > 0) {
      G_SaveState();
    }

    N_UpdateSync();
  }

  gametic++;
}

void SV_DisconnectLaggedClients(void) {
  if (!SERVER) {
    return;
  }

  if (gamestate != GS_LEVEL) {
    return;
  }

  NETPEER_FOR_EACH(iter) {
    if (!N_PeerSynchronized(iter.np)) {
      continue;
    }

    if (N_PeerTooLagged(iter.np)) {
      D_Msg(MSG_INFO, "(%d) Player %d is too lagged\n",
        gametic,
        N_PeerGetPlayernum(iter.np)
      );
      N_DisconnectPeer(iter.np);
    }
  }
}

void SV_UpdatePings(void) {
  if (!SERVER) {
    return;
  }

  if ((gametic % (TICRATE / 2)) != 0) {
    return;
  }

  NETPEER_FOR_EACH(iter) {
    int playernum;

    if (!N_PeerSynchronized(iter.np)) {
      continue;
    }

    playernum = N_PeerGetPlayernum(iter.np);

    if (playernum == consoleplayer) {
      continue;
    }

    SV_SendPing(playernum);
  }
}

bool N_TryRunTics(void) {
  static int tics_built = 0;

  int tics_elapsed = I_GetTime() - tics_built;
  bool needs_rendering = should_render();

  if ((gametic > 0) &&
      (((tics_elapsed <= 0) && (!needs_rendering)) ||
       (SERVER && N_PeerGetCount() == 0))) {
    N_ServiceNetwork();
    C_ECIService();
    I_Sleep(1);
    return false;
  }

  if (tics_elapsed > 0) {
    tics_built += tics_elapsed;

    if (ffmap) {
      tics_elapsed++;
    }

    CL_CheckForStateUpdates();

    process_tics(tics_elapsed);

    SV_UpdatePings();
    SV_CleanupOldCommandsAndStates();
    SV_DisconnectLaggedClients();

    if (CLIENT && gametic > 0 && CL_GetServerPeer() == NULL) {
      P_Echo(consoleplayer, "Server disconnected.");
      N_Disconnect();
    }
  }

  N_ServiceNetwork();
  C_ECIService();

  if ((!SERVER) && (!nodrawers)) {
    if (!X_Call(X_GetState(), "console", "tick", 0, 0)) {
      I_Error("Error ticking console: %s\n", X_GetError(X_GetState()));
    }

    HU_Ticker();
    D_Display();
  }

  X_RunGC(X_GetState());

  return tics_elapsed > 0;
}

/* vi: set et ts=2 sw=2: */

