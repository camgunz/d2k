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
#include "d_event.h"
#include "g_game.h"
#include "pl_main.h"
#include "g_state.h"
#include "n_main.h"
#include "cl_cmd.h"
#include "cl_net.h"
#include "s_sound.h"

/*
 * The current command index, used for generating new commands
 */
static int cl_local_command_index = 1;

/*
 * Index of the currently running command
 */
static int cl_current_command_index = 1;

/*
 * Marked true when the client is running consoleplayer's commands
 */
static bool cl_running_consoleplayer_commands = false;

/*
 * Marked true when the client is running commands for a non-consoleplayer
 * player
 */
static bool cl_running_nonconsoleplayer_commands = false;

/*
 * Marked true when the client is running thinkers and specials
 */
static bool cl_running_thinkers = false;

int cl_extrapolate_player_positions = false;

bool CL_Predicting(void) {
  return !(CL_Synchronizing() || CL_RePredicting());
}

bool CL_RunningConsoleplayerCommands(void) {
  return cl_running_consoleplayer_commands;
}

bool CL_RunningNonConsoleplayerCommands(void) {
  return cl_running_nonconsoleplayer_commands;
}

void CL_SetRunningThinkers(bool running) {
  cl_running_thinkers = running;
}

bool CL_RunningThinkers(void) {
  return cl_running_thinkers;
}

void CL_SetupCommandState(int playernum, unsigned int command_index) {
  if (!CLIENT) {
    return;
  }

  if (playernum == consoleplayer) {
    cl_running_consoleplayer_commands = true;
    cl_running_nonconsoleplayer_commands = false;
  }
  else {
    cl_running_consoleplayer_commands = false;
    cl_running_nonconsoleplayer_commands = true;
  }

  if (cl_running_consoleplayer_commands) {
    cl_current_command_index = command_index;
  }
}

void CL_ShutdownCommandState(void) {
  cl_running_consoleplayer_commands = false;
  cl_running_nonconsoleplayer_commands = false;
}

int CL_GetCurrentCommandIndex(void) {
  return cl_current_command_index;
}

int CL_GetNextCommandIndex(void) {
  int out = cl_local_command_index;

  cl_local_command_index++;

  return out;
}

void CL_Init(void) {
}

void CL_Reset(void) {
  cl_local_command_index = 1;
  cl_current_command_index = 1;
}

/* vi: set et ts=2 sw=2: */

