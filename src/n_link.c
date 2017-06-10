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

#include "n_main.h"
#include "n_chan.h"
#include "n_com.h"
#include "n_link.h"

void N_LinkInit(netlink_t *nl, void *base_net_peer) {
  N_ComInit(&nl->com, base_net_peer);
  N_SyncInit(&nl->sync);
  nl->connection_start_time = 0;
  nl->disconnection_start_time = 0;
}

void N_LinkClear(netlink_t *nl) {
  N_ComClear(&nl->com);
  N_SyncClear(&nl->sync);
  nl->connection_start_time = 0;
  nl->disconnection_start_time = 0;
}

void N_LinkFree(netlink_t *nl) {
  N_ComFree(&nl->com);
  N_SyncFree(&nl->sync);
  nl->connection_start_time = 0;
  nl->disconnection_start_time = 0;
}

void N_LinkDisconnect(netlink_t *nl, disconnection_reason_e reason) {
  time_t now = time(NULL);

  if (nl->disconnect_start_time) {
    return;
  }

  I_NetPeerDisconnect(nc->base_net_peer, reason);
  nl->disconnect_start_time = now;
}

bool N_LinkCheckTimeout(netlink_t *nl) {
  time_t now = time(NULL);

  if (nl->connect_start_time != 0) {
    if (difftime(now, nl->connect_start_time) > NET_CONNECT_WAIT) {
      return true;
    }
  }

  if (nl->disconnect_start_time != 0) {
    if (difftime(now, nl->disconnect_start_time) > NET_DISCONNECT_WAIT) {
      return true;
    }
  }

  return false;
}

void N_LinkSetConnected(netlink_t *nl) {
  nl->connect_start_time = 0;
}

/* vi: set et ts=2 sw=2: */