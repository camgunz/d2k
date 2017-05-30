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

#include "i_system.h"
#include "m_bitmap.h"
#include "m_file.h"
#include "d_deh.h"
#include "d_event.h"
#include "d_main.h"
#include "d_res.h"
#include "g_game.h"
#include "g_state.h"
#include "p_map.h"
#include "pl_main.h"
#include "w_wad.h"

#include "n_main.h"
#include "n_msg.h"
#include "n_peer.h"
#include "n_proto.h"
#include "cl_cmd.h"
#include "cl_main.h"
#include "cl_net.h"

/* CG: FIXME: Most of these should be more than just defines tucked here */
#define MAX_IWAD_NAME_LENGTH 20
#define MAX_RESOURCE_NAMES 1000
#define MAX_RESOURCE_NAME_LENGTH 128
#define MAX_PLAYER_PREFERENCE_NAME_SIZE 32
#define MAX_PLAYER_NAME_SIZE 32
#define MAX_PASSWORD_LENGTH 256
#define MAX_COMMAND_LENGTH 32
#define MAX_CHAT_MESSAGE_SIZE 256

#define check_range(var, min, max)                                            \
  if ((var < min) || (var > max)) {                                           \
    D_MsgLocalError(                                                          \
      "%s: Invalid message: %s is out of range (%s, %s)\n",                   \
      __func__, #var, #min, #max                                              \
    );                                                                        \
  }

#define read_num(pbuf, var, name)                                             \
  if (!M_PBufReadNum(pbuf, &var)) {                                           \
    D_MsgLocalError(                                                          \
      "%s: Error reading %s: %s.\n", __func__, name, M_PBufGetError(pbuf)     \
    );                                                                        \
    return false;                                                             \
  }

#define read_unum(pbuf, var, name)                                            \
  if (!M_PBufReadUNum(pbuf, &var)) {                                          \
    D_MsgLocalError(                                                          \
      "%s: Error reading %s: %s.\n", __func__, name, M_PBufGetError(pbuf)     \
    );                                                                        \
    return false;                                                             \
  }

#define read_ranged_num(pbuf, var, name, min, max)                            \
  read_num(pbuf, var, name)                                                   \
  check_range(var, min, max) 

#define read_ranged_unum(pbuf, var, name, min, max)                           \
  read_unum(pbuf, var, name)                                                  \
  check_range(var, min, max) 

#define _read_cnum(pbuf, var, name, type, min, max) do {                      \
  int64_t _int_var = 0;                                                       \
  read_ranged_num(pbuf, _int_var, name, min, max)                             \
  var = (type)_int_var;                                                       \
} while (0);

#define _read_cunum(pbuf, var, name, type, min, max) do {                     \
  uint64_t _uint_var = 0;                                                     \
  read_ranged_unum(pbuf, _uint_var, name, min, max)                           \
  var = (type)_uint_var;                                                      \
} while (0);

#define read_ranged_char(pbuf, var, name, min, max)                           \
  _read_cnum(pbuf, var, name, int8_t, min, max)

#define read_ranged_uchar(pbuf, var, name, min, max)                          \
  _read_cunum(pbuf, var, name, uint8_t, min, max)

#define read_char(pbuf, var, name)                                            \
  read_ranged_char(pbuf, var, name, -128, 127)   

#define read_uchar(pbuf, var, name)                                           \
  read_ranged_uchar(pbuf, var, name, 0, 255)   

#define read_ranged_short(pbuf, var, name, min, max)                          \
  _read_cnum(pbuf, var, name, int16_t, min, max)

#define read_ranged_ushort(pbuf, var, name, min, max)                         \
  _read_cunum(pbuf, var, name, uint16_t, min, max)

#define read_short(pbuf, var, name)                                           \
  read_ranged_short(pbuf, var, name, -32768, 32767)   

#define read_ushort(pbuf, var, name)                                          \
  read_ranged_ushort(pbuf, var, name, 0, 65535)   

#define read_ranged_int(pbuf, var, name, min, max)                            \
  _read_cnum(pbuf, var, name, int8_t, min, max)

#define read_ranged_uint(pbuf, var, name, min, max)                           \
  _read_cunum(pbuf, var, name, uint8_t, min, max)

#define read_int(pbuf, var, name)                                             \
  read_ranged_int(pbuf, var, name, -2147483647, 2147483647)   

#define read_uint(pbuf, var, name)                                            \
  read_ranged_uint(pbuf, var, name, 0, 4294967295)   

#define read_ranged_long(pbuf, var, name, min, max)                           \
  _read_cnum(pbuf, var, name, int8_t, min, max)

#define read_ranged_ulong(pbuf, var, name, min, max)                          \
  _read_cunum(pbuf, var, name, uint8_t, min, max)

#define read_long(pbuf, var, name)                                            \
  read_ranged_long(                                                           \
    pbuf, var, name, -9223372036854775808LL, 9223372036854775807LL            \
  )

#define read_ulong(pbuf, var, name)                                           \
  read_ranged_ulong(pbuf, var, name, 0, 18446744073709551615ULL)   

#define read_double(pbuf, var, name)                                          \
  if (!M_PBufReadDouble(pbuf, &var)) {                                        \
    D_MsgLocalError(                                                          \
      "%s: Error reading %s: %s.\n", __func__, name, M_PBufGetError(pbuf)     \
    );                                                                        \
    return false;                                                             \
  }

#define read_ranged_double(pbuf, var, name, min, max)                         \
  read_double(pbuf, var, name);                                               \
  check_range(var, min, max);

#define read_bool(pbuf, var, name)                                            \
  if (!M_PBufReadBool(pbuf, &var)) {                                          \
    D_MsgLocalError(                                                          \
      "%s: Error reading %s: %s.\n", __func__, name, M_PBufGetError(pbuf)     \
    );                                                                        \
    return false;                                                             \
  }

#define read_array(pbuf, var, name)                                           \
  if (!M_PBufReadArray(pbuf, &var)) {                                         \
    D_MsgLocalError(                                                          \
      "%s: Error reading %s: %s.\n", __func__, name, M_PBufGetError(pbuf)     \
    );                                                                        \
    return false;                                                             \
  }

#define read_map(pbuf, var, name)                                             \
  if (!M_PBufReadMap(pbuf, &var)) {                                           \
    D_MsgLocalError(                                                          \
      "%s: Error reading %s: %s.\n", __func__, name, M_PBufGetError(pbuf)     \
    );                                                                        \
    return false;                                                             \
  }

#define read_bytes(pbuf, var, name)                                           \
  M_BufferClear(&var);                                                        \
  if (!M_PBufReadBytes(pbuf, &var)) {                                         \
    D_MsgLocalError(                                                          \
      "%s: Error reading %s: %s.\n", __func__, name, M_PBufGetError(pbuf)     \
    );                                                                        \
    return false;                                                             \
  }

#define read_packed_game_state(pbuf, var, tic, name)                          \
  var = G_ReadNewStateFromPackedBuffer(tic, pbuf);                            \
  if (!var) {                                                                 \
    D_MsgLocalError(                                                          \
      "%s: Error reading %s: %s.\n", __func__, name, M_PBufGetError(pbuf)     \
    );                                                                        \
    return false;                                                             \
  }

#define read_string(pbuf, var, name, sz)                                      \
  M_BufferClear(var);                                                         \
  if (!M_PBufReadString(pbuf, var, sz)) {                                     \
    D_MsgLocalError(                                                          \
      "%s: Error reading %s: %s.\n", __func__, name, M_PBufGetError(pbuf)     \
    );                                                                        \
    return false;                                                             \
  }

#define read_string_array(pbuf, var, name, count, length)                     \
  if (!M_PBufReadStringArray(pbuf, var, count, length)) {                     \
    D_MsgLocalError(                                                          \
      "%s: Error reading %s: %s.\n", __func__, name, M_PBufGetError(pbuf)     \
    );                                                                        \
    return false;                                                             \
  }

#define read_player(pbuf, var) do {                                           \
  uint32_t _player_id = 0;                                                    \
  read_uint(pbuf, _player_id, "player ID")                                    \
  if ((_player_id == 0) || (_player_id >= PLAYER_CAP)) {                      \
    D_MsgLocalError("%s: Invalid player ID\n", __func__);                     \
    return false;                                                             \
  }                                                                           \
  var = P_PlayersLookup(_player_id);                                          \
  if (!var) {                                                                 \
    D_MsgLocalError("%s: No player for ID %u\n", __func__, _player_id);       \
    return false;                                                             \
  }                                                                           \
} while (0);

#define read_peer(pbuf, var) do {                                             \
  uint32_t _peer_id = 0;                                                      \
  read_uint(pbuf, _peer_id, "peer ID")                                        \
  if (_peer_id == 0) {                                                        \
    D_MsgLocalError("%s: Invalid peer ID\n", __func__);                       \
    return false;                                                             \
  }                                                                           \
  var = N_PeersLookup(_peer_id);                                              \
  if (!var) {                                                                 \
    D_MsgLocalError("%s: No peer for ID %u\n", __func__, _peer_id);           \
    return false;                                                             \
  }                                                                           \
} while (0);

#define pack_player_preference_change(pbuf, gametic, playernum, pn, pnsz)     \
  pbuf_t *pbuf = N_PeerBeginMessage(np, NM_PLAYER_PREFERENCE_CHANGE);         \
  M_PBufWriteNum(pbuf, gametic);                                              \
  M_PBufWriteUNum(pbuf, playernum);                                           \
  M_PBufWriteString(pbuf, pn, pnsz)

#define unpack_net_command(pbuf, m_icmd)                                      \
  read_uint(pbuf,  m_icmd.index,      "command index");                       \
  read_uint(pbuf,  m_icmd.tic,        "command TIC");                         \
  read_uint(pbuf,  m_icmd.server_tic, "server command TIC");                  \
  read_char(pbuf,  m_icmd.forward,    "command forward value");               \
  read_char(pbuf,  m_icmd.side,       "command side value");                  \
  read_short(pbuf, m_icmd.angle,      "command angle value");                 \
  read_uchar(pbuf, m_icmd.buttons,    "command buttons value")

static void free_string(gpointer data) {
  free(data);
}

static void pack_run_commands(pbuf_t *pbuf, netpeer_t *np) {
  player_t *player = N_PeerGetPlayer(np);
  GPtrArray *commands = NULL;
  size_t command_count = 0;
  size_t total_command_count = 0;

  if (!player) {
    D_MsgLocalError("pack_run_commands: No player for peer %u\n",
      N_PeerGetID(np)
    );
    return;
  }

  commands = player->cmdq.commands;

  for (size_t i = 0; i < commands->len; i++) {
    idxticcmd_t *icmd = g_ptr_array_index(commands, i);

    total_command_count++;

    if (icmd->server_tic != 0) {
      command_count++;
    }
  }

  N_MsgSyncLocalDebug("(%d) (%u) Synced/Total: %zu/%zu\n",
    gametic,
    player->id,
    command_count,
    total_command_count
  );

  M_PBufWriteUNum(pbuf, command_count);

  if (command_count > 0) {
    N_MsgSyncLocalDebug("(%d) (%u) Ran commands: [", gametic, player->id);
  }

  for (size_t i = 0; i < commands->len; i++) {
    idxticcmd_t *icmd = g_ptr_array_index(commands, i);

    if (icmd->server_tic != 0) {
      N_MsgSyncLocalDebug(" %u/%d/%d",
        icmd->index,
        icmd->tic,
        icmd->server_tic
      );
      M_PBufWriteUNum(pbuf, icmd->index);
      M_PBufWriteUNum(pbuf, icmd->server_tic);
    }
  }

  if (command_count > 0) {
    N_MsgSyncLocalDebug(" ]\n");
  }
}

static void pack_unsynchronized_commands(pbuf_t *pbuf, netpeer_t *np,
                                                       player_t *player) {
  GPtrArray *commands = NULL;
  unsigned int command_count = 0;

  if (!player) {
    D_MsgLocalError("pack_unsynchronized_commands: No player for peer %u\n",
      N_PeerGetID(np)
    );
    return;
  }

  commands = player->cmdq.commands;

  for (size_t i = 0; i < commands->len; i++) {
    idxticcmd_t *icmd = (idxticcmd_t *)g_ptr_array_index(commands, i);

    if (icmd->index > N_PeerSyncGetCommandIndexForPlayer(np, player)) {
      command_count++;
    }
    else {
      N_MsgSyncLocalDebug("(%d) (%u->%u) Skipping command (%u <= %u)\n",
        gametic,
        player->id,
        N_PeerGetID(np),
        icmd->index,
        N_PeerSyncGetCommandIndexForPlayer(np, player)
      );
    }
  }

  M_PBufWriteUNum(player->id);
  M_PBufWriteUNum(pbuf, command_count);

  if (command_count > 0) {
    N_MsgSyncLocalDebug("(%d) (%u->%u) Unsync'd commands: [",
      gametic,
      player->id,
      N_PeerGetID(np)
    );
  }

  for (size_t i = 0; i < commands->len; i++) {
    idxticcmd_t *icmd = (idxticcmd_t *)g_ptr_array_index(commands, i);

    if (icmd->index > N_PeerSyncGetCommandIndexForPlayer(np, player)) {
      N_MsgSyncLocalDebug(" %u/%u/%u",
        icmd->index,
        icmd->tic,
        icmd->server_tic
      );
      M_PBufWriteUNum(pbuf, icmd->index);
      M_PBufWriteUNum(pbuf, icmd->tic);
      M_PBufWriteUNum(pbuf, icmd->server_tic);
      M_PBufWriteNum(pbuf, icmd->forward);
      M_PBufWriteNum(pbuf, icmd->side);
      M_PBufWriteNum(pbuf, icmd->angle);
      M_PBufWriteUNum(pbuf, icmd->buttons);
    }
  }

  if (command_count > 0) {
    N_MsgSyncLocalDebug(" ]\n");
  }
}

void N_PackSetupRequest(netpeer_t *np) {
  pbuf_t *pbuf = N_PeerBeginMessage(np, NM_SETUP);

  M_PBufWriteUNum(pbuf, 0);
}

void N_PackSetup(netpeer_t *np) {
  game_state_t *gs = G_GetLatestState();
  pbuf_t *pbuf = NULL;
  size_t resource_count = 0;
  size_t deh_count = deh_files->len;
  const char *iwad = D_GetIWAD();

  pbuf = N_PeerBeginMessage(np, NM_SETUP);

  M_PBufWriteNum(pbuf, deathmatch);

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

  M_PBufWriteUNum(pbuf, deh_count);
  if (deh_count > 0) {
    for (unsigned int i = 0; i < deh_files->len; i++) {
      deh_file_t *df = g_ptr_array_index(deh_files, i);
      char *deh_name;

      if (df->lumpnum) {
        M_PBufWriteBool(pbuf, true);
        M_PBufWriteNum(pbuf, df->lumpnum);
        continue;
      }

      deh_name = M_Basename(df->filename);

      if (!deh_name)
        I_Error("N_PackSetup: Error getting basename of %s\n", df->filename);

      M_PBufWriteString(pbuf, deh_name, strlen(deh_name));
      free(deh_name);
    }
  }

  M_PBufWriteNum(pbuf, gs->tic);
  M_PBufWriteBytes(pbuf, M_PBufGetData(gs->data), M_PBufGetSize(gs->data));
  N_PeerSyncUpdateTIC(np, gs->tic);
}

bool N_UnpackSetup(netpeer_t *np) {
  pbuf_t *pbuf = N_PeerGetIncomingMessageData(np);
  int m_deathmatch = 0;
  int m_state_tic;
  game_state_t *gs;
  bool has_resources;
  unsigned int deh_file_count;
  buf_t iwad_buf;
  char *iwad_name;
  char *iwad_path;
  GPtrArray *rf_list;

  read_ranged_int(pbuf, m_deathmatch, "deathmatch", 0, 2);

  M_BufferInit(&iwad_buf);
  read_string(pbuf, &iwad_buf, "IWAD", MAX_IWAD_NAME_LENGTH);
  iwad_name = M_StripExtension(M_BufferGetData(&iwad_buf));

  M_BufferFree(&iwad_buf);

  iwad_path = I_FindFile(iwad_name, ".wad");

  if (!iwad_path) {
    D_MsgLocalWarn("IWAD %s not found\n", iwad_name);
    free(iwad_name);
    return false;
  }

  free(iwad_name);

  W_ReleaseAllWads();
  D_ClearIWAD();
  D_ClearResourceFiles();
  D_ClearDEHFiles();

  D_AddIWAD(iwad_path);
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

      D_AddResource(resource_name, source_net);
    }
    g_ptr_array_free(rf_list, true);
  }

  D_AddResource(PACKAGE_TARNAME ".wad", source_auto_load);

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
        D_AddDEH(NULL, deh_lumpnum);
      }
      else {
        read_string(pbuf, &deh_name, "DeH/BEX name", MAX_RESOURCE_NAME_LENGTH);
        D_AddDEH(M_BufferGetData(&deh_name), deh_lumpnum);
      }
    }
  }

  read_int(pbuf, m_state_tic, "game state tic");
  read_packed_game_state(pbuf, gs, m_state_tic, "game state data");

  deathmatch = m_deathmatch;

  G_SetLatestState(gs);

  N_PeerSyncUpdateTIC(np, gs->tic);

  return true;
}

void N_PackFullState(netpeer_t *np) {
  game_state_t *gs = G_GetLatestState();
  pbuf_t *pbuf = N_PeerBeginMessage(np, NM_FULL_STATE);

  M_PBufWriteNum(pbuf, gs->tic);
  M_PBufWriteBytes(pbuf, M_PBufGetData(gs->data), M_PBufGetSize(gs->data));

  N_PeerSyncUpdateTIC(np, gs->tic);
}

bool N_UnpackFullState(netpeer_t *np) {
  pbuf_t *pbuf = N_PeerGetIncomingMessageData(np);
  int m_state_tic;
  game_state_t *gs;

  read_int(pbuf, m_state_tic, "game state tic");
  read_packed_game_state(pbuf, gs, m_state_tic, "game state data");

  G_SetLatestState(gs);

  N_PeerSyncUpdateTIC(np, gs->tic);

  return true;
}

void N_PackAuthResponse(netpeer_t *np, auth_level_e auth_level) {
  pbuf_t *pbuf = N_PeerBeginMessage(np, NM_AUTH);

  M_PBufWriteUNum(pbuf, auth_level);
}

bool N_UnpackAuthResponse(netpeer_t *np, auth_level_e *auth_level) {
  pbuf_t *pbuf = N_PeerGetIncomingMessageData(np);
  unsigned char m_auth_level = 0;

  read_ranged_uchar(
    pbuf, m_auth_level, "auth level", AUTH_LEVEL_NONE, AUTH_LEVEL_MAX - 1
  );

  switch (m_auth_level) {
    case AUTH_LEVEL_NONE:
    case AUTH_LEVEL_SPECTATOR:
    case AUTH_LEVEL_PLAYER:
    case AUTH_LEVEL_MODERATOR:
    case AUTH_LEVEL_ADMINISTRATOR:
      *auth_level = m_auth_level;
    break;
    default:
      D_MsgLocalError("Invalid auth level type %d.\n", m_auth_level);
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

  M_PBufWriteUNum(pbuf, CHAT_CHANNEL_ALL);
  M_PBufWriteString(pbuf, message, strlen(message));
}

void N_PackRelayedChatMessage(netpeer_t *np, uint32_t sender_id,
                                             const char *message) {
  pbuf_t *pbuf = N_PeerBeginMessage(np, NM_CHAT_MESSAGE);

  M_PBufWriteUNum(pbuf, CHAT_CHANNEL_ALL);
  M_PBufWriteUNum(pbuf, sender_id);
  M_PBufWriteString(pbuf, message, strlen(message));
}

void N_PackTeamChatMessage(netpeer_t *np, const char *message) {
  pbuf_t *pbuf = N_PeerBeginMessage(np, NM_CHAT_MESSAGE);

  M_PBufWriteUNum(pbuf, CHAT_CHANNEL_TEAM);
  M_PBufWriteString(pbuf, message, strlen(message));
}

void N_PackRelayedTeamChatMessage(netpeer_t *np, uint32_t sender_id,
                                                 const char *message) {
  pbuf_t *pbuf = N_PeerBeginMessage(np, NM_CHAT_MESSAGE);

  M_PBufWriteUNum(pbuf, CHAT_CHANNEL_TEAM);
  M_PBufWriteUNum(pbuf, sender_id);
  M_PBufWriteString(pbuf, message, strlen(message));
}

void N_PackPlayerChatMessage(netpeer_t *np, uint32_t recipient_id,
                                            const char *message) {
  pbuf_t *pbuf = N_PeerBeginMessage(np, NM_CHAT_MESSAGE);

  M_PBufWriteUNum(pbuf, CHAT_CHANNEL_PEER);
  M_PBufWriteUNum(pbuf, recipient_id);
  M_PBufWriteString(pbuf, message, strlen(message));
}

void N_PackRelayedPlayerChatMessage(netpeer_t *np, uint32_t sender_id,
                                                   uint32_t recipient_id,
                                                   const char *message) {
  pbuf_t *pbuf = N_PeerBeginMessage(np, NM_CHAT_MESSAGE);

  M_PBufWriteUNum(pbuf, CHAT_CHANNEL_PEER);
  M_PBufWriteUNum(pbuf, sender_id);
  M_PBufWriteUNum(pbuf, recipient_id);
  M_PBufWriteString(pbuf, message, strlen(message));
}

void N_PackServerChatMessage(netpeer_t *np, const char *message) {
  pbuf_t *pbuf = N_PeerBeginMessage(np, NM_CHAT_MESSAGE);

  M_PBufWriteUNum(pbuf, CHAT_CHANNEL_SERVER);
  M_PBufWriteString(pbuf, message, strlen(message));
}

bool N_UnpackChatMessage(netpeer_t *np,
                         chat_channel_e *chat_channel,
                         uint32_t *sender_id,
                         uint32_t *recipient_id,
                         buf_t *message_contents) {
  pbuf_t *pbuf = N_PeerGetIncomingMessageData(np);
  uint8_t m_chat_channel;
  uint32_t m_sender_id = 0;
  uint32_t m_recipient_id = 0;

  read_ranged_uchar(pbuf, m_chat_channel, "chat channel",
    CHAT_CHANNEL_MIN, CHAT_CHANNEL_MAX
  );

  if ((CLIENT) && (m_chat_channel != CHAT_CHANNEL_SERVER)) {
    read_peer(pbuf, m_sender_id);
  }

  if (m_chat_channel == CHAT_CHANNEL_PEER) {
    read_peer(pbuf, m_message_recipient);
  }

  read_string(
    pbuf, message_contents, "chat message", MAX_CHAT_MESSAGE_SIZE
  );

  *chat_channel = m_chat_channel;

  if (CLIENT) {
    *message_sender = m_message_sender;
  }
  else {
    *message_sender = N_PeerGetID(np);
  }

  if (m_chat_channel == CHAT_CHANNEL_PEER) {
    *message_recipient = m_message_recipient;
  }

  return true;
}

void N_PackSync(netpeer_t *np) {
  pbuf_t *pbuf = N_PeerBeginMessage(np, NM_SYNC);

  if (CLIENT) {
    player_t *player = P_GetConsolePlayer();
    bool is_player = (player != NULL);

    M_PBufWriteNum(pbuf, N_PeerSyncGetTIC(np));
    M_PBufWriteBool(pbuf, is_player);

    if (is_player) {
      pack_unsynchronized_commands(pbuf, np, player);
      M_PBufWriteUNum(pbuf, P_PlayersGetCount() - 1);
    }
    else {
      M_PBufWriteUNum(pbuf, P_PlayersGetCount());
    }

    PLAYERS_FOR_EACH(iter) {
      if (is_player && iter.player == consoleplayer) {
        continue;
      }

      M_PBufWriteUNum(pbuf, iter.player->id);
      M_PBufWriteUNum(pbuf, N_PeerSyncGetCommandIndexForPlayer(np, i));
    }
  }
  else if (SERVER) {
    game_state_delta_t *delta = N_PeerSyncGetStateDelta(np);
    bool is_player = N_PeerGetStatus(np) == NETPEER_STATUS_PLAYER;
    size_t player_peer_count = 0;

    M_PBufWriteUNum(pbuf, N_PeerSyncGetCommandIndex(np));
    M_PBufWriteBool(pbuf, is_player);

    if (is_player) {
      pack_run_commands(pbuf, np);
    }

    NETPEER_FOR_EACH(iter) {
      if (N_PeerGetStatus(np) == NETPEER_STATUS_PLAYER) {
        player_peer_count++;
      }
    }

    M_PBufWriteUNum(pbuf, player_peer_count);

    NETPEER_FOR_EACH(iter) {
      if (N_PeerGetStatus(np) != NETPEER_STATUS_PLAYER) {
        continue;
      }

      if (iter.np == np) {
        continue;
      }

      pack_unsynchronized_commands(pbuf, np, iter.np);
    }

    N_MsgSyncLocalDebug("(%d) Syncing %d => %d (%d)\n",
      gametic,
      delta->from_tic,
      delta->to_tic,
      player->cmdq.latest_command_run_index
    );
    M_PBufWriteNum(pbuf, delta->from_tic);
    M_PBufWriteNum(pbuf, delta->to_tic);
    M_PBufWriteBytes(pbuf, delta->data.data, delta->data.size);
  }
}

bool N_UnpackSync(netpeer_t *np) {
  pbuf_t *pbuf = N_PeerGetIncomingMessageData(np);
  uint32_t m_command_index;

  if (CLIENT) {
    int32_t m_delta_from_tic;
    int32_t m_delta_to_tic;
    bool m_is_player;
    uint32_t m_peer_player_count;
    uint32_t m_command_count;

    read_uint(pbuf, m_command_index, "command index");
    read_bool(pbuf, m_is_player, "peer is player");
    read_uint(pbuf, m_peer_player_count, "peer player count");

    if (m_is_player) {
      read_uint(pbuf, m_command_count, "consoleplayer sync command count");

      for (uint32_t j = 0; j < m_command_count; j++) {
        uint32_t m_run_command_index;
        uint32_t m_server_tic;

        read_uint(pbuf, m_run_command_index, "run command index");
        read_uint(pbuf, m_server_tic, "server TIC");

        P_UpdateCommandServerTic(i, m_run_command_index, m_server_tic);
      }
    }

    for (size_t i = 0; i < m_peer_player_count; i++) {
      player_t *m_player;

      read_player(pbuf, m_player);
      read_uint(pbuf, m_command_count, "command count");

      for (size_t j = 0; j < m_command_count; j++) {
        idxticcmd_t m_icmd;

        unpack_net_command(pbuf, m_icmd);

        PL_QueueCommand(m_player, &m_icmd);
      }

      for (size_t ci = 0; ci < PL_GetCommandCount(m_player); ci++) {
        idxticcmd_t *m_icmd = PL_GetCommand(m_player, ci);

        if (!m_icmd) {
          continue;
        }

        if (m_icmd->server_tic == 0) {
          continue;
        }

        if (m_icmd->index <= m_command_index) {
          continue;
        }

        m_command_index = m_icmd->index;
      }

      if (m_command_index > N_PeerSyncGetCommandIndexForPlayer(np, i)) {
        I_Error("Bad server sync command index for %zu: %u > %u",
          i, m_command_index, N_PeerSyncGetCommandIndexForPlayer(np, i)
        );
      }

      N_PeerSyncUpdateCommandIndexForPlayer(np, m_player, m_command_index);
    }

    read_int(pbuf, m_delta_from_tic, "delta from tic");
    read_int(pbuf, m_delta_to_tic,   "delta to tic");

    /* [CG] pbuf now points to the delta's binary data */
    N_PeerSyncUpdateStateDelta(np, m_delta_from_tic, m_delta_to_tic, pbuf);
  }
  else if (SERVER) {
    int32_t m_sync_tic;
    uint32_t m_command_count;
    bool m_is_player;

    read_int(pbuf, m_sync_tic, "sync tic");
    read_bool(pbuf, m_is_player, "peer is player");

    if (m_is_player) {
      if (N_PeerGetStatus(np) != NETPEER_STATUS_PLAYER) {
        D_MsgLocalError(
          "N_UnpackSync: Receiving commands from non-player peer %u\n",
          N_PeerGetID(np)
        );

        /* [CG] [FIXME] Should maybe read/ignore these instead of bailing? */

        return false;
      }

      read_uint(pbuf, m_command_count, "command count");

      for (size_t i = 0; i < m_command_count; i++) {
        idxticcmd_t m_icmd;

        unpack_net_command(pbuf, m_icmd);

        m_icmd.server_tic = 0;

        P_QueuePlayerCommand(playernum, &m_icmd);
      }

      if (m_command_count > 0) {
        m_command_index = P_GetLatestCommandIndex(player);

        N_PeerSyncUpdateCommandIndexForPlayer(np, player, m_command_index);
      }

      read_uint(pbuf, m_player_count, "player count");

      for (size_t i = 0; i < m_player_count; i++) {
        player_t *m_player;

        read_player(pbuf, m_player);
        read_unum(pbuf, m_command_index, "command index");
      }
    }

    N_PeerSyncUpdateTIC(np, m_sync_tic);

    for (size_t i = 0; i < PLAYER_CAP; i++) {
      if (i == playernum) {
        continue;
      }

      if (!bitmap_get_bit(bitmap, i)) {
        continue;
      }

      read_uint(pbuf, m_command_index, "command index");

      N_PeerSyncUpdateCommandIndexForPlayer(np, i, m_command_index);
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

  M_PBufWriteUNum(pbuf, new_team);
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

  M_PBufWriteUNum(pbuf, 0); /* CG: TODO */
}

bool N_UnpackPWOChange(netpeer_t *np) {
  return false; /* CG: TODO */
}

void N_PackWSOPChange(netpeer_t *np, unsigned short playernum,
                                     unsigned char new_wsop_flags) {
  pack_player_preference_change(pbuf, gametic, playernum, "wsop", 4);

  M_PBufWriteUChar(pbuf, new_wsop_flags);
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

  M_PBufWriteNum(pbuf, new_color_index);
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
}

bool N_UnpackSkinChange(netpeer_t *np, unsigned short *playernum) {
  return false; /* CG: TODO */
}

void N_PackAuthRequest(netpeer_t *np, const char *password) {
  pbuf_t *pbuf = N_PeerBeginMessage(np, NM_AUTH);

  M_PBufWriteString(pbuf, password, strlen(password));
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
}

bool N_UnpackRCONCommand(netpeer_t *np, buf_t *buf) {
  pbuf_t *pbuf = N_PeerGetIncomingMessageData(np);

  read_string(pbuf, buf, "RCON command", MAX_COMMAND_LENGTH);

  return true;
}

void N_PackVoteRequest(netpeer_t *np, const char *command) {
  pbuf_t *pbuf = N_PeerBeginMessage(np, NM_VOTE_REQUEST);

  M_PBufWriteString(pbuf, command, strlen(command));
}

bool N_UnpackVoteRequest(netpeer_t *np, buf_t *buf) {
  pbuf_t *pbuf = N_PeerGetIncomingMessageData(np);

  read_string(pbuf, buf, "vote command", MAX_COMMAND_LENGTH);

  return true;
}

void N_PackGameActionChange(netpeer_t *np) {
  pbuf_t *pbuf = N_PeerBeginMessage(np, NM_GAME_ACTION);

  M_PBufWriteNum(pbuf, G_GetGameAction());
  M_PBufWriteNum(pbuf, gametic);
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

