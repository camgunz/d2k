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

static void destroy_message(gpointer data) {
  player_message_t *msg = data;

  free(msg->content);
  free(msg);
}

static void add_player_vmessage(int pn, bool is_markup, bool centered, int sfx,
                                const char *fmt, va_list args) {
  gchar *gcontent;
  player_message_t *msg;

  if (CL_Synchronizing() || CL_RePredicting())
    return;

  if (strlen(fmt) <= 0)
    return;

  msg = malloc(sizeof(player_message_t));

  if (msg == NULL)
    I_Error("P_Printf: malloc failed");

  gcontent = g_strdup_vprintf(fmt, args);

  if (pn == consoleplayer)
    D_Msg(MSG_GAME, "%s", gcontent);

#if 0
  if (!is_markup) {
    gchar *escaped_gcontent = g_markup_escape_text(gcontent, -1);

    g_free(gcontent);
    gcontent = escaped_gcontent;
  }
#endif

  msg->content = strdup(gcontent);
  msg->centered = centered;
  msg->processed = false;
  msg->sfx = sfx;

  g_free(gcontent);

  P_AddMessage(pn, msg);
}

static void add_player_message(int pn, bool is_markup, bool centered, int sfx,
                               const char *fmt, ...) {
  va_list args;

  va_start(args, fmt);
  add_player_vmessage(pn, is_markup, centered, sfx, fmt, args);
  va_end(args);
}

void PL_InitMessaging(player_t *player) {
  player->messages.message = g_ptr_array_new_with_free_func(destroy_message);
  player->messages.updated = false;
}

void PL_AddMessage(player_t *player, player_message_t *message) {
  g_ptr_array_add(player->messages.messages, message);
  player->messages.updated = true;
}

void PL_ClearMessagesUpdated(player_t *player) {
  player->messages.updated = false;
  PL_ClearMessages(player);
}

void P_Printf(player_t *player, const char *fmt, ...) {
  va_list args;

  va_start(args, fmt);
  add_player_vmessage(player, false, false, 0, fmt, args);
  va_end(args);
}

void P_VPrintf(player_t *player, const char *fmt, va_list args) {
  add_player_vmessage(player, false, false, 0, fmt, args);
}

void P_Echo(player_t *player, const char *message) {
  add_player_message(player, false, false, 0, "%s\n", message);
}

void P_Write(player_t *player, const char *message) {
  add_player_message(player, false, false, 0, "%s", message);
}

void P_MPrintf(player_t *player, const char *fmt, ...) {
  va_list args;

  va_start(args, fmt);
  add_player_vmessage(player, true, false, 0, fmt, args);
  va_end(args);
}

void P_MVPrintf(player_t *player, const char *fmt, va_list args) {
  add_player_vmessage(player, true, false, 0, fmt, args);
}

void P_MEcho(player_t *player, const char *message) {
  add_player_message(player, true, false, 0, "%s\n", message);
}

void P_MWrite(player_t *player, const char *message) {
  add_player_message(player, true, false, 0, "%s", message);
}

void P_SPrintf(player_t *player, int sfx, const char *fmt, ...) {
  va_list args;

  va_start(args, fmt);
  add_player_vmessage(player, false, false, sfx, fmt, args);
  va_end(args);
}

void P_SVPrintf(player_t *player, int sfx, const char *fmt, va_list args) {
  add_player_vmessage(player, false, false, sfx, fmt, args);
}

void P_SEcho(player_t *player, int sfx, const char *message) {
  add_player_message(player, false, false, sfx, "%s\n", message);
}

void P_SWrite(player_t *player, int sfx, const char *message) {
  add_player_message(player, false, false, sfx, "%s", message);
}

void P_MSPrintf(player_t *player, int sfx, const char *fmt, ...) {
  va_list args;

  va_start(args, fmt);
  add_player_vmessage(player, true, false, sfx, fmt, args);
  va_end(args);
}

void P_MSVPrintf(player_t *player, int sfx, const char *fmt, va_list args) {
  add_player_vmessage(player, true, false, sfx, fmt, args);
}

void P_MSEcho(player_t *player, int sfx, const char *message) {
  add_player_message(player, true, false, sfx, "%s\n", message);
}

void P_MSWrite(player_t *player, int sfx, const char *message) {
  add_player_message(player, true, false, sfx, "%s", message);
}

void P_CenterPrintf(player_t *player, const char *fmt, ...) {
  va_list args;

  va_start(args, fmt);
  add_player_vmessage(player, false, true, 0, fmt, args);
  va_end(args);
}

void P_CenterVPrintf(player_t *player, const char *fmt, va_list args) {
  add_player_vmessage(player, false, true, 0, fmt, args);
}

void P_CenterEcho(player_t *player, const char *message) {
  add_player_message(player, false, true, 0, "%s\n", message);
}

void P_CenterWrite(player_t *player, const char *message) {
  add_player_message(player, false, true, 0, "%s", message);
}

void P_CenterMPrintf(player_t *player, const char *fmt, ...) {
  va_list args;

  va_start(args, fmt);
  add_player_vmessage(player, true, true, 0, fmt, args);
  va_end(args);
}

void P_CenterMVPrintf(player_t *player, const char *fmt, va_list args) {
  add_player_vmessage(player, true, true, 0, fmt, args);
}

void P_CenterMEcho(player_t *player, const char *message) {
  add_player_message(player, true, true, 0, "%s\n", message);
}

void P_CenterMWrite(player_t *player, const char *message) {
  add_player_message(player, true, true, 0, "%s", message);
}

void P_CenterSPrintf(player_t *player, int sfx, const char *fmt, ...) {
  va_list args;

  va_start(args, fmt);
  add_player_vmessage(player, false, true, sfx, fmt, args);
  va_end(args);
}

void P_CenterSVPrintf(player_t *player, int sfx, const char *fmt, va_list args) {
  add_player_vmessage(player, false, true, sfx, fmt, args);
}

void P_CenterSEcho(player_t *player, int sfx, const char *message) {
  add_player_message(player, false, true, sfx, "%s\n", message);
}

void P_CenterSWrite(player_t *player, int sfx, const char *message) {
  add_player_message(player, false, true, sfx, "%s", message);
}

void P_CenterMSPrintf(player_t *player, int sfx, const char *fmt, ...) {
  va_list args;

  va_start(args, fmt);
  add_player_vmessage(player, true, true, sfx, fmt, args);
  va_end(args);
}

void P_CenterMSVPrintf(player_t *player, int sfx, const char *fmt, va_list args) {
  add_player_vmessage(player, true, true, sfx, fmt, args);
}

void P_CenterMSEcho(player_t *player, int sfx, const char *message) {
  add_player_message(player, true, true, sfx, "%s\n", message);
}

void P_CenterMSWrite(player_t *player, int sfx, const char *message) {
  add_player_message(player, true, true, sfx, "%s", message);
}

void P_SendMessage(const char *message) {
  int sfx;

  if (gamemode == commercial) {
    sfx = sfx_radio;
  }
  else {
    sfx = sfx_tink;
  }

  if (CLIENT) {
    CL_SendMessage(message);
  }
  else if (SERVER) {
    SV_BroadcastMessage(message);
  }

  if (players[consoleplayer].name != NULL) {
    P_Printf(consoleplayer, "<%s>: %s\n", players[consoleplayer].name, message);
  }
  else {
    P_Printf(consoleplayer, "<Player %d>: %s\n", consoleplayer, message);
  }

  S_StartSound(NULL, sfx);
}

void P_ClearMessages(player_t *player) {
  if (players[player].messages.messages->len == 0)
    return;

  g_ptr_array_remove_range(
    player->messages.messages,
    0,
    player->messages.messages->len
  );
}

void PL_SetNameRaw(player_t *player, const char *name) {
  if (player->name) {
    free((char *)player->name);
  }

  player->name = name;
}

