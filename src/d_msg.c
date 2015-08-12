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

#include "c_main.h"
#include "d_msg.h"
#include "m_file.h"

/*
 * CG [TODO]: Add an API to disable flushing a message channel's log file after
 *            writing every message.
 */

typedef struct message_channel_s {
  bool  active;
  FILE *fobj;
} message_channel_t;

static bool message_channels_initialized = false;
static message_channel_t message_channels[MSG_MAX];

static void check_message_channel(msg_channel_e c) {
  if (!message_channels_initialized)
    I_Error("Messaging has not yet been initialized!");

  if (c < MSG_MIN)
    I_Error("Invalid message channel %d (valid: %d - %d)", c, MSG_MIN, MSG_MAX);

  if (c > MSG_MAX)
    I_Error("Invalid message channel %d (valid: %d - %d)", c, MSG_MIN, MSG_MAX);
}

static void deactivate_message_channels(void) {
  for (msg_channel_e chan = MSG_MIN; chan < MSG_MAX; chan++)
    D_MsgDeactivate(chan);
}

void D_InitMessaging(void) {
  for (msg_channel_e chan = MSG_MIN; chan <= MSG_MAX; chan++) {
    message_channel_t *mc = &message_channels[chan];

    mc->active = false;
    mc->fobj = NULL;
  }
  atexit(deactivate_message_channels);

  message_channels_initialized = true;
}

bool D_MsgActive(msg_channel_e chan) {
  check_message_channel(chan);

  return message_channels[chan].active;
}

void D_MsgActivate(msg_channel_e chan) {
  check_message_channel(chan);

  message_channels[chan].active = true;
}

bool D_MsgActivateWithFile(msg_channel_e chan, const char *file_path) {
  check_message_channel(chan);

  if (!D_LogToFile(chan, file_path))
    return false;

  D_MsgActivate(chan);

  return true;
}

void D_MsgDeactivate(msg_channel_e chan) {
  message_channel_t *mc;

  check_message_channel(chan);

  mc = &message_channels[chan];

  if (mc->fobj != NULL) {
    fflush(mc->fobj);
    fclose(mc->fobj);
  }

  mc->active = false;
  mc->fobj = NULL;
}

void D_VMsg(msg_channel_e chan, const char *fmt, va_list args) {
  va_list log_args;
  va_list console_args;
  message_channel_t *mc;

  check_message_channel(chan);

  mc = &message_channels[chan];

  va_copy(console_args, args);
  C_MVPrintf(fmt, console_args);
  va_end(console_args);

  if (mc->fobj) {
    va_copy(log_args, args);
    vfprintf(mc->fobj, fmt, log_args);
    va_end(log_args);
    fflush(mc->fobj);
  }
}

void D_Msg(msg_channel_e chan, const char *fmt, ...) {
  va_list args;
  va_list log_args;
  va_list console_args;
  message_channel_t *mc;

  check_message_channel(chan);

  mc = &message_channels[chan];

  if (!mc->active)
    return;

  va_start(args, fmt);

  va_copy(console_args, args);
  C_MVPrintf(fmt, console_args);
  va_end(console_args);

  if (mc->fobj) {
    va_copy(log_args, args);
    vfprintf(mc->fobj, fmt, log_args);
    va_end(log_args);
    fflush(mc->fobj);
  }

  va_end(args);
}

bool D_LogToFile(msg_channel_e chan, const char *file_path) {
  check_message_channel(chan);

  if (message_channels[chan].fobj) {
    if (!M_CloseFile(message_channels[chan].fobj)) {
      D_MsgDeactivate(chan);
      return false;
    }
  }

  message_channels[chan].fobj = M_OpenFile(file_path, "w");

  if (message_channels[chan].fobj)
    D_MsgActivate(chan);
  else
    D_MsgDeactivate(chan);

  return D_MsgActive(chan);
}

int D_MsgGetFD(msg_channel_e chan) {
  int fd;
  message_channel_t *mc;

  check_message_channel(chan);

  mc = &message_channels[chan];

  if (!mc->active)
    return -1;

  if (!mc->fobj)
    return -1;

  fd = fileno(mc->fobj);

  if (fd == -1) {
    I_Error("Error retrieving file descriptor from log file: %s\n",
      strerror(errno)
    );
  }

  return fd;
}

/* vi: set et ts=2 sw=2: */

