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
#include "d_deh.h"
#include "d_main.h"
#include "d_ticcmd.h"
#include "doomstat.h"
#include "g_game.h"
#include "i_system.h"
#include "lprintf.h"
#include "m_file.h"
#include "p_cmd.h"
#include "p_pspr.h"
#include "p_user.h"
#include "w_wad.h"

#include "n_net.h"
#include "n_main.h"
#include "n_state.h"
#include "n_peer.h"
#include "n_proto.h"

/* CG: FIXME: Most of these should be more than just defines tucked here */
#define MAX_IWAD_NAME_LENGTH 20
#define MAX_RESOURCE_NAMES 1000
#define MAX_RESOURCE_NAME_LENGTH 128
#define MAX_SERVER_MESSAGE_SIZE 256
#define MAX_PLAYER_MESSAGE_SIZE 256
#define MAX_PLAYER_PREFERENCE_NAME_SIZE 32
#define MAX_PLAYER_NAME_SIZE 32
#define MAX_PASSWORD_LENGTH 256
#define MAX_COMMAND_LENGTH 32

#define check_range(x, min, max)                                              \
  if (x < min || x > max) {                                                   \
    P_Printf(consoleplayer,                                                   \
      "%s: Invalid message: %s is out of range (%s, %s)\n",                   \
      __func__, #x, #min, #max                                                \
    );                                                                        \
  }

#define read_char(pbuf, var, name)                                            \
  if (!M_PBufReadChar(pbuf, &var)) {                                          \
    P_Printf(consoleplayer,                                                   \
      "%s: Error reading %s: %s.\n",                                          \
      __func__, name, M_PBufGetError(pbuf)                                    \
    );                                                                        \
    return false;                                                             \
  }

#define read_ranged_char(pbuf, var, name, min, max)                           \
  read_char(pbuf, var, name);                                                 \
  check_range(var, min, max);

#define read_uchar(pbuf, var, name)                                           \
  if (!M_PBufReadUChar(pbuf, &var)) {                                         \
    P_Printf(consoleplayer,                                                   \
      "%s: Error reading %s: %s.\n",                                          \
      __func__, name, M_PBufGetError(pbuf)                                    \
    );                                                                        \
    return false;                                                             \
  }

#define read_ranged_uchar(pbuf, var, name, min, max)                          \
  read_uchar(pbuf, var, name);                                                \
  check_range(var, min, max);

#define read_short(pbuf, var, name)                                           \
  if (!M_PBufReadShort(pbuf, &var)) {                                         \
    P_Printf(consoleplayer,                                                   \
      "%s: Error reading %s: %s.\n",                                          \
      __func__, name, M_PBufGetError(pbuf)                                    \
    );                                                                        \
    return false;                                                             \
  }

#define read_ranged_short(pbuf, var, name, min, max)                          \
  read_short(pbuf, var, name);                                                \
  check_range(var, min, max);

#define read_ushort(pbuf, var, name)                                          \
  if (!M_PBufReadUShort(pbuf, &var)) {                                        \
    P_Printf(consoleplayer,                                                   \
      "%s: Error reading %s: %s.\n",                                          \
      __func__, name, M_PBufGetError(pbuf)                                    \
    );                                                                        \
    return false;                                                             \
  }

#define read_ranged_ushort(pbuf, var, name, min, max)                         \
  read_ushort(pbuf, var, name);                                               \
  check_range(var, min, max);

#define read_int(pbuf, var, name)                                             \
  if (!M_PBufReadInt(pbuf, &var)) {                                           \
    P_Printf(consoleplayer,                                                   \
      "%s: Error reading %s: %s.\n",                                          \
      __func__, name, M_PBufGetError(pbuf)                                    \
    );                                                                        \
    M_PBufPrint(pbuf);                                                        \
    return false;                                                             \
  }

#define read_ranged_int(pbuf, var, name, min, max)                            \
  read_int(pbuf, var, name);                                                  \
  check_range(var, min, max);

#define read_uint(pbuf, var, name)                                            \
  if (!M_PBufReadUInt(pbuf, &var)) {                                          \
    P_Printf(consoleplayer,                                                   \
      "%s: Error reading %s: %s.\n",                                          \
      __func__, name, M_PBufGetError(pbuf)                                    \
    );                                                                        \
    return false;                                                             \
  }

#define read_ranged_uint(pbuf, var, name, min, max)                           \
  read_uint(pbuf, var, name);                                                 \
  check_range(var, min, max);

#define read_long(pbuf, var, name)                                            \
  if (!M_PBufReadLong(pbuf, &var)) {                                          \
    P_Printf(consoleplayer,                                                   \
      "%s: Error reading %s: %s.\n",                                          \
      __func__, name, M_PBufGetError(pbuf)                                    \
    );                                                                        \
    return false;                                                             \
  }

#define read_ranged_long(pbuf, var, name, min, max)                           \
  read_long(pbuf, var, name);                                                 \
  check_range(var, min, max);

#define read_ulong(pbuf, var, name)                                           \
  if (!M_PBufReadULong(pbuf, &var)) {                                         \
    P_Printf(consoleplayer,                                                   \
      "%s: Error reading %s: %s.\n",                                          \
      __func__, name, M_PBufGetError(pbuf)                                    \
    );                                                                        \
    return false;                                                             \
  }

#define read_ranged_ulong(pbuf, var, name, min, max)                          \
  read_ulong(pbuf, var, name);                                                \
  check_range(var, min, max);

#define read_double(pbuf, var, name)                                          \
  if (!M_PBufReadDouble(pbuf, &var)) {                                        \
    P_Printf(consoleplayer,                                                   \
      "%s: Error reading %s: %s.\n",                                          \
      __func__, name, M_PBufGetError(pbuf)                                    \
    );                                                                        \
    return false;                                                             \
  }

#define read_ranged_double(pbuf, var, name, min, max)                         \
  read_double(pbuf, var, name);                                               \
  check_range(var, min, max);

#define read_bool(pbuf, var, name)                                            \
  if (!M_PBufReadBool(pbuf, &var)) {                                          \
    P_Printf(consoleplayer,                                                   \
      "%s: Error reading %s: %s.\n",                                          \
      __func__, name, M_PBufGetError(pbuf)                                    \
    );                                                                        \
    return false;                                                             \
  }

#define read_array(pbuf, var, name)                                           \
  if (!M_PBufReadArray(pbuf, &var)) {                                         \
    P_Printf(consoleplayer,                                                   \
      "%s: Error reading %s: %s.\n",                                          \
      __func__, name, M_PBufGetError(pbuf)                                    \
    );                                                                        \
    return false;                                                             \
  }

#define read_map(pbuf, var, name)                                             \
  if (!M_PBufReadMap(pbuf, &var)) {                                           \
    P_Printf(consoleplayer,                                                   \
      "%s: Error reading %s: %s.\n",                                          \
      __func__, name, M_PBufGetError(pbuf)                                    \
    );                                                                        \
    return false;                                                             \
  }

#define read_bytes(pbuf, var, name)                                           \
  M_BufferClear(&var);                                                        \
  if (!M_PBufReadBytes(pbuf, &var)) {                                         \
    P_Printf(consoleplayer,                                                   \
      "%s: Error reading %s: %s.\n",                                          \
      __func__, name, M_PBufGetError(pbuf)                                    \
    );                                                                        \
    return false;                                                             \
  }

#define read_packed_bytes(pbuf, var, name)                                    \
  M_PBufClear(&var);                                                          \
  if (!M_PBufReadBytes(pbuf, M_PBufGetBuffer(&var))) {                        \
    P_Printf(consoleplayer,                                                   \
      "%s: Error reading %s: %s.\n",                                          \
      __func__, name, M_PBufGetError(pbuf)                                    \
    );                                                                        \
    return false;                                                             \
  }

#define read_string(pbuf, var, name, sz)                                      \
  M_BufferClear(var);                                                         \
  if (!M_PBufReadString(pbuf, var, sz)) {                                     \
    P_Printf(consoleplayer,                                                   \
      "%s: Error reading %s: %s.\n",                                          \
      __func__, name, M_PBufGetError(pbuf)                                    \
    );                                                                        \
    return false;                                                             \
  }

#define read_string_array(pbuf, var, name, count, length)                     \
  if (!M_PBufReadStringArray(pbuf, var, count, length)) {                     \
    P_Printf(consoleplayer,                                                   \
      "%s: Error reading %s: %s.\n",                                          \
      __func__, name, M_PBufGetError(pbuf)                                    \
    );                                                                        \
    return false;                                                             \
  }

#define read_recipient_array(pbuf, var, name, count)                          \
  if (!M_PBufReadShortArray(pbuf, var, count)) {                              \
    P_Printf(consoleplayer,                                                   \
      "%s: Error reading %s: %s.\n",                                          \
      __func__, name, M_PBufGetError(pbuf)                                    \
    );                                                                        \
    return false;                                                             \
  }

#define read_player(pbuf, var)                                                \
  if (!M_PBufReadShort(pbuf, &var)) {                                         \
    P_Printf(consoleplayer,                                                   \
      "%s: Error reading player number: %s.\n",                               \
      __func__, M_PBufGetError(pbuf)                                          \
    );                                                                        \
    return false;                                                             \
  }                                                                           \
  if (var >= MAXPLAYERS) {                                                    \
    P_Printf(consoleplayer,                                                   \
      "%s: Invalid player number %d.\n", __func__, var);                      \
    return false;                                                             \
  }

#define read_message_recipient(pbuf, var)                                     \
  if (!M_PBufReadShort(pbuf, &var)) {                                         \
    P_Printf(consoleplayer,                                                   \
      "%s: Error reading recipient number: %s.\n",                            \
      __func__, M_PBufGetError(pbuf)                                          \
    );                                                                        \
    return false;                                                             \
  }                                                                           \
  if (var != -1 && var >= MAXPLAYERS) {                                       \
    P_Printf(consoleplayer,                                                   \
      "%s: Invalid recipient number %d.\n", __func__, var                     \
    );                                                                        \
    return false;                                                             \
  }

#define pack_player_preference_change(pbuf, gametic, playernum, pn, pnsz)     \
  pbuf_t *pbuf = N_PeerBeginMessage(                                          \
    np->peernum, NET_CHANNEL_RELIABLE, nm_playerpreferencechange              \
  );                                                                          \
  M_PBufWriteInt(pbuf, gametic);                                              \
  M_PBufWriteShort(pbuf, playernum);                                          \
  M_PBufWriteMap(pbuf, 1);                                                    \
  M_PBufWriteString(pbuf, pn, pnsz)

static void pack_commands(pbuf_t *pbuf, netpeer_t *np, short playernum) {
  netticcmd_t *n = NULL;
  byte command_count = 0;
  cbuf_t *commands = NULL;

  M_PBufWriteShort(pbuf, playernum);

  if (DELTACLIENT && playernum == consoleplayer)
    commands = P_GetLocalCommands();
  else
    commands = &players[playernum].commands;

  CBUF_FOR_EACH(commands, entry) {
    netticcmd_t *ncmd = (netticcmd_t *)entry.obj;

    if (ncmd->index > np->sync.cmd)
      command_count++;
  }

  M_PBufWriteUChar(pbuf, command_count);

  if (command_count == 0) {
    D_Log(LOG_SYNC, "[...]\n");
    return;
  }

  CBUF_FOR_EACH(commands, entry) {
    netticcmd_t *ncmd = (netticcmd_t *)entry.obj;

    if (ncmd->index <= np->sync.cmd)
      continue;

    if (n == NULL)
      D_Log(LOG_SYNC, "[%d/%d => ", ncmd->index, ncmd->tic);

    n = ncmd;

    M_PBufWriteInt(pbuf, ncmd->index);
    M_PBufWriteInt(pbuf, ncmd->tic);
    M_PBufWriteChar(pbuf, ncmd->cmd.forwardmove);
    M_PBufWriteChar(pbuf, ncmd->cmd.sidemove);
    M_PBufWriteShort(pbuf, ncmd->cmd.angleturn);
    M_PBufWriteShort(pbuf, ncmd->cmd.consistancy);
    M_PBufWriteUChar(pbuf, ncmd->cmd.chatchar);
    M_PBufWriteUChar(pbuf, ncmd->cmd.buttons);
  }

  if (n != NULL)
    D_Log(LOG_SYNC, "%d/%d]\n", n->index, n->tic);
}

void N_PackSetup(netpeer_t *np) {
  game_state_t *gs = N_GetLatestState();
  unsigned short player_count = 0;
  pbuf_t *pbuf = NULL;
  size_t resource_count = 0;
  size_t deh_count = M_CBufGetObjectCount(&deh_files_buf);
  const char *iwad = D_GetIWAD();

  for (int i = 0; i < MAXPLAYERS; i++) {
    if (playeringame[i]) {
      player_count++;
    }
  }

  pbuf = N_PeerBeginMessage(np->peernum, NET_CHANNEL_RELIABLE, nm_setup);

  M_PBufWriteInt(pbuf, netsync);
  M_PBufWriteUShort(pbuf, player_count);
  M_PBufWriteShort(pbuf, np->playernum);
  M_PBufWriteString(pbuf, iwad, strlen(iwad));

  CBUF_FOR_EACH(&resource_files_buf, entry) {
    wadfile_info_t *wf = (wadfile_info_t *)entry.obj;

    if (wf->src != source_iwad && wf->src != source_auto_load)
      resource_count++;
  }

  M_PBufWriteBool(pbuf, resource_count > 0);
  if (resource_count > 0) {
    M_PBufWriteArray(pbuf, resource_count);

    CBUF_FOR_EACH(&resource_files_buf, entry) {
      wadfile_info_t *wf = (wadfile_info_t *)entry.obj;
      char *wad_name;

      if (wf->src == source_iwad || wf->src == source_auto_load)
        continue;
      
      wad_name = M_Basename(wf->name);

      if (!wad_name)
        I_Error("N_PackSetup: Error getting basename of %s\n", wf->name);

      M_PBufWriteString(pbuf, wad_name, strlen(wad_name));

      free(wad_name);
    }
  }

  M_PBufWriteBool(pbuf, deh_count > 0);
  if (deh_count > 0) {
    M_PBufWriteArray(pbuf, M_CBufGetObjectCount(&deh_files_buf));

    CBUF_FOR_EACH(&deh_files_buf, entry) {
      deh_file_t *df = (deh_file_t *)entry.obj;
      char *deh_name = M_Basename(df->filename);

      if (!deh_name)
        I_Error("N_PackSetup: Error getting basename of %s\n", df->filename);

      M_PBufWriteString(pbuf, deh_name, strlen(deh_name));
      free(deh_name);
    }
  }

  M_PBufWriteInt(pbuf, gs->tic);
  M_PBufWriteBytes(pbuf, M_PBufGetData(&gs->data), M_PBufGetSize(&gs->data));
  np->sync.tic = gs->tic;

  /*
  D_Log(LOG_SYNC, "Resources:");
  OBUF_FOR_EACH(&resource_files_buf, entry) {
    D_Log(LOG_SYNC, "  %s\n", (char *)entry.obj);
  }

  D_Log(LOG_SYNC, "DeH/BEX files:");
  OBUF_FOR_EACH(&deh_files_buf, entry) {
    D_Log(LOG_SYNC, "  %s\n", (char *)entry.obj);
  }

  D_Log(LOG_SYNC, "N_PackSetup: Game State: %d %d %d %d %d %d %zu\n",
    netsync, player_count, np->playernum,
    M_OBufGetObjectCount(&resource_files_buf),
    M_OBufGetObjectCount(&deh_files_buf),
    gs->tic,
    gs->data.size
  );
  */

  D_Log(LOG_SYNC, "N_PackSetup: Sent game state at %d (player count: %d).\n",
    gs->tic, player_count
  );
}

dboolean N_UnpackSetup(netpeer_t *np, net_sync_type_e *sync_type,
                                      unsigned short *player_count,
                                      short *playernum) {
  pbuf_t *pbuf = &np->netcom.incoming.messages;
  int m_sync_type = 0;
  unsigned short m_player_count = 0;
  short m_playernum = 0;
  game_state_t *gs = NULL;
  dboolean has_resources;
  dboolean has_deh_files;
  buf_t iwad_buf;
  obuf_t resource_files;
  obuf_t deh_files;

  read_ranged_int(
    pbuf, m_sync_type, "netsync", NET_SYNC_TYPE_COMMAND, NET_SYNC_TYPE_DELTA
  );
  D_Log(LOG_SYNC, "netsync: %d\n", m_sync_type);
  read_ushort(pbuf, m_player_count, "player count");
  D_Log(LOG_SYNC, "player count: %d\n", m_player_count);
  read_short(pbuf, m_playernum, "consoleplayer");
  D_Log(LOG_SYNC, "consoleplayer: %d\n", m_playernum);

  W_ReleaseAllWads();
  D_ClearIWAD();
  D_ClearResourceFiles();
  D_ClearDEHFiles();

  M_BufferInit(&iwad_buf);
  read_string(pbuf, &iwad_buf, "IWAD", MAX_IWAD_NAME_LENGTH);
  D_SetIWAD(M_BufferGetData(&iwad_buf));
  IdentifyVersion();
  M_BufferFree(&iwad_buf);

  D_AddFile(PACKAGE_TARNAME ".wad", source_auto_load);

  /*
   * CG: TODO: Add missing resources to a list of resources to fetch with
   *           N_GetWad (which should probably be N_GetResource); in the event
   *           fetching is required, disconnect, fetch all the missing
   *           resources, and reconnect (provided all resources were
   *           successfully obtained
   */

  read_bool(pbuf, has_resources, "has resources");
  if (has_resources) {
    M_OBufInit(&resource_files);
    read_string_array(
      pbuf,
      &resource_files,
      "resource names",
      MAX_RESOURCE_NAMES,
      MAX_RESOURCE_NAME_LENGTH
    );
    D_Log(LOG_SYNC,
      "Loaded %d resource files\n", M_OBufGetObjectCount(&resource_files)
    );
    OBUF_FOR_EACH(&resource_files, entry) {
      int resource_index = entry.index;
      char *resource_name = (char *)entry.obj;

      D_AddFile(resource_name, source_net);
      D_Log(LOG_SYNC, " %d: %s\n", resource_index, resource_name);
    }
  }

  read_bool(pbuf, has_deh_files, "has DeH/BEX file");
  if (has_deh_files) {
    M_OBufInit(&deh_files);
    read_string_array(
      pbuf,
      &deh_files,
      "DeH/BEX names",
      MAX_RESOURCE_NAMES,
      MAX_RESOURCE_NAME_LENGTH
    );
    D_Log(LOG_SYNC,
      "Loaded %d DeH/BEX files", M_OBufGetObjectCount(&deh_files)
    );
    OBUF_FOR_EACH(&deh_files, entry) {
      int deh_index = entry.index;
      char *deh_name = (char *)entry.obj;

      D_AddDEH(deh_name, 0);
      D_Log(LOG_SYNC, "DeH/BEX %d: %s\n", deh_index, deh_name);
    }
  }

  //jff 9/3/98 use logical output routine
  lprintf(LO_INFO, "W_Init: Init WADfiles.\n");
  W_Init();
  // killough 3/6/98: add a newline, by popular demand :)
  lprintf(LO_INFO, "\n");

  gs = N_GetNewState();

  read_int(pbuf, gs->tic, "game state tic");
  D_Log(LOG_SYNC, "Game State TIC: %d\n", gs->tic);

  D_Log(LOG_SYNC, "N_UnpackSetup: Game State: %d %d %d %d %d %d\n",
    m_sync_type, m_player_count, m_playernum,
    M_CBufGetObjectCount(&resource_files_buf),
    M_CBufGetObjectCount(&deh_files_buf),
    gs->tic
  );

  read_packed_bytes(pbuf, gs->data, "game state data");

  switch (m_sync_type) {
    case NET_SYNC_TYPE_COMMAND:
      *sync_type = NET_SYNC_TYPE_COMMAND;
    break;
    case NET_SYNC_TYPE_DELTA:
      *sync_type = NET_SYNC_TYPE_DELTA;
    break;
    default:
      P_Printf(consoleplayer, "Invalid sync type %d.\n", m_sync_type);
      return false;
    break;
  }

  *player_count = m_player_count;
  *playernum = m_playernum;

  N_SetLatestState(gs);

  np->sync.tic = gs->tic;

  if (gamestate == GS_INTERMISSION)
    N_LoadLatestState(true);

  return true;
}

void N_PackAuthResponse(netpeer_t *np, auth_level_e auth_level) {
  pbuf_t *pbuf = N_PeerBeginMessage(
    np->peernum, NET_CHANNEL_RELIABLE, nm_authresponse
  );
  printf("Packing auth response\n");

  M_PBufWriteUChar(pbuf, auth_level);
}

dboolean N_UnpackAuthResponse(netpeer_t *np, auth_level_e *auth_level) {
  pbuf_t *pbuf = &np->netcom.incoming.messages;
  unsigned char m_auth_level = 0;

  read_ranged_uchar(
    pbuf, m_auth_level, "auth level", AUTH_LEVEL_NONE, AUTH_LEVEL_MAX - 1
  );

  switch (m_auth_level) {
    case AUTH_LEVEL_NONE:
      *auth_level = AUTH_LEVEL_NONE;
    break;
    case AUTH_LEVEL_SPECTATOR:
      *auth_level = AUTH_LEVEL_SPECTATOR;
    break;
    case AUTH_LEVEL_PLAYER:
      *auth_level = AUTH_LEVEL_PLAYER;
    break;
    case AUTH_LEVEL_MODERATOR:
      *auth_level = AUTH_LEVEL_MODERATOR;
    break;
    case AUTH_LEVEL_ADMINISTRATOR:
      *auth_level = AUTH_LEVEL_ADMINISTRATOR;
    break;
    default:
      P_Printf(consoleplayer, "Invalid auth level type %d.\n", m_auth_level);
      return false;
    break;
  }

  return true;
}

void N_PackServerMessage(netpeer_t *np, const char *message) {
  pbuf_t *pbuf = N_PeerBeginMessage(
    np->peernum, NET_CHANNEL_RELIABLE, nm_servermessage
  );

  printf("Packing server message\n");
  M_PBufWriteString(pbuf, message, strlen(message));
}

dboolean N_UnpackServerMessage(netpeer_t *np, buf_t *buf) {
  pbuf_t *pbuf = &np->netcom.incoming.messages;

  read_string(pbuf, buf, "server message content", MAX_SERVER_MESSAGE_SIZE);

  return true;
}

void N_PackSync(netpeer_t *np) {
  pbuf_t *pbuf = N_PeerBeginMessage(
    np->peernum, NET_CHANNEL_UNRELIABLE, nm_sync
  );
  unsigned short player_count = 0;


  D_Log(LOG_SYNC, "(%d) Sending sync: ST/CT: (%d/%d) ",
    gametic, np->sync.tic, np->sync.cmd
  );

  M_PBufWriteInt(pbuf, np->sync.tic);

  if (SERVER)
    player_count = N_PeerGetCount();
  else
    player_count = 1;

  M_PBufWriteUShort(pbuf, player_count);

  if (SERVER) {
    NETPEER_FOR_EACH(entry) {
      pack_commands(pbuf, np, entry.np->playernum);
    }
  }
  else {
    pack_commands(pbuf, np, consoleplayer);
  }
}

dboolean N_UnpackSync(netpeer_t *np, dboolean *update_sync) {
  pbuf_t *pbuf = &np->netcom.incoming.messages;
  unsigned short m_player_count = 0;
  int m_sync_tic = -1;
  int m_command_index = np->sync.cmd;
  dboolean m_update_sync = false;

  *update_sync = false;

  read_int(pbuf, m_sync_tic, "sync tic");

  if (m_sync_tic <= np->sync.tic)
    return true;

  D_Log(LOG_SYNC, "(%d) Received sync ", gametic);

  if ((np->sync.tic != m_sync_tic))
    m_update_sync = true;

  D_Log(LOG_SYNC, "ST/CT: (%d/%d) ", m_sync_tic, m_command_index);

  read_ushort(pbuf, m_player_count, "player count");

  for (int i = 0; i < m_player_count; i++) {
    short m_playernum = 0;
    byte command_count = 0;
    cbuf_t *commands = NULL;

    read_player(pbuf, m_playernum);

    if (SERVER && np->playernum != m_playernum) {
      D_Log(
        LOG_SYNC, 
        "N_UnpackPlayerCommands: Erroneously received player commands for %d "
        "from player %d\n",
        m_playernum,
        np->playernum
      );
      return false;
    }

  /*
   * CG: TODO: Add a limit to the number of commands accepted here.  uchar
   *           limits this to 255 commands, but in reality that's > 7 seconds,
   *           which is still far too long.  Quake has an "sv_maxlag" setting
   *           (or something); that may be preferable to a static limit... but
   *           I still think having an upper bound on that setting is prudent.
   */
    read_uchar(pbuf, command_count, "command count");

    commands = &players[m_playernum].commands;

    M_CBufEnsureCapacity(commands, command_count);

    netticcmd_t *n = NULL;

    D_Log(LOG_SYNC, "Unpacking %d commands.\n", command_count);

    while (command_count--) {
      int command_index = -1;
      int tic = -1;

      read_int(pbuf, command_index, "command index");
      read_int(pbuf, tic, "command tic");
      
      if (m_command_index == 0)
        m_command_index = command_index - 1;

      if (command_index > m_command_index) {
        netticcmd_t *ncmd = M_CBufGetFirstFreeOrNewSlot(commands);

        m_command_index = command_index;

        ncmd->index = command_index;
        ncmd->tic = tic;

        if (n == NULL)
          D_Log(LOG_SYNC, " [%d/%d => ", ncmd->index, ncmd->tic);

        n = ncmd;

        read_char(pbuf, ncmd->cmd.forwardmove, "command forward value");
        read_char(pbuf, ncmd->cmd.sidemove, "command side value");
        read_short(pbuf, ncmd->cmd.angleturn, "command angle value");
        read_short(pbuf, ncmd->cmd.consistancy, "command consistancy value");
        read_uchar(pbuf, ncmd->cmd.chatchar, "comand chatchar value");
        read_uchar(pbuf, ncmd->cmd.buttons, "command buttons value");
      }
      else {
        ticcmd_t cmd;

        read_char(pbuf, cmd.forwardmove, "command forward value");
        read_char(pbuf, cmd.sidemove, "command side value");
        read_short(pbuf, cmd.angleturn, "command angle value");
        read_short(pbuf, cmd.consistancy, "command consistancy value");
        read_uchar(pbuf, cmd.chatchar, "comand chatchar value");
        read_uchar(pbuf, cmd.buttons, "command buttons value");
      }
    }

    if (n != NULL)
      D_Log(LOG_SYNC, "%d/%d]\n", n->index, n->tic);
    else
      D_Log(LOG_SYNC, "[...]\n");

    if (n != NULL) {
      D_Log(LOG_SYNC, "Commands after sync: ");
      P_PrintPlayerCommands(commands);
    }
  }

  if (np->sync.cmd != m_command_index)
    m_update_sync = true;

  if (m_update_sync) {
    np->sync.tic = m_sync_tic;
    np->sync.cmd = m_command_index;
    *update_sync = m_update_sync;
  }

  return true;
}

void N_PackDeltaSync(netpeer_t *np) {
  pbuf_t *pbuf = N_PeerBeginMessage(
    np->peernum, NET_CHANNEL_UNRELIABLE, nm_sync
  );

  D_Log(LOG_SYNC, "(%d) Sending sync: ST/CT: (%d/%d) Delta: [%d => %d] (%zu)\n",
    gametic,
    np->sync.tic,
    np->sync.cmd,
    np->sync.delta.from_tic,
    np->sync.delta.to_tic,
    np->sync.delta.data.size
  );

  M_PBufWriteInt(pbuf, np->sync.tic);
  M_PBufWriteInt(pbuf, np->sync.cmd);
  M_PBufWriteInt(pbuf, np->sync.delta.from_tic);
  M_PBufWriteInt(pbuf, np->sync.delta.to_tic);
  M_PBufWriteBytes(pbuf, np->sync.delta.data.data, np->sync.delta.data.size);
}

dboolean N_UnpackDeltaSync(netpeer_t *np) {
  pbuf_t *pbuf = &np->netcom.incoming.messages;
  int m_sync_tic = 0;
  int m_command_index = 0;
  int m_delta_from_tic = 0;
  int m_delta_to_tic = 0;

  read_int(pbuf, m_sync_tic, "delta tic");
  read_int(pbuf, m_command_index, "delta command index");
  read_int(pbuf, m_delta_from_tic, "delta from tic");
  read_int(pbuf, m_delta_to_tic, "delta to tic");

  if (m_delta_to_tic <= np->sync.tic)
    return true;

  /*
   * CG: Don't load a delta in the very near future; give ourselves time to
   * catch up
   */
  if ((m_delta_to_tic >= gametic) && (m_delta_to_tic < gametic + 1))
    return true;

  if (m_delta_to_tic > gametic) {
    D_Log(
      LOG_SYNC, 
      "(%d) [!!!] Received future sync! -- Delta: [%d => %d] -- %d, %d.\n",
      gametic,
      m_delta_from_tic,
      m_delta_to_tic,
      m_delta_to_tic - gametic,
      m_delta_to_tic > gametic
    );
  }

  np->sync.tic = m_sync_tic;
  np->sync.cmd = m_command_index;
  np->sync.delta.from_tic = m_delta_from_tic;
  np->sync.delta.to_tic = m_delta_to_tic;
  read_bytes(pbuf, np->sync.delta.data, "delta data");

  D_Log(LOG_SYNC,
    "(%d) Received new sync: ST/CT: (%d/%d) Delta: [%d => %d (%d)] (%zu)\n",
    gametic,
    np->sync.tic,
    np->sync.cmd,
    np->sync.delta.from_tic,
    np->sync.delta.to_tic,
    np->sync.delta.to_tic - np->sync.delta.from_tic,
    np->sync.delta.data.size
  );

  return true;
}

void N_PackRelayedPlayerMessage(netpeer_t *np, short sender, short recipient,
                                               const char *message) {
  pbuf_t *pbuf = N_PeerBeginMessage(
    np->peernum, NET_CHANNEL_RELIABLE, nm_playermessage
  );

  M_PBufWriteUShort(pbuf, sender);
  M_PBufWriteString(pbuf, message, strlen(message));

  printf("Relayed player message\n");
}

void N_PackPlayerMessage(netpeer_t *np, short sender, buf_t *recipients,
                                        const char *message) {
  pbuf_t *pbuf = N_PeerBeginMessage(
    np->peernum, NET_CHANNEL_RELIABLE, nm_playermessage
  );

  M_PBufWriteUShort(pbuf, sender);
  M_PBufWriteString(pbuf, message, strlen(message));
  M_PBufWriteShortArray(pbuf, recipients);

  printf("Packed player message\n");
}

dboolean N_UnpackPlayerMessage(netpeer_t *np, short *sender, buf_t *recipients,
                                              buf_t *buf) {
  pbuf_t *pbuf = &np->netcom.incoming.messages;
  short m_sender = 0;

  read_player(pbuf, m_sender);
  read_string(pbuf, buf, "player message content", MAX_PLAYER_MESSAGE_SIZE);

  if (SERVER) {
    read_recipient_array(
      pbuf, recipients, "message recipients", MAXPLAYERS + 1
    );
  }

  *sender = m_sender;

  return true;
}

dboolean N_UnpackPlayerPreferenceChange(netpeer_t *np, int *tic,
                                                       short *playernum,
                                                       unsigned int *count) {
  pbuf_t *pbuf = &np->netcom.incoming.messages;
  short m_playernum = 0;
  int m_tic = 0;
  unsigned int m_count = 0;

  read_int(pbuf, m_tic, "player preference change tic");
  read_player(pbuf, m_playernum);
  read_map(pbuf, m_count, "player preference change count");

  *tic = m_tic;
  *playernum = m_playernum;
  *count = m_count;

  return true;
}

dboolean N_UnpackPlayerPreferenceName(netpeer_t *np, buf_t *buf) {
  pbuf_t *pbuf = &np->netcom.incoming.messages;
  
  read_string(
    pbuf, buf, "player preference name", MAX_PLAYER_PREFERENCE_NAME_SIZE
  );

  return true;
}

void N_PackNameChange(netpeer_t *np, short playernum, const char *new_name) {
  pack_player_preference_change(pbuf, gametic, playernum, "name", 4);

  M_PBufWriteString(pbuf, new_name, strlen(new_name));

  printf("Packed name change [%s]\n", new_name);
}

dboolean N_UnpackNameChange(netpeer_t *np, buf_t *buf) {
  pbuf_t *pbuf = &np->netcom.incoming.messages;

  read_string(pbuf, buf, "new name", MAX_PLAYER_NAME_SIZE);

  printf("Unpacked name change [%s]\n", M_BufferGetData(buf));

  return true;
}

void N_PackTeamChange(netpeer_t *np, short playernum, byte new_team) {
  pack_player_preference_change(pbuf, gametic, playernum, "team", 4);

  M_PBufWriteUChar(pbuf, new_team);

  printf("Packed team change\n");
}

dboolean N_UnpackTeamChange(netpeer_t *np, byte *new_team) {
  pbuf_t *pbuf = &np->netcom.incoming.messages;
  int team_count = 0;
  byte m_new_team = 0;

  if (team_count > 0) { /* CG: TODO: teams */
    read_ranged_uchar(pbuf, m_new_team, "new team index", 0, team_count - 1);
  }

  *new_team = m_new_team;

  return true;
}

void N_PackPWOChange(netpeer_t *np, short playernum) {
  pack_player_preference_change(pbuf, gametic, playernum, "pwo", 3);

  M_PBufWriteUChar(pbuf, 0); /* CG: TODO */

  printf("Packed PWO change\n");
}

dboolean N_UnpackPWOChange(netpeer_t *np) {
  return false; /* CG: TODO */
}

void N_PackWSOPChange(netpeer_t *np, short playernum, byte new_wsop_flags) {
  pack_player_preference_change(pbuf, gametic, playernum, "wsop", 4);

  M_PBufWriteUChar(pbuf, new_wsop_flags);

  printf("Packed WSOP change\n");
}

dboolean N_UnpackWSOPChange(netpeer_t *np, byte *new_wsop_flags) {
  pbuf_t *pbuf = &np->netcom.incoming.messages;
  byte m_new_wsop_flags = 0;

  read_ranged_uchar(
    pbuf, m_new_wsop_flags, "new WSOP flags", WSOP_NONE, WSOP_MAX - 1
  );

  *new_wsop_flags = m_new_wsop_flags;

  return true;
}

void N_PackBobbingChange(netpeer_t *np, short playernum,
                                        double new_bobbing_amount) {
  pack_player_preference_change(pbuf, gametic, playernum, "bobbing", 7);

  M_PBufWriteDouble(pbuf, new_bobbing_amount);

  printf("Packed bobbing change\n");
}

dboolean N_UnpackBobbingChanged(netpeer_t *np, double *new_bobbing_amount) {
  pbuf_t *pbuf = &np->netcom.incoming.messages;
  double m_new_bobbing_amount = 0;

  read_ranged_double(
    pbuf, m_new_bobbing_amount, "new bobbing amount", 0.0, 1.0
  );

  *new_bobbing_amount = m_new_bobbing_amount;

  return true;
}

void N_PackAutoaimChange(netpeer_t *np, short playernum,
                                        dboolean new_autoaim_enabled) {
  pack_player_preference_change(pbuf, gametic, playernum, "autoaim", 7);

  M_PBufWriteBool(pbuf, new_autoaim_enabled);

  printf("Packed autoaim change\n");
}

dboolean N_UnpackAutoaimChange(netpeer_t *np, dboolean *new_autoaim_enabled) {
  pbuf_t *pbuf = &np->netcom.incoming.messages;
  dboolean m_new_autoaim_enabled = false;

  read_bool(pbuf, m_new_autoaim_enabled, "new autoaim enabled value");

  *new_autoaim_enabled = m_new_autoaim_enabled;

  return true;
}

void N_PackWeaponSpeedChange(netpeer_t *np, short playernum,
                                            byte new_weapon_speed) {
  pack_player_preference_change(pbuf, gametic, playernum, "weapon speed", 12);

  M_PBufWriteUChar(pbuf, new_weapon_speed);

  printf("Packed weapon speed change\n");
}

dboolean N_UnpackWeaponSpeedChange(netpeer_t *np, byte *new_weapon_speed) {
  pbuf_t *pbuf = &np->netcom.incoming.messages;
  byte m_new_weapon_speed = 0;

  read_uchar(pbuf, m_new_weapon_speed, "new weapon speed");

  *new_weapon_speed = m_new_weapon_speed;

  return true;
}

void N_PackColorChange(netpeer_t *np, short playernum, byte new_red,
                                                       byte new_green,
                                                       byte new_blue) {
  pack_player_preference_change(pbuf, gametic, playernum, "color", 5);

  M_PBufWriteUInt(pbuf, (new_red << 24) | (new_green << 16) | (new_blue << 8));

  printf("Packed color change\n");
}

dboolean N_UnpackColorChange(netpeer_t *np, byte *new_red,
                                            byte *new_green,
                                            byte *new_blue) {
  pbuf_t *pbuf = &np->netcom.incoming.messages;
  unsigned int m_new_color = 0;

  read_uint(pbuf, m_new_color, "new color");

  *new_red   = (m_new_color >> 24) & 0xFF;
  *new_green = (m_new_color >> 16) & 0xFF;
  *new_blue  = (m_new_color >>  8) & 0xFF;

  return true;
}

void N_PackColorIndexChange(netpeer_t *np, short playernum,
                                           int new_color_index) {
  pack_player_preference_change(pbuf, gametic, playernum, "color index", 11);

  M_PBufWriteInt(pbuf, new_color_index);

  printf("Packed color index change\n");
}

dboolean N_UnpackColorIndexChange(netpeer_t *np, int *new_color_index) {
  pbuf_t *pbuf = &np->netcom.incoming.messages;
  int m_new_color_index = 0;

  /* CG: TODO: Ensure new color map index is reasonable */
  read_int(pbuf, m_new_color_index, "new color index");

  *new_color_index = m_new_color_index;

  return true;
}

void N_PackSkinChange(netpeer_t *np, short playernum) {
  pack_player_preference_change(pbuf, gametic, playernum, "skin name", 9);

  M_PBufWriteUChar(pbuf, 0); /* CG: TODO */

  printf("Packed skin change\n");
}

dboolean N_UnpackSkinChange(netpeer_t *np, short *playernum) {
  return false; /* CG: TODO */
}

void N_PackAuthRequest(netpeer_t *np, const char *password) {
  pbuf_t *pbuf = N_PeerBeginMessage(
    np->peernum, NET_CHANNEL_RELIABLE, nm_authrequest
  );

  M_PBufWriteString(pbuf, password, strlen(password));

  printf("Packed auth request\n");
}

dboolean N_UnpackAuthRequest(netpeer_t *np, buf_t *buf) {
  pbuf_t *pbuf = &np->netcom.incoming.messages;

  read_string(
    pbuf, buf, "authorization request password", MAX_PASSWORD_LENGTH
  );

  return true;
}

void N_PackRCONCommand(netpeer_t *np, const char *command) {
  pbuf_t *pbuf = N_PeerBeginMessage(
    np->peernum, NET_CHANNEL_RELIABLE, nm_rconcommand
  );

  M_PBufWriteString(pbuf, command, strlen(command));

  printf("Packed RCON command\n");
}

dboolean N_UnpackRCONCommand(netpeer_t *np, buf_t *buf) {
  pbuf_t *pbuf = &np->netcom.incoming.messages;

  read_string(pbuf, buf, "RCON command", MAX_COMMAND_LENGTH);

  return true;
}

void N_PackVoteRequest(netpeer_t *np, const char *command) {
  pbuf_t *pbuf = N_PeerBeginMessage(
    np->peernum, NET_CHANNEL_RELIABLE, nm_voterequest
  );

  M_PBufWriteString(pbuf, command, strlen(command));

  printf("Packed vote request\n");
}

dboolean N_UnpackVoteRequest(netpeer_t *np, buf_t *buf) {
  pbuf_t *pbuf = &np->netcom.incoming.messages;

  read_string(pbuf, buf, "vote command", MAX_COMMAND_LENGTH);

  return true;
}

/* vi: set et ts=2 sw=2: */

