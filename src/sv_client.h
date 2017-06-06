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


#ifndef SV_MAIN_H__
#define SV_MAIN_H__

typedef struct server_client_s {
  client_info_t client_info;
  netcom_t      com;
  netsync_t     sync;
  time_t        connect_start_time;
  time_t        disconnect_start_time;
  time_t        last_setup_request_time;
} server_client_t;

double SV_ClientGetConnectionWaitTime(server_client_t *sc);
double SV_ClientGetDisconnectionWaitTime(server_client_t *sc);
double SV_ClientGetLastSetupRequestTime(server_client_t *sc);
void   SV_ClientUpdateLastSetupRequestTime(server_client_t *sc);
#endif

/* vi: set et ts=2 sw=2: */
