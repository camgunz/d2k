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


#ifndef N_LINK_H__
#define N_LINK_H__

typedef struct netlink_s {
  netcom_t   com;
  netsync_t  sync;
  time_t     connection_start_time;
  time_t     disconnection_start_time;
} netlink_t;

void         N_LinkInit(netlink_t *nl, void *base_net_peer);
void         N_LinkClear(netlink_t *nl);
void         N_LinkFree(netlink_t *nl);
void         N_LinkDisconnect(netlink_t *nl, disconnection_reason_e reason);
bool         N_LinkCheckTimeout(netlink_t *nl);
void         N_LinkSetConnected(netlink_t *nl);

#endif

/* vi: set et ts=2 sw=2: */
