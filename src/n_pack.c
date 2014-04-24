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

#include "d_ticcmd.h"
#include "doomstat.h"
#include "g_game.h"
#include "lprintf.h"
#include "m_pbuf.h"
#include "p_pspr.h"
#include "p_user.h"
#include "w_wad.h"

#include "n_net.h"
#include "n_buf.h"
#include "n_main.h"
#include "n_state.h"
#include "n_peer.h"
#include "n_proto.h"

/* CG: FIXME: Most of these should be more than just defines tucked here */
#define MAX_RESOURCE_NAMES 1000
#define MAX_RESOURCE_NAME_LENGTH 128
#define MAX_SERVER_MESSAGE_SIZE 256
#define MAX_PLAYER_MESSAGE_SIZE 256
#define MAX_PLAYER_PREFERENCE_NAME_SIZE 32
#define MAX_PLAYER_NAME_SIZE 32
#define MAX_PASSWORD_LENGTH 256
#define MAX_COMMAND_LENGTH 32

#define check_range(x, min, max)                                              \
  if (!(min <= x <= max)) {                                                   \
    doom_printf("%s: Invalid message: %s is out of range (%s, %s)\n",         \
      __func__, #x, #min, #max                                                \
    );                                                                        \
  }

#define read_char(pbuf, var, name)                                            \
  if (!M_PBufReadChar(pbuf, &var)) {                                          \
    doom_printf("%s: Error reading %s.\n", __func__, name);                   \
    return false;                                                             \
  }

#define read_ranged_char(pbuf, var, name, min, max)                           \
  read_char(pbuf, var, name);                                                 \
  check_range(var, min, max);

#define read_uchar(pbuf, var, name)                                           \
  if (!M_PBufReadUChar(pbuf, &var)) {                                         \
    doom_printf("%s: Error reading %s.\n", __func__, name);                   \
    return false;                                                             \
  }

#define read_ranged_uchar(pbuf, var, name, min, max)                          \
  read_uchar(pbuf, var, name);                                                \
  check_range(var, min, max);

#define read_short(pbuf, var, name)                                           \
  if (!M_PBufReadShort(pbuf, &var)) {                                         \
    doom_printf("%s: Error reading %s.\n", __func__, name);                   \
    return false;                                                             \
  }

#define read_ranged_short(pbuf, var, name, min, max)                          \
  read_short(pbuf, var, name);                                                \
  check_range(var, min, max);

#define read_ushort(pbuf, var, name)                                          \
  if (!M_PBufReadUShort(pbuf, &var)) {                                        \
    doom_printf("%s: Error reading %s.\n", __func__, name);                   \
    return false;                                                             \
  }

#define read_ranged_ushort(pbuf, var, name, min, max)                         \
  read_ushort(pbuf, var, name);                                               \
  check_range(var, min, max);

#define read_int(pbuf, var, name)                                             \
  if (!M_PBufReadInt(pbuf, &var)) {                                           \
    doom_printf("%s: Error reading %s.\n", __func__, name);                   \
    return false;                                                             \
  }

#define read_ranged_int(pbuf, var, name, min, max)                            \
  read_int(pbuf, var, name);                                                  \
  check_range(var, min, max);

#define read_uint(pbuf, var, name)                                            \
  if (!M_PBufReadUInt(pbuf, &var)) {                                          \
    doom_printf("%s: Error reading %s.\n", __func__, name);                   \
    return false;                                                             \
  }

#define read_ranged_uint(pbuf, var, name, min, max)                           \
  read_uint(pbuf, var, name);                                                 \
  check_range(var, min, max);

#define read_long(pbuf, var, name)                                            \
  if (!M_PBufReadLong(pbuf, &var)) {                                          \
    doom_printf("%s: Error reading %s.\n", __func__, name);                   \
    return false;                                                             \
  }

#define read_ranged_long(pbuf, var, name, min, max)                           \
  read_long(pbuf, var, name);                                                 \
  check_range(var, min, max);

#define read_ulong(pbuf, var, name)                                           \
  if (!M_PBufReadULong(pbuf, &var)) {                                         \
    doom_printf("%s: Error reading %s.\n", __func__, name);                   \
    return false;                                                             \
  }

#define read_ranged_ulong(pbuf, var, name, min, max)                          \
  read_ulong(pbuf, var, name);                                                \
  check_range(var, min, max);

#define read_double(pbuf, var, name)                                          \
  if (!M_PBufReadDouble(pbuf, &var)) {                                        \
    doom_printf("%s: Error reading %s.\n", __func__, name);                   \
    return false;                                                             \
  }

#define read_ranged_double(pbuf, var, name, min, max)                         \
  read_double(pbuf, var, name);                                               \
  check_range(var, min, max);

#define read_bool(pbuf, var, name)                                            \
  if (!M_PBufReadBool(pbuf, &var)) {                                          \
    doom_printf("%s: Error reading %s.\n", __func__, name);                   \
    return false;                                                             \
  }

#define read_array(pbuf, var, name)                                           \
  if (!M_PBufReadArray(pbuf, &var)) {                                         \
    doom_printf("%s: Error reading %s.\n", __func__, name);                   \
    return false;                                                             \
  }

#define read_map(pbuf, var, name)                                             \
  if (!M_PBufReadMap(pbuf, &var)) {                                           \
    doom_printf("%s: Error reading %s.\n", __func__, name);                   \
    return false;                                                             \
  }

#define read_bytes(pbuf, var, name)                                           \
  if (!M_PBufReadBytes(pbuf, &var)) {                                         \
    doom_printf("%s: Error reading %s.\n", __func__, name);                   \
    return false;                                                             \
  }

#define read_string(pbuf, var, name, sz)                                      \
  if (!M_PBufReadString(pbuf, var, sz)) {                                    \
    doom_printf("%s: Error reading %s.\n", __func__, name);                   \
    return false;                                                             \
  }

#define read_string_array(pbuf, var, name, count, length)                     \
  if (!M_PBufReadStringArray(pbuf, var, count, length)) {                    \
    doom_printf("%s: Error reading %s.\n", __func__, name);                   \
    return false;                                                             \
  }

#define read_recipient_array(pbuf, var, name, count)                          \
  if (!M_PBufReadShortArray(pbuf, var, count)) {                             \
    doom_printf("%s: Error reading %s.\n", __func__, name);                   \
    return false;                                                             \
  }

#define read_player(pbuf, var)                                                \
  if (!M_PBufReadShort(pbuf, &var)) {                                         \
    doom_printf("%s: Error reading player number.\n", __func__);              \
    return false;                                                             \
  }                                                                           \
  if (var >= MAXPLAYERS) {                                                    \
    doom_printf("%s: Invalid player number %d.\n", __func__, var);            \
    return false;                                                             \
  }

#define read_message_recipient(pbuf, var)                                     \
  if (!M_PBufReadShort(pbuf, &var)) {                                         \
    doom_printf("%s: Error reading recipient number.\n", __func__);           \
    return false;                                                             \
  }                                                                           \
  if (var != -1 && var >= MAXPLAYERS) {                                       \
    doom_printf("%s: Invalid recipient number %d.\n", __func__, var);         \
    return false;                                                             \
  }

#define pack_player_preference_change(pbuf, gametic, playernum, pn, pnsz)     \
  pbuf_t *pbuf = N_NBufBeginMessage(                                          \
    &np->netbuf, NET_CHANNEL_RELIABLE, nm_playerpreferencechange              \
  );                                                                          \
  M_PBufWriteInt(pbuf, gametic);                                              \
  M_PBufWriteShort(pbuf, playernum);                                          \
  M_PBufWriteMap(pbuf, 2);                                                    \
  M_PBufWriteBytes(pbuf, pn, pnsz)

static void pack_commands(pbuf_t *pbuf, netpeer_t *np, short playernum) {
  netticcmd_t *n = NULL;
  byte command_count = 0;
  cbuf_t *commands = NULL;

  M_PBufWriteShort(pbuf, playernum);

  commands = P_GetPlayerCommands(playernum);

  CBUF_FOR_EACH(commands, entry) {
    netticcmd_t *ncmd = (netticcmd_t *)entry.obj;

    if (ncmd->tic > np->command_tic)
      command_count++;

  }

  M_PBufWriteUChar(pbuf, command_count);

  if (command_count == 0) {
    printf("[...]\n");
    return;
  }

  CBUF_FOR_EACH(commands, entry) {
    netticcmd_t *ncmd = (netticcmd_t *)entry.obj;

    if (ncmd->tic <= np->command_tic)
      continue;

    if (n == NULL)
      printf("[%d => ", ncmd->tic);

    n = ncmd;

    M_PBufWriteInt(pbuf, ncmd->tic);
    M_PBufWriteChar(pbuf, ncmd->cmd.forwardmove);
    M_PBufWriteChar(pbuf, ncmd->cmd.sidemove);
    M_PBufWriteShort(pbuf, ncmd->cmd.angleturn);
    M_PBufWriteShort(pbuf, ncmd->cmd.consistancy);
    M_PBufWriteUChar(pbuf, ncmd->cmd.chatchar);
    M_PBufWriteUChar(pbuf, ncmd->cmd.buttons);
  }

  if (n != NULL)
    printf("%d]\n", n->tic);
}

void N_PackSetup(netpeer_t *np) {
  game_state_t *gs = N_GetLatestState();
  unsigned short player_count = 0;
  pbuf_t *pbuf = NULL;

  printf("Packing setup (%d)\n", gametic);

  for (int i = 0; i < MAXPLAYERS; i++) {
    if (playeringame[i]) {
      player_count++;
    }
  }

  pbuf = N_NBufBeginMessage(&np->netbuf, NET_CHANNEL_RELIABLE, nm_setup);
  M_PBufWriteInt(pbuf, netsync);
  M_PBufWriteUShort(pbuf, player_count);
  M_PBufWriteShort(pbuf, np->playernum);
  M_PBufWriteStringArray(pbuf, &resource_files_buf);
  M_PBufWriteStringArray(pbuf, &deh_files_buf);
  M_PBufWriteUInt(pbuf, gs->tic);
  M_PBufWriteBytes(pbuf, gs->data.data, gs->data.size);

  printf(
    "N_PackSetup: Sent game state at %d (player count: %d).\n",
    gs->tic, player_count
  );
}

dboolean N_UnpackSetup(netpeer_t *np, net_sync_type_e *sync_type,
                                      unsigned short *player_count,
                                      short *playernum) {
  pbuf_t *pbuf = &np->netbuf.incoming.messages;
  int m_sync_type = 0;
  unsigned short m_player_count = 0;
  short m_playernum = 0;
  game_state_t *gs = NULL;
  
  read_ranged_int(
    pbuf, m_sync_type, "netsync", NET_SYNC_TYPE_COMMAND, NET_SYNC_TYPE_DELTA
  );
  read_ushort(pbuf, m_player_count, "player count");
  read_short(pbuf, m_playernum, "consoleplayer");
  read_string_array(
    pbuf,
    &resource_files_buf,
    "resource names",
    MAX_RESOURCE_NAMES,
    MAX_RESOURCE_NAME_LENGTH
  );
  read_string_array(
    pbuf,
    &deh_files_buf,
    "DeHackEd/BEX names",
    MAX_RESOURCE_NAMES,
    MAX_RESOURCE_NAME_LENGTH
  );

  gs = N_GetNewState();

  read_int(pbuf, gs->tic, "game state tic");
  read_bytes(pbuf, gs->data, "game state data");

  switch (m_sync_type) {
    case NET_SYNC_TYPE_COMMAND:
      *sync_type = NET_SYNC_TYPE_COMMAND;
    break;
    case NET_SYNC_TYPE_DELTA:
      *sync_type = NET_SYNC_TYPE_DELTA;
    break;
    default:
      doom_printf("Invalid sync type %d.\n", m_sync_type);
      return false;
    break;
  }
  *player_count = m_player_count;
  *playernum = m_playernum;

  N_SetLatestState(gs);

  return true;
}

void N_PackAuthResponse(netpeer_t *np, auth_level_e auth_level) {
  pbuf_t *pbuf = N_NBufBeginMessage(
    &np->netbuf, NET_CHANNEL_RELIABLE, nm_authresponse
  );
  printf("Packing auth response\n");

  M_PBufWriteUChar(pbuf, auth_level);
}

dboolean N_UnpackAuthResponse(netpeer_t *np, auth_level_e *auth_level) {
  pbuf_t *pbuf = &np->netbuf.incoming.messages;
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
      doom_printf("Invalid auth level type %d.\n", m_auth_level);
      return false;
    break;
  }

  return true;
}

void N_PackServerMessage(netpeer_t *np, char *message) {
  pbuf_t *pbuf = N_NBufBeginMessage(
    &np->netbuf, NET_CHANNEL_RELIABLE, nm_servermessage
  );

  printf("Packing server message\n");
  M_PBufWriteString(pbuf, message, strlen(message));
}

dboolean N_UnpackServerMessage(netpeer_t *np, buf_t *buf) {
  pbuf_t *pbuf = &np->netbuf.incoming.messages;

  read_string(pbuf, buf, "server message content", MAX_SERVER_MESSAGE_SIZE);

  return true;
}

void N_PackSync(netpeer_t *np) {
  pbuf_t *pbuf = N_NBufBeginMessage(
    &np->netbuf, NET_CHANNEL_UNRELIABLE, nm_sync
  );
  unsigned short player_count = 0;


  printf("(%d) Sending sync: ST/CT: (%d/%d) ",
    gametic, np->state_tic, np->command_tic
  );

  M_PBufWriteInt(pbuf, np->state_tic);

  if (SERVER) {
    for (int i = 0; i < N_GetPeerCount(); i++) {
      if (N_GetPeer(i) != NULL) {
        player_count++;
      }
    }
  }
  else {
    player_count = 1;
  }

  M_PBufWriteUShort(pbuf, player_count);

  if (SERVER) {
    for (int i = 0; i < N_GetPeerCount(); i++) {
      netpeer_t *snp = N_GetPeer(i);

      if (snp != NULL)
        pack_commands(pbuf, np, snp->playernum);
    }
  }
  else {
    pack_commands(pbuf, np, consoleplayer);
  }
}

dboolean N_UnpackSync(netpeer_t *np, dboolean *update_sync) {
  pbuf_t *pbuf = &np->netbuf.incoming.messages;
  unsigned short m_player_count = 0;
  int m_state_tic = -1;
  int m_command_tic = np->command_tic;
  dboolean m_update_sync = false;

  *update_sync = false;

  printf("(%d) Received sync ", gametic);

  read_int(pbuf, m_state_tic, "state_tic");

  if ((np->state_tic != m_state_tic))
    m_update_sync = true;

  printf("ST/CT: (%d/%d) ", m_state_tic, m_command_tic);

  read_ushort(pbuf, m_player_count, "player count");

  for (int i = 0; i < m_player_count; i++) {
    short m_playernum = 0;
    byte command_count = 0;
    cbuf_t *commands = NULL;

    read_player(pbuf, m_playernum);

    if (SERVER && np->playernum != m_playernum) {
      printf(
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
   *           (or something), that may be preferable to a static limit... but
   *           I think having an upper bound on that setting is still prudent.
   */
    read_uchar(pbuf, command_count, "command count");

    commands = &players[m_playernum].commands;

    M_CBufEnsureCapacity(commands, command_count);

    netticcmd_t *n = NULL;

    printf("Unpacking %d commands.\n", command_count);

    /*
     * CG: It seems like this allows holes in the command buffer, i.e.
     *     [67, 68, 70, 71, 73, 76].  This is true, but such holes do not
     *     result from packet loss, rather they result from CPU lag on the
     *     client; it simply didn't make a command fast enough and dropped the
     *     tic.  This is surprisingly normal, so command buffers won't be
     *     contiguous.
     */

    while (command_count--) {
      int command_tic = -1;

      read_int(pbuf, command_tic, "command tic");
      
      if (m_command_tic == 0)
        m_command_tic = command_tic - 1;

      if (command_tic > m_command_tic) {
        netticcmd_t *ncmd = M_CBufGetFirstFreeOrNewSlot(commands);

        m_command_tic = command_tic;

        ncmd->tic = command_tic;

        if (n == NULL)
          printf(" [%d => ", ncmd->tic);

        n = ncmd;

        read_char(pbuf, ncmd->cmd.forwardmove, "command forward value");
        read_char(pbuf, ncmd->cmd.sidemove, "command side value");
        read_short(pbuf, ncmd->cmd.angleturn, "command angle value");
        read_short(pbuf, ncmd->cmd.consistancy, "command consistancy value");
        read_uchar(pbuf, ncmd->cmd.chatchar, "comand chatchar value");
        read_uchar(pbuf, ncmd->cmd.buttons, "command buttons value");

        printf("CMD: {%d, %d, %d, %d, %d, %u, %u}\n",
          ncmd->tic,
          ncmd->cmd.forwardmove,
          ncmd->cmd.sidemove,
          ncmd->cmd.angleturn,
          ncmd->cmd.consistancy,
          ncmd->cmd.chatchar,
          ncmd->cmd.buttons
        );
      }
      else {
        ticcmd_t cmd;

        // printf("Skipping %d (< %d)\n", command_tic, m_command_tic + 1);
        read_char(pbuf, cmd.forwardmove, "command forward value");
        read_char(pbuf, cmd.sidemove, "command side value");
        read_short(pbuf, cmd.angleturn, "command angle value");
        read_short(pbuf, cmd.consistancy, "command consistancy value");
        read_uchar(pbuf, cmd.chatchar, "comand chatchar value");
        read_uchar(pbuf, cmd.buttons, "command buttons value");
      }
    }

    if (n != NULL)
      printf("%d]\n", n->tic);
    else
      printf("[...]\n");

    if (n != NULL) {
      printf("Commands after sync: ");
      N_PrintPlayerCommands(commands);
    }
  }

  if (np->command_tic != m_command_tic)
    m_update_sync = true;

  if (m_update_sync) {
    np->state_tic = m_state_tic;
    np->command_tic = m_command_tic;
    *update_sync = m_update_sync;
  }

  return true;
}

void N_PackDeltaSync(netpeer_t *np) {
  pbuf_t *pbuf = N_NBufBeginMessage(
    &np->netbuf, NET_CHANNEL_UNRELIABLE, nm_sync
  );

  printf("(%d) Sending sync: ST/CT: (%d/%d) Delta: [%d => %d]\n",
    gametic,
    np->state_tic,
    np->command_tic,
    np->delta.from_tic,
    np->delta.to_tic
  );

  M_PBufWriteInt(pbuf, np->state_tic);
  M_PBufWriteInt(pbuf, np->command_tic);
  M_PBufWriteInt(pbuf, np->delta.from_tic);
  M_PBufWriteInt(pbuf, np->delta.to_tic);
  M_PBufWriteBytes(pbuf, np->delta.data.data, np->delta.data.size);
}

dboolean N_UnpackDeltaSync(netpeer_t *np) {
  pbuf_t *pbuf = &np->netbuf.incoming.messages;
  int m_state_tic = 0;
  int m_command_tic = 0;
  int m_delta_from_tic = 0;
  int m_delta_to_tic = 0;

  read_int(pbuf, m_state_tic, "state tic");
  read_int(pbuf, m_command_tic, "command tic");
  read_int(pbuf, m_delta_from_tic, "delta from tic");
  read_int(pbuf, m_delta_to_tic, "delta to tic");

  /*
  printf("(%d) Received sync: ST/CT: (%d/%d) Delta: [%d => %d] (%d).\n",
    gametic,
    m_state_tic,
    m_command_tic,
    m_delta_from_tic,
    m_delta_to_tic,
    np->state_tic < m_delta_to_tic
  );
  */

  if (np->state_tic >= m_delta_to_tic)
    return false;

  np->state_tic = m_state_tic;
  np->command_tic = m_command_tic;
  np->delta.from_tic = m_delta_from_tic;
  np->delta.to_tic = m_delta_to_tic;
  read_bytes(pbuf, np->delta.data, "delta data");

  return true;
}

void N_PackPlayerMessage(netpeer_t *np, short sender, buf_t *recipients,
                                        char *message) {
  pbuf_t *pbuf = N_NBufBeginMessage(
    &np->netbuf, NET_CHANNEL_RELIABLE, nm_playermessage
  );

  M_PBufWriteUShort(pbuf, sender);
  M_PBufWriteShortArray(pbuf, recipients);
  M_PBufWriteString(pbuf, message, strlen(message));

  printf("Packed player message\n");
}

dboolean N_UnpackPlayerMessage(netpeer_t *np, short *sender, buf_t *recipients,
                                              buf_t *buf) {
  pbuf_t *pbuf = &np->netbuf.incoming.messages;
  short m_sender = 0;

  read_player(pbuf, m_sender);
  read_recipient_array(pbuf, recipients, "message recipients", MAXPLAYERS + 1);
  read_string(pbuf, buf, "player message content", MAX_PLAYER_MESSAGE_SIZE);

  *sender = m_sender;

  return true;
}

dboolean N_UnpackPlayerPreferenceChange(netpeer_t *np, short *playernum,
                                                       int *tic,
                                                       unsigned int *count) {
  pbuf_t *pbuf = &np->netbuf.incoming.messages;
  short m_playernum = 0;
  int m_tic = 0;
  unsigned int m_count = 0;

  read_player(pbuf, m_playernum);
  read_int(pbuf, m_tic, "player preference change tic");
  read_map(pbuf, m_count, "player preference change count");

  *playernum = m_playernum;
  *tic = m_tic;
  *count = m_count;

  return true;
}

dboolean N_UnpackPlayerPreferenceName(netpeer_t *np, buf_t *buf) {
  pbuf_t *pbuf = &np->netbuf.incoming.messages;
  
  read_string(
    pbuf, buf, "player preference name", MAX_PLAYER_PREFERENCE_NAME_SIZE
  );

  return true;
}

void N_PackNameChange(netpeer_t *np, short playernum, char *new_name) {
  pack_player_preference_change(pbuf, gametic, playernum, "name", 4);

  M_PBufWriteShort(pbuf, playernum);
  M_PBufWriteString(pbuf, new_name, strlen(new_name));

  printf("Packed name change\n");
}

dboolean N_UnpackNameChange(netpeer_t *np, short *playernum, buf_t *buf) {
  pbuf_t *pbuf = &np->netbuf.incoming.messages;
  short m_playernum = 0;

  read_player(pbuf, m_playernum);
  read_string(pbuf, buf, "new name", MAX_PLAYER_NAME_SIZE);

  *playernum = m_playernum;

  return true;
}

void N_PackTeamChange(netpeer_t *np, short playernum, byte new_team) {
  pack_player_preference_change(pbuf, gametic, playernum, "team", 4);

  M_PBufWriteShort(pbuf, playernum);
  M_PBufWriteUChar(pbuf, new_team);

  printf("Packed team change\n");
}

dboolean N_UnpackTeamChange(netpeer_t *np, short *playernum, byte *new_team) {
  pbuf_t *pbuf = &np->netbuf.incoming.messages;
  int team_count = 0;
  short m_playernum = 0;
  byte m_new_team = 0;

  read_player(pbuf, m_playernum);
  if (team_count > 0) { /* CG: TODO: teams */
    read_ranged_uchar(pbuf, m_new_team, "new team index", 0, team_count - 1);
  }

  *new_team = m_new_team;
  *playernum = m_playernum;

  return true;
}

void N_PackPWOChange(netpeer_t *np, short playernum) {
  pack_player_preference_change(pbuf, gametic, playernum, "pwo", 3);

  M_PBufWriteShort(pbuf, playernum);
  M_PBufWriteUChar(pbuf, 0); /* CG: TODO */

  printf("Packed PWO change\n");
}

dboolean N_UnpackPWOChange(netpeer_t *np, short *playernum) {
  return false; /* CG: TODO */
}

void N_PackWSOPChange(netpeer_t *np, short playernum, byte new_wsop_flags) {
  pack_player_preference_change(pbuf, gametic, playernum, "wsop", 4);

  M_PBufWriteShort(pbuf, playernum);
  M_PBufWriteUChar(pbuf, new_wsop_flags);

  printf("Packed WSOP change\n");
}

dboolean N_UnpackWSOPChange(netpeer_t *np, short *playernum,
                                           byte *new_wsop_flags) {
  pbuf_t *pbuf = &np->netbuf.incoming.messages;
  short m_playernum = 0;
  byte m_new_wsop_flags = 0;

  read_player(pbuf, m_playernum);
  read_ranged_uchar(
    pbuf, m_new_wsop_flags, "new WSOP flags", WSOP_NONE, WSOP_MAX - 1
  );

  *playernum = m_playernum;
  *new_wsop_flags = m_new_wsop_flags;

  return true;
}

void N_PackBobbingChange(netpeer_t *np, short playernum,
                                        double new_bobbing_amount) {
  pack_player_preference_change(pbuf, gametic, playernum, "bobbing", 7);

  M_PBufWriteShort(pbuf, playernum);
  M_PBufWriteDouble(pbuf, new_bobbing_amount);

  printf("Packed bobbing change\n");
}

dboolean N_UnpackBobbingChanged(netpeer_t *np, short *playernum,
                                               double *new_bobbing_amount) {
  pbuf_t *pbuf = &np->netbuf.incoming.messages;
  short m_playernum = 0;
  double m_new_bobbing_amount = 0;

  read_player(pbuf, m_playernum);
  read_ranged_double(
    pbuf, m_new_bobbing_amount, "new bobbing amount", 0.0, 1.0
  );

  *playernum = m_playernum;
  *new_bobbing_amount = m_new_bobbing_amount;

  return true;
}

void N_PackAutoaimChange(netpeer_t *np, short playernum,
                                        dboolean new_autoaim_enabled) {
  pack_player_preference_change(pbuf, gametic, playernum, "autoaim", 7);

  M_PBufWriteShort(pbuf, playernum);
  M_PBufWriteBool(pbuf, new_autoaim_enabled);

  printf("Packed autoaim change\n");
}

dboolean N_UnpackAutoaimChange(netpeer_t *np, short *playernum,
                                              dboolean *new_autoaim_enabled) {
  pbuf_t *pbuf = &np->netbuf.incoming.messages;
  short m_playernum = 0;
  dboolean m_new_autoaim_enabled = false;

  read_player(pbuf, m_playernum);
  read_bool(pbuf, m_new_autoaim_enabled, "new autoaim enabled value");

  *playernum = m_playernum;
  *new_autoaim_enabled = m_new_autoaim_enabled;

  return true;
}

void N_PackWeaponSpeedChange(netpeer_t *np, short playernum,
                                            byte new_weapon_speed) {
  pack_player_preference_change(pbuf, gametic, playernum, "weapon speed", 12);

  M_PBufWriteShort(pbuf, playernum);
  M_PBufWriteUChar(pbuf, new_weapon_speed);

  printf("Packed weapon speed change\n");
}

dboolean N_UnpackWeaponSpeedChange(netpeer_t *np, short *playernum,
                                                  byte *new_weapon_speed) {
  pbuf_t *pbuf = &np->netbuf.incoming.messages;
  short m_playernum = 0;
  byte m_new_weapon_speed = 0;

  read_player(pbuf, m_playernum);
  read_uchar(pbuf, m_new_weapon_speed, "new weapon speed");

  *playernum = m_playernum;
  *new_weapon_speed = m_new_weapon_speed;

  return true;
}

void N_PackColorChange(netpeer_t *np, short playernum, byte new_red,
                                                       byte new_green,
                                                       byte new_blue) {
  pack_player_preference_change(pbuf, gametic, playernum, "color", 5);

  M_PBufWriteShort(pbuf, playernum);
  M_PBufWriteUInt(pbuf, (new_red << 24) | (new_green << 16) | (new_blue << 8));

  printf("Packed color change\n");
}

dboolean N_UnpackColorChange(netpeer_t *np, short *playernum, byte *new_red,
                                                              byte *new_green,
                                                              byte *new_blue) {
  pbuf_t *pbuf = &np->netbuf.incoming.messages;
  short m_playernum = 0;
  unsigned int m_new_color = 0;

  read_player(pbuf, m_playernum);
  read_uint(pbuf, m_new_color, "new color");

  *playernum = m_playernum;
  *new_red   = (m_new_color >> 24) & 0xFF;
  *new_green = (m_new_color >> 16) & 0xFF;
  *new_blue  = (m_new_color >>  8) & 0xFF;

  return true;
}

void N_PackColorIndexChange(netpeer_t *np, short playernum,
                                           int new_color_index) {
  pack_player_preference_change(pbuf, gametic, playernum, "color index", 11);

  M_PBufWriteShort(pbuf, playernum);
  M_PBufWriteInt(pbuf, new_color_index);

  printf("Packed color index change\n");
}

dboolean N_UnpackColorIndexChange(netpeer_t *np, short *playernum,
                                                 int *new_color_index) {
  pbuf_t *pbuf = &np->netbuf.incoming.messages;
  short m_playernum = 0;
  int m_new_color_index = 0;

  read_player(pbuf, m_playernum);
  /* CG: TODO: Ensure new color map index is reasonable */
  read_int(pbuf, m_new_color_index, "new color index");

  *playernum = m_playernum;
  *new_color_index = m_new_color_index;

  return true;
}

void N_PackSkinChange(netpeer_t *np, short playernum) {
  pack_player_preference_change(pbuf, gametic, playernum, "skin name", 9);

  M_PBufWriteShort(pbuf, playernum);
  M_PBufWriteUChar(pbuf, 0); /* CG: TODO */

  printf("Packed skin change\n");
}

dboolean N_UnpackSkinChange(netpeer_t *np, short *playernum) {
  return false; /* CG: TODO */
}

void N_PackAuthRequest(netpeer_t *np, char *password) {
  pbuf_t *pbuf = N_NBufBeginMessage(
    &np->netbuf, NET_CHANNEL_RELIABLE, nm_authrequest
  );

  M_PBufWriteString(pbuf, password, strlen(password));

  printf("Packed auth request\n");
}

dboolean N_UnpackAuthRequest(netpeer_t *np, buf_t *buf) {
  pbuf_t *pbuf = &np->netbuf.incoming.messages;

  read_string(
    pbuf, buf, "authorization request password", MAX_PASSWORD_LENGTH
  );

  return true;
}

void N_PackRCONCommand(netpeer_t *np, char *command) {
  pbuf_t *pbuf = N_NBufBeginMessage(
    &np->netbuf, NET_CHANNEL_RELIABLE, nm_rconcommand
  );

  M_PBufWriteString(pbuf, command, strlen(command));

  printf("Packed RCON command\n");
}

dboolean N_UnpackRCONCommand(netpeer_t *np, buf_t *buf) {
  pbuf_t *pbuf = &np->netbuf.incoming.messages;

  read_string(pbuf, buf, "RCON command", MAX_COMMAND_LENGTH);

  return true;
}

void N_PackVoteRequest(netpeer_t *np, char *command) {
  pbuf_t *pbuf = N_NBufBeginMessage(
    &np->netbuf, NET_CHANNEL_RELIABLE, nm_voterequest
  );

  M_PBufWriteString(pbuf, command, strlen(command));

  printf("Packed vote request\n");
}

dboolean N_UnpackVoteRequest(netpeer_t *np, buf_t *buf) {
  pbuf_t *pbuf = &np->netbuf.incoming.messages;

  read_string(pbuf, buf, "vote command", MAX_COMMAND_LENGTH);

  return true;
}

/* vi: set et ts=2 sw=2: */

