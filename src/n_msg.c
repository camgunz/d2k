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

#include "doomdef.h"
#include "n_main.h"

net_message_info_t network_message_info[NM_MAX] = {
  { "none", NET_CHANNEL_UNRELIABLE },
  { "setup", NET_CHANNEL_RELIABLE },
  { "full state", NET_CHANNEL_RELIABLE },
  { "authentication", NET_CHANNEL_RELIABLE },
  { "chat message", NET_CHANNEL_RELIABLE },
  { "sync", NET_CHANNEL_UNRELIABLE },
  { "player preference change", NET_CHANNEL_RELIABLE },
  { "game action", NET_CHANNEL_RELIABLE },
  { "RCON command", NET_CHANNEL_RELIABLE },
  { "vote request", NET_CHANNEL_RELIABLE },
  { "ping", NET_CHANNEL_RELIABLE },
};

/* vi: set et ts=2 sw=2: */
