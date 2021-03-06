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
#include "d_deh.h"
#include "d_event.h"
#include "d_main.h"
#include "g_game.h"
#include "g_state.h"
#include "i_system.h"
#include "m_bitmap.h"
#include "m_file.h"
#include "n_main.h"
#include "p_user.h"
#include "cl_cmd.h"
#include "cl_main.h"
#include "cl_net.h"
#include "p_user.h"
#include "w_wad.h"

/* CG: FIXME: Most of these should be more than just defines tucked here */
#define MAX_IWAD_NAME_LENGTH 20
#define MAX_RESOURCE_NAMES 1000
#define MAX_RESOURCE_NAME_LENGTH 128
#define MAX_PLAYER_PREFERENCE_NAME_SIZE 32
#define MAX_PLAYER_NAME_SIZE 32
#define MAX_PASSWORD_LENGTH 256
#define MAX_COMMAND_LENGTH 32
#define MAX_CHAT_MESSAGE_SIZE 256

#define check_range(x, min, max)                                              \
  if ((x < min) || (x > max)) {                                               \
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

#define read_packed_game_state(pbuf, var, tic, name)                          \
  var = G_ReadNewStateFromPackedBuffer(tic, pbuf);                            \
  if (var == NULL) {                                                          \
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

#define read_player(pbuf, var)                                                \
  if (!M_PBufReadUShort(pbuf, &var)) {                                        \
    P_Printf(consoleplayer,                                                   \
      "%s: Error reading player number: %s.\n",                               \
      __func__, M_PBufGetError(pbuf)                                          \
    );                                                                        \
    return false;                                                             \
  }                                                                           \
  if (var >= MAXPLAYERS) {                                                    \
    P_Printf(consoleplayer,                                                   \
      "%s: Invalid player number %d.\n",                                      \
      __func__, var                                                           \
    );                                                                        \
    return false;                                                             \
  }

#define read_bitmap(pbuf, var, size, name)                                    \
  if (!M_PBufReadBitmap(pbuf, (char *)var, size)) {                           \
    P_Printf(consoleplayer,                                                   \
      "%s: Error reading %s: %s.\n",                                          \
      __func__, name, M_PBufGetError(pbuf)                                    \
    );                                                                        \
    return false;                                                             \
  }

#define pack_player_preference_change(pbuf, gametic, playernum, pn, pnsz)     \
  pbuf_t *pbuf = N_PeerBeginMessage(np, NM_PLAYER_PREFERENCE_CHANGE);         \
  M_PBufWriteInt(pbuf, gametic);                                              \
  M_PBufWriteUShort(pbuf, playernum);                                         \
  M_PBufWriteMap(pbuf, 1);                                                    \
  M_PBufWriteString(pbuf, pn, pnsz)

#define unpack_net_command(pbuf, m_ncmd)                                      \
  read_uint(pbuf,  m_ncmd.index,      "command index");                       \
  read_uint(pbuf,  m_ncmd.tic,        "command TIC");                         \
  read_uint(pbuf,  m_ncmd.server_tic, "server command TIC");                  \
  read_char(pbuf,  m_ncmd.forward,    "command forward value");               \
  read_char(pbuf,  m_ncmd.side,       "command side value");                  \
  read_short(pbuf, m_ncmd.angle,      "command angle value");                 \
  read_uchar(pbuf, m_ncmd.buttons,    "command buttons value")

static void free_string(gpointer data) {
  free(data);
}

static void pack_run_commands(pbuf_t *pbuf, unsigned int playernum) {
  GPtrArray *commands = players[playernum].cmdq.commands;
  unsigned int command_count = 0;
  unsigned int total_command_count = 0;

  for (unsigned int i = 0; i < commands->len; i++) {
    netticcmd_t *ncmd = g_ptr_array_index(commands, i);

    total_command_count++;

    if (ncmd->server_tic != 0) {
      command_count++;
    }
  }

  D_Msg(MSG_SYNC, "(%d) (%d) Synced/Total: %u/%u\n",
    gametic,
    playernum,
    command_count,
    total_command_count
  );

  M_PBufWriteUNum(pbuf, command_count);

  if (command_count > 0) {
    D_Msg(MSG_SYNC, "(%d) (%d) Ran commands: [", gametic, playernum);
  }

  for (unsigned int i = 0; i < commands->len; i++) {
    netticcmd_t *ncmd = g_ptr_array_index(commands, i);

    if (ncmd->server_tic != 0) {
      D_Msg(MSG_SYNC, " %d/%d/%d", ncmd->index, ncmd->tic, ncmd->server_tic);
      M_PBufWriteUNum(pbuf, ncmd->index);
      M_PBufWriteUNum(pbuf, ncmd->server_tic);
    }
  }

  if (command_count > 0) {
    D_Msg(MSG_SYNC, " ]\n");
  }
}

static void pack_unsynchronized_commands(pbuf_t *pbuf, netpeer_t *np,
                                                       int src_playernum) {
  GPtrArray *commands = players[src_playernum].cmdq.commands;
  unsigned int command_count = 0;

  for (unsigned int i = 0; i < commands->len; i++) {
    netticcmd_t *ncmd = (netticcmd_t *)g_ptr_array_index(commands, i);

    if (ncmd->index > N_PeerGetSyncCommandIndexForPlayer(np, src_playernum)) {
      command_count++;
    }
    else {
      D_Msg(MSG_SYNC, "(%d) (%u->%u) Skipping command (%u <= %u)\n",
        gametic,
        src_playernum,
        N_PeerGetPlayernum(np),
        ncmd->index,
        N_PeerGetSyncCommandIndexForPlayer(np, src_playernum)
      );
    }
  }

  M_PBufWriteUNum(pbuf, command_count);

  if (command_count > 0) {
    D_Msg(MSG_SYNC, "(%d) (%u->%u) Unsync'd commands: [",
      gametic,
      src_playernum,
      N_PeerGetPlayernum(np)
    );
  }

  for (size_t i = 0; i < commands->len; i++) {
    netticcmd_t *ncmd = (netticcmd_t *)g_ptr_array_index(commands, i);

    if (ncmd->index > N_PeerGetSyncCommandIndexForPlayer(np, src_playernum)) {
      D_Msg(MSG_SYNC, " %u/%u/%u", ncmd->index, ncmd->tic, ncmd->server_tic);
      M_PBufWriteUNum(pbuf, ncmd->index);
      M_PBufWriteUNum(pbuf, ncmd->tic);
      M_PBufWriteUNum(pbuf, ncmd->server_tic);
      M_PBufWriteNum(pbuf, ncmd->forward);
      M_PBufWriteNum(pbuf, ncmd->side);
      M_PBufWriteNum(pbuf, ncmd->angle);
      M_PBufWriteUNum(pbuf, ncmd->buttons);
    }
  }

  if (command_count > 0) {
    D_Msg(MSG_SYNC, " ]\n");
  }
}

void N_PackSetupRequest(netpeer_t *np) {
  pbuf_t *pbuf = N_PeerBeginMessage(np, NM_SETUP);

  M_PBufWriteUChar(pbuf, 0);
}

void N_PackSetup(netpeer_t *np) {
  game_state_t *gs = G_GetLatestState();
  pbuf_t *pbuf = NULL;
  size_t resource_count = 0;
  size_t deh_count = deh_files->len;
  const char *iwad = D_GetIWAD();
  def_bitmap_zero(bitmap, MAXPLAYERS);

  pbuf = N_PeerBeginMessage(np, NM_SETUP);

  M_PBufWriteNum(pbuf, deathmatch);
  M_PBufWriteUNum(pbuf, N_PeerGetPlayernum(np));

  /*
   * We will support 2048 connections, which is a maximum of 2048 players.  I
   * could do (and have done) math here to figure out where the optimal
   * crossover point between indices and a bitmap is, but the fact is that
   * putting a 2048 short array in a struct is 4k, and that tends to blow
   * stacks.  So bitmap it (always) is, then.
   */

  for (size_t i = 0; i < MAXPLAYERS; i++) {
    if (playeringame[i]) {
      bitmap_set_bit(bitmap, i);
    }
  }

  M_PBufWriteBitmap(pbuf, (char *)bitmap, sizeof(bitmap));

  M_PBufWriteString(pbuf, iwad, strlen(iwad));

  for (unsigned int i = 0; i < resource_files->len; i++) {
    wadfile_info_t *wf = g_ptr_array_index(resource_files, i);

    if (wf->src != source_iwad && wf->src != source_auto_load)
      resource_count++;
  }

  M_PBufWriteBool(pbuf, resource_count > 0);
  if (resource_count > 0) {
    M_PBufWriteArray(pbuf, resource_count);

    for (unsigned int i = 0; i < resource_files->len; i++) {
      wadfile_info_t *wf = g_ptr_array_index(resource_files, i);
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

  M_PBufWriteUInt(pbuf, deh_count);
  if (deh_count > 0) {
    for (unsigned int i = 0; i < deh_files->len; i++) {
      deh_file_t *df = g_ptr_array_index(deh_files, i);
      char *deh_name;

      if (df->lumpnum) {
        M_PBufWriteBool(pbuf, true);
        M_PBufWriteInt(pbuf, df->lumpnum);
        continue;
      }

      deh_name = M_Basename(df->filename);

      if (!deh_name)
        I_Error("N_PackSetup: Error getting basename of %s\n", df->filename);

      M_PBufWriteString(pbuf, deh_name, strlen(deh_name));
      free(deh_name);
    }
  }

  M_PBufWriteInt(pbuf, gs->tic);
  M_PBufWriteBytes(pbuf, M_PBufGetData(gs->data), M_PBufGetSize(gs->data));
  N_PeerUpdateSyncTIC(np, gs->tic);
}

bool N_UnpackSetup(netpeer_t *np, unsigned short *playernum) {
  pbuf_t *pbuf = N_PeerGetIncomingMessageData(np);
  int m_deathmatch = 0;
  unsigned int m_playernum = 0;
  def_bitmap_zero(bitmap, MAXPLAYERS);
  int m_state_tic;
  game_state_t *gs;
  bool has_resources;
  unsigned int deh_file_count;
  buf_t iwad_buf;
  char *iwad_name;
  char *iwad_path;
  GPtrArray *rf_list;

  for (size_t i = 0; i < MAXPLAYERS; i++) {
    playeringame[i] = false;
  }

  read_ranged_int(pbuf, m_deathmatch, "deathmatch", 0, 2);
  read_uint(pbuf, m_playernum, "consoleplayer");

  read_bitmap(pbuf, bitmap, MAXPLAYERS, "playeringame bitmap");

  for (size_t i = 0; i < MAXPLAYERS; i++) {
    if (bitmap_get_bit(bitmap, i)) {
      playeringame[i] = true;
    }
  }

  M_BufferInit(&iwad_buf);
  read_string(pbuf, &iwad_buf, "IWAD", MAX_IWAD_NAME_LENGTH);
  iwad_name = M_StripExtension(M_BufferGetData(&iwad_buf));

  M_BufferFree(&iwad_buf);

  iwad_path = I_FindFile(iwad_name, ".wad");

  if (!iwad_path) {
    D_Msg(MSG_WARN, "IWAD %s not found\n", iwad_name);
    free(iwad_name);
    return false;
  }

  free(iwad_name);

  W_ReleaseAllWads();
  D_ClearIWAD();
  D_ClearResourceFiles();
  D_ClearDEHFiles();

  AddIWAD(iwad_path);
  D_SetIWAD(iwad_path);
  free(iwad_path);

  IdentifyVersion();

  /*
   * CG: TODO: Add missing resources to a list of resources to fetch with
   *           N_GetWad (which should probably be N_GetResource); in the event
   *           fetching is required, disconnect, fetch all the missing
   *           resources, and reconnect (provided all resources were
   *           successfully obtained
   */

  read_bool(pbuf, has_resources, "has resources");

  if (has_resources) {
    rf_list = g_ptr_array_new_with_free_func(free_string);
    read_string_array(
      pbuf,
      rf_list,
      "resource names",
      MAX_RESOURCE_NAMES,
      MAX_RESOURCE_NAME_LENGTH
    );
    for (unsigned int i = 0; i < rf_list->len; i++) {
      char *resource_name = g_ptr_array_index(rf_list, i);

      W_AddResource(resource_name, source_net);
    }
    g_ptr_array_free(rf_list, true);
  }

  W_AddResource(PACKAGE_TARNAME ".wad", source_auto_load);

  read_uint(pbuf, deh_file_count, "DeH/BEX file count");

  if (deh_file_count) {
    buf_t deh_name;
    int deh_lumpnum;
    bool is_lump;

    M_BufferInitWithCapacity(&deh_name, MAX_RESOURCE_NAME_LENGTH);

    for (unsigned int i = 0; i < deh_file_count; i++) {
      read_bool(pbuf, is_lump, "DeH/BEX is lump");

      if (is_lump) {
        read_int(pbuf, deh_lumpnum, "DeH/BEX lumpnum");
        W_AddDEH(NULL, deh_lumpnum);
      }
      else {
        read_string(pbuf, &deh_name, "DeH/BEX name", MAX_RESOURCE_NAME_LENGTH);
        W_AddDEH(M_BufferGetData(&deh_name), deh_lumpnum);
      }
    }
  }

  read_int(pbuf, m_state_tic, "game state tic");
  read_packed_game_state(pbuf, gs, m_state_tic, "game state data");

  deathmatch = m_deathmatch;
  *playernum = m_playernum;

  G_SetLatestState(gs);

  N_PeerUpdateSyncTIC(np, gs->tic);

  return true;
}

void N_PackFullState(netpeer_t *np) {
  game_state_t *gs = G_GetLatestState();
  pbuf_t *pbuf = N_PeerBeginMessage(np, NM_FULL_STATE);

  M_PBufWriteInt(pbuf, gs->tic);
  M_PBufWriteBytes(pbuf, M_PBufGetData(gs->data), M_PBufGetSize(gs->data));

  N_PeerUpdateSyncTIC(np, gs->tic);
}

bool N_UnpackFullState(netpeer_t *np) {
  pbuf_t *pbuf = N_PeerGetIncomingMessageData(np);
  int m_state_tic;
  game_state_t *gs;

  read_int(pbuf, m_state_tic, "game state tic");
  read_packed_game_state(pbuf, gs, m_state_tic, "game state data");

  G_SetLatestState(gs);

  N_PeerUpdateSyncTIC(np, gs->tic);

  return true;
}

void N_PackAuthResponse(netpeer_t *np, auth_level_e auth_level) {
  pbuf_t *pbuf = N_PeerBeginMessage(np, NM_AUTH);

  M_PBufWriteUChar(pbuf, auth_level);
}

bool N_UnpackAuthResponse(netpeer_t *np, auth_level_e *auth_level) {
  pbuf_t *pbuf = N_PeerGetIncomingMessageData(np);
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

void N_PackPing(netpeer_t *np, double server_time) {
  pbuf_t *pbuf = N_PeerBeginMessage(np, NM_PING);

  M_PBufWriteDouble(pbuf, server_time);
}

bool N_UnpackPing(netpeer_t *np, double *server_time) {
  pbuf_t *pbuf = N_PeerGetIncomingMessageData(np);
  double m_server_time;

  read_double(pbuf, m_server_time, "server time");

  *server_time = m_server_time;

  return true;
}

void N_PackChatMessage(netpeer_t *np, const char *message) {
  pbuf_t *pbuf = N_PeerBeginMessage(np, NM_CHAT_MESSAGE);

  M_PBufWriteUChar(pbuf, CHAT_CHANNEL_ALL);
  M_PBufWriteString(pbuf, message, strlen(message));
}

void N_PackRelayedChatMessage(netpeer_t *np, unsigned short sender,
                                             const char *message) {
  pbuf_t *pbuf = N_PeerBeginMessage(np, NM_CHAT_MESSAGE);

  M_PBufWriteUChar(pbuf, CHAT_CHANNEL_ALL);
  M_PBufWriteUShort(pbuf, sender);
  M_PBufWriteString(pbuf, message, strlen(message));
}

void N_PackTeamChatMessage(netpeer_t *np, const char *message) {
  pbuf_t *pbuf = N_PeerBeginMessage(np, NM_CHAT_MESSAGE);

  M_PBufWriteUChar(pbuf, CHAT_CHANNEL_TEAM);
  M_PBufWriteString(pbuf, message, strlen(message));
}

void N_PackRelayedTeamChatMessage(netpeer_t *np, unsigned short sender,
                                                 const char *message) {
  pbuf_t *pbuf = N_PeerBeginMessage(np, NM_CHAT_MESSAGE);

  M_PBufWriteUChar(pbuf, CHAT_CHANNEL_TEAM);
  M_PBufWriteUShort(pbuf, sender);
  M_PBufWriteString(pbuf, message, strlen(message));
}

void N_PackPlayerChatMessage(netpeer_t *np, unsigned short recipient,
                                            const char *message) {
  pbuf_t *pbuf = N_PeerBeginMessage(np, NM_CHAT_MESSAGE);

  M_PBufWriteUChar(pbuf, CHAT_CHANNEL_PLAYER);
  M_PBufWriteUShort(pbuf, recipient);
  M_PBufWriteString(pbuf, message, strlen(message));
}

void N_PackRelayedPlayerChatMessage(netpeer_t *np, unsigned short sender,
                                                   unsigned short recipient,
                                                   const char *message) {
  pbuf_t *pbuf = N_PeerBeginMessage(np, NM_CHAT_MESSAGE);

  M_PBufWriteUChar(pbuf, CHAT_CHANNEL_PLAYER);
  M_PBufWriteUShort(pbuf, sender);
  M_PBufWriteUShort(pbuf, recipient);
  M_PBufWriteString(pbuf, message, strlen(message));
}

void N_PackServerChatMessage(netpeer_t *np, const char *message) {
  pbuf_t *pbuf = N_PeerBeginMessage(np, NM_CHAT_MESSAGE);

  M_PBufWriteUChar(pbuf, CHAT_CHANNEL_SERVER);
  M_PBufWriteString(pbuf, message, strlen(message));
}

bool N_UnpackChatMessage(netpeer_t *np,
                         chat_channel_e *chat_channel,
                         unsigned short *message_sender,
                         unsigned short *message_recipient,
                         buf_t *message_contents) {
  pbuf_t *pbuf = N_PeerGetIncomingMessageData(np);
  unsigned char m_chat_channel;
  unsigned short m_message_sender = 0;
  unsigned short m_message_recipient = 0;

  read_ranged_uchar(pbuf, m_chat_channel, "chat channel",
    CHAT_CHANNEL_MIN, CHAT_CHANNEL_MAX
  );

  if ((CLIENT) && (m_chat_channel != CHAT_CHANNEL_SERVER)) {
    read_player(pbuf, m_message_sender);
  }

  if (m_chat_channel == CHAT_CHANNEL_PLAYER) {
    read_player(pbuf, m_message_recipient);
  }

  read_string(
    pbuf, message_contents, "chat message", MAX_CHAT_MESSAGE_SIZE
  );

  *chat_channel = m_chat_channel;

  if (CLIENT) {
    *message_sender = m_message_sender;
  }
  else {
    *message_sender = N_PeerGetPlayernum(np);
  }

  if (m_chat_channel == CHAT_CHANNEL_PLAYER) {
    *message_recipient = m_message_recipient;
  }

  return true;
}

void N_PackSync(netpeer_t *np) {
  pbuf_t *pbuf = N_PeerBeginMessage(np, NM_SYNC);
  unsigned int dest_playernum = N_PeerGetPlayernum(np);
  def_bitmap_zero(bitmap, MAXPLAYERS);

  if (CLIENT) {
    for (int i = 0; i < MAXPLAYERS; i++) {
      if (playeringame[i] && (i > 0) && (i != consoleplayer)) {
        bitmap_set_bit(bitmap, i);
      }
    }

    M_PBufWriteNum(pbuf, N_PeerGetSyncTIC(np));
    M_PBufWriteBitmap(pbuf, (char *)bitmap, sizeof(bitmap));

    pack_unsynchronized_commands(pbuf, np, consoleplayer);

    for (size_t i = 0; i < MAXPLAYERS; i++) {
      if (!bitmap_get_bit(bitmap, i)) {
        continue;
      }

      M_PBufWriteUNum(pbuf, N_PeerGetSyncCommandIndexForPlayer(np, i));
    }
  }
  else if (SERVER) {
    player_t *player = &players[N_PeerGetPlayernum(np)];
    game_state_delta_t *delta = N_PeerGetSyncStateDelta(np);

    M_PBufWriteUNum(pbuf, N_PeerGetSyncCommandIndex(np));

    NETPEER_FOR_EACH(iter) {
      unsigned int playernum  = N_PeerGetPlayernum(iter.np);

      bitmap_set_bit(bitmap, playernum);
    }

    M_PBufWriteBitmap(pbuf, (char *)bitmap, sizeof(bitmap));

    NETPEER_FOR_EACH(iter) {
      unsigned int playernum  = N_PeerGetPlayernum(iter.np);

      if (playernum == dest_playernum) {
        pack_run_commands(pbuf, dest_playernum);
      }
      else {
        pack_unsynchronized_commands(pbuf, np, playernum);
      }
    }

    D_Msg(MSG_SYNC, "(%d) Syncing %d => %d (%d)\n",
      gametic,
      delta->from_tic,
      delta->to_tic,
      player->cmdq.latest_command_run_index
    );
    M_PBufWriteInt(pbuf, delta->from_tic);
    M_PBufWriteInt(pbuf, delta->to_tic);
    M_PBufWriteBytes(pbuf, delta->data.data, delta->data.size);
  }
}

bool N_UnpackSync(netpeer_t *np) {
  pbuf_t *pbuf = N_PeerGetIncomingMessageData(np);
  uint32_t m_command_index;
  def_bitmap(bitmap, MAXPLAYERS);

  if (CLIENT) {
    int m_delta_from_tic;
    int m_delta_to_tic;

    read_uint(pbuf, m_command_index, "command index");
    read_bitmap(pbuf, bitmap, MAXPLAYERS, "sync player bitmap");

    for (size_t i = 0; i < MAXPLAYERS; i++) {
      uint32_t m_command_count;

      if (!bitmap_get_bit(bitmap, i)) {
        continue;
      }

      if (i == consoleplayer) {
        read_uint(pbuf, m_command_count, "consoleplayer sync command count");

        for (uint32_t j = 0; j < m_command_count; j++) {
          uint32_t m_run_command_index;
          uint32_t m_server_tic;

          read_uint(pbuf, m_run_command_index, "run command index");
          read_uint(pbuf, m_server_tic, "server TIC");

          P_UpdateCommandServerTic(i, m_run_command_index, m_server_tic);
        }
      }
      else {
        read_uint(pbuf, m_command_count, "command count");

        for (unsigned int j = 0; j < m_command_count; j++) {
          netticcmd_t m_ncmd;

          unpack_net_command(pbuf, m_ncmd);

          P_QueuePlayerCommand(i, &m_ncmd);
        }

        m_command_index = 0;

        for (unsigned int ci = 0; ci < P_GetCommandCount(i); ci++) {
          netticcmd_t *m_ncmd = P_GetCommand(i, ci);

          if (!m_ncmd) {
            continue;
          }

          if (m_ncmd->server_tic == 0) {
            continue;
          }

          if (m_ncmd->index <= m_command_index) {
            continue;
          }

          m_command_index = m_ncmd->index;
        }
      }

      if (m_command_index > N_PeerGetSyncCommandIndexForPlayer(np, i)) {
        I_Error("Bad server sync command index for %zu: %u > %u",
          i, m_command_index, N_PeerGetSyncCommandIndexForPlayer(np, i)
        );
      }

      N_PeerUpdateSyncCommandIndexForPlayer(np, i, m_command_index);
    }

    read_int(pbuf, m_delta_from_tic, "delta from tic");
    read_int(pbuf, m_delta_to_tic,   "delta to tic");

    /* [CG] pbuf now points to the delta's binary data */
    N_PeerUpdateSyncStateDelta(np, m_delta_from_tic, m_delta_to_tic, pbuf);
  }
  else if (SERVER) {
    int m_sync_tic;
    unsigned int m_command_count;
    unsigned int playernum = N_PeerGetPlayernum(np);

    read_int(pbuf, m_sync_tic, "sync tic");
    read_bitmap(pbuf, bitmap, MAXPLAYERS, "sync player bitmap");

    read_uint(pbuf, m_command_count, "command count");

    N_PeerUpdateSyncTIC(np, m_sync_tic);

    for (unsigned int i = 0; i < m_command_count; i++) {
      netticcmd_t m_ncmd;

      unpack_net_command(pbuf, m_ncmd);

      m_ncmd.server_tic = 0;

      P_QueuePlayerCommand(playernum, &m_ncmd);
    }

    if (m_command_count > 0) {
      m_command_index = P_GetLatestCommandIndex(playernum);

      N_PeerUpdateSyncCommandIndexForPlayer(np, playernum, m_command_index);
    }

    for (size_t i = 0; i < MAXPLAYERS; i++) {
      if (i == playernum) {
        continue;
      }

      if (!bitmap_get_bit(bitmap, i)) {
        continue;
      }

      read_uint(pbuf, m_command_index, "command index");

      N_PeerUpdateSyncCommandIndexForPlayer(np, i, m_command_index);
    }
  }

  return true;
}

bool N_UnpackPlayerPreferenceChange(netpeer_t *np, int *tic,
                                                   unsigned short *playernum,
                                                   unsigned int *count) {
  pbuf_t *pbuf = N_PeerGetIncomingMessageData(np);
  unsigned short m_playernum = 0;
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

bool N_UnpackPlayerPreferenceName(netpeer_t *np, buf_t *buf) {
  pbuf_t *pbuf = N_PeerGetIncomingMessageData(np);

  read_string(
    pbuf, buf, "player preference name", MAX_PLAYER_PREFERENCE_NAME_SIZE
  );

  return true;
}

void N_PackNameChange(netpeer_t *np, unsigned short playernum,
                                     const char *new_name) {
  pack_player_preference_change(pbuf, gametic, playernum, "name", 4);

  M_PBufWriteString(pbuf, new_name, strlen(new_name));
}

bool N_UnpackNameChange(netpeer_t *np, buf_t *buf) {
  pbuf_t *pbuf = N_PeerGetIncomingMessageData(np);

  read_string(pbuf, buf, "new name", MAX_PLAYER_NAME_SIZE);

  return true;
}

void N_PackTeamChange(netpeer_t *np, unsigned short playernum,
                                     unsigned char new_team) {
  pack_player_preference_change(pbuf, gametic, playernum, "team", 4);

  M_PBufWriteUChar(pbuf, new_team);

  puts("Packed team change");
}

bool N_UnpackTeamChange(netpeer_t *np, unsigned char *new_team) {
  pbuf_t *pbuf = N_PeerGetIncomingMessageData(np);
  int team_count = 0;
  unsigned char m_new_team = 0;

  if (team_count > 0) { /* CG: TODO: teams */
    read_ranged_uchar(pbuf, m_new_team, "new team index", 0, team_count - 1);
  }

  *new_team = m_new_team;

  return true;
}

void N_PackPWOChange(netpeer_t *np, unsigned short playernum) {
  pack_player_preference_change(pbuf, gametic, playernum, "pwo", 3);

  M_PBufWriteUChar(pbuf, 0); /* CG: TODO */

  puts("Packed PWO change");
}

bool N_UnpackPWOChange(netpeer_t *np) {
  return false; /* CG: TODO */
}

void N_PackWSOPChange(netpeer_t *np, unsigned short playernum,
                                     unsigned char new_wsop_flags) {
  pack_player_preference_change(pbuf, gametic, playernum, "wsop", 4);

  M_PBufWriteUChar(pbuf, new_wsop_flags);

  puts("Packed WSOP change");
}

bool N_UnpackWSOPChange(netpeer_t *np, unsigned char *new_wsop_flags) {
  pbuf_t *pbuf = N_PeerGetIncomingMessageData(np);
  unsigned char m_new_wsop_flags = 0;

  read_ranged_uchar(
    pbuf, m_new_wsop_flags, "new WSOP flags", WSOP_NONE, WSOP_MAX - 1
  );

  *new_wsop_flags = m_new_wsop_flags;

  return true;
}

void N_PackBobbingChange(netpeer_t *np, unsigned short playernum,
                                        double new_bobbing_amount) {
  pack_player_preference_change(pbuf, gametic, playernum, "bobbing", 7);

  M_PBufWriteDouble(pbuf, new_bobbing_amount);

  puts("Packed bobbing change");
}

bool N_UnpackBobbingChanged(netpeer_t *np, double *new_bobbing_amount) {
  pbuf_t *pbuf = N_PeerGetIncomingMessageData(np);
  double m_new_bobbing_amount = 0;

  read_ranged_double(
    pbuf, m_new_bobbing_amount, "new bobbing amount", 0.0, 1.0
  );

  *new_bobbing_amount = m_new_bobbing_amount;

  return true;
}

void N_PackAutoaimChange(netpeer_t *np, unsigned short playernum,
                                        bool new_autoaim_enabled) {
  pack_player_preference_change(pbuf, gametic, playernum, "autoaim", 7);

  M_PBufWriteBool(pbuf, new_autoaim_enabled);

  puts("Packed autoaim change");
}

bool N_UnpackAutoaimChange(netpeer_t *np, bool *new_autoaim_enabled) {
  pbuf_t *pbuf = N_PeerGetIncomingMessageData(np);
  bool m_new_autoaim_enabled = false;

  read_bool(pbuf, m_new_autoaim_enabled, "new autoaim enabled value");

  *new_autoaim_enabled = m_new_autoaim_enabled;

  return true;
}

void N_PackWeaponSpeedChange(netpeer_t *np, unsigned short playernum,
                                            unsigned char new_weapon_speed) {
  pack_player_preference_change(pbuf, gametic, playernum, "weapon speed", 12);

  M_PBufWriteUChar(pbuf, new_weapon_speed);

  puts("Packed weapon speed change");
}

bool N_UnpackWeaponSpeedChange(netpeer_t *np,
                               unsigned char *new_weapon_speed) {
  pbuf_t *pbuf = N_PeerGetIncomingMessageData(np);
  unsigned char m_new_weapon_speed = 0;

  read_uchar(pbuf, m_new_weapon_speed, "new weapon speed");

  *new_weapon_speed = m_new_weapon_speed;

  return true;
}

void N_PackColorChange(netpeer_t *np, unsigned short playernum,
                                      unsigned char new_red,
                                      unsigned char new_green,
                                      unsigned char new_blue) {
  pack_player_preference_change(pbuf, gametic, playernum, "color", 5);

  M_PBufWriteUInt(pbuf, (new_red << 24) | (new_green << 16) | (new_blue << 8));

  puts("Packed color change");
}

bool N_UnpackColorChange(netpeer_t *np, unsigned char *new_red,
                                            unsigned char *new_green,
                                            unsigned char *new_blue) {
  pbuf_t *pbuf = N_PeerGetIncomingMessageData(np);
  unsigned int m_new_color = 0;

  read_uint(pbuf, m_new_color, "new color");

  *new_red   = (m_new_color >> 24) & 0xFF;
  *new_green = (m_new_color >> 16) & 0xFF;
  *new_blue  = (m_new_color >>  8) & 0xFF;

  return true;
}

void N_PackColorIndexChange(netpeer_t *np, unsigned short playernum,
                                           int new_color_index) {
  pack_player_preference_change(pbuf, gametic, playernum, "color index", 11);

  M_PBufWriteInt(pbuf, new_color_index);

  puts("Packed color index change");
}

bool N_UnpackColorIndexChange(netpeer_t *np, int *new_color_index) {
  pbuf_t *pbuf = N_PeerGetIncomingMessageData(np);
  int m_new_color_index = 0;

  /* CG: TODO: Ensure new color map index is reasonable */
  read_int(pbuf, m_new_color_index, "new color index");

  *new_color_index = m_new_color_index;

  return true;
}

void N_PackSkinChange(netpeer_t *np, unsigned short playernum) {
  pack_player_preference_change(pbuf, gametic, playernum, "skin name", 9);

  M_PBufWriteUChar(pbuf, 0); /* CG: TODO */

  puts("Packed skin change");
}

bool N_UnpackSkinChange(netpeer_t *np, unsigned short *playernum) {
  return false; /* CG: TODO */
}

void N_PackAuthRequest(netpeer_t *np, const char *password) {
  pbuf_t *pbuf = N_PeerBeginMessage(np, NM_AUTH);

  M_PBufWriteString(pbuf, password, strlen(password));

  puts("Packed auth request");
}

bool N_UnpackAuthRequest(netpeer_t *np, buf_t *buf) {
  pbuf_t *pbuf = N_PeerGetIncomingMessageData(np);

  read_string(
    pbuf, buf, "authorization request password", MAX_PASSWORD_LENGTH
  );

  return true;
}

void N_PackRCONCommand(netpeer_t *np, const char *command) {
  pbuf_t *pbuf = N_PeerBeginMessage(np, NM_RCON_COMMAND);

  M_PBufWriteString(pbuf, command, strlen(command));

  puts("Packed RCON command");
}

bool N_UnpackRCONCommand(netpeer_t *np, buf_t *buf) {
  pbuf_t *pbuf = N_PeerGetIncomingMessageData(np);

  read_string(pbuf, buf, "RCON command", MAX_COMMAND_LENGTH);

  return true;
}

void N_PackVoteRequest(netpeer_t *np, const char *command) {
  pbuf_t *pbuf = N_PeerBeginMessage(np, NM_VOTE_REQUEST);

  M_PBufWriteString(pbuf, command, strlen(command));

  puts("Packed vote request");
}

bool N_UnpackVoteRequest(netpeer_t *np, buf_t *buf) {
  pbuf_t *pbuf = N_PeerGetIncomingMessageData(np);

  read_string(pbuf, buf, "vote command", MAX_COMMAND_LENGTH);

  return true;
}

void N_PackGameActionChange(netpeer_t *np) {
  pbuf_t *pbuf = N_PeerBeginMessage(np, NM_GAME_ACTION);

  M_PBufWriteInt(pbuf, G_GetGameAction());
  M_PBufWriteInt(pbuf, gametic);
}

bool N_UnpackGameActionChange(netpeer_t *np, gameaction_t *new_gameaction,
                                             int *new_gametic) {
  pbuf_t *pbuf = N_PeerGetIncomingMessageData(np);

  int m_new_gameaction = 0;
  int m_new_gametic = 0;

  read_ranged_int(
    pbuf, m_new_gameaction, "game action", ga_nothing, ga_worlddone
  );
  read_int(pbuf, m_new_gametic, "game action gametic");

  *new_gameaction = m_new_gameaction;
  *new_gametic = m_new_gametic;

  return true;
}

/* vi: set et ts=2 sw=2: */

