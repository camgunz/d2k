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


#ifndef N_UDS_H__
#define N_UDS_H__

struct uds_s;
struct uds_peer_s;

typedef void (*uds_handle_data)(struct uds_s *uds, struct uds_peer_s *peer);
typedef void (*uds_handle_oob)(struct uds_s *uds);

/* CG: TODO: Add error handler */

typedef struct uds_s {
  gchar               *socket_path;
  GSocket             *socket;
  GIOCondition         condition;
  GPtrArray           *peers;
  GHashTable          *peer_directory;
  GString             *input;
  GString             *oob;
  uds_handle_data      handle_data;
  uds_handle_oob       handle_oob;
  gboolean             waiting_for_connection;
  gboolean             has_pending_data;
  gboolean             service_manually;
} uds_t;

typedef struct uds_peer_s {
  uds_t          *uds;
  GSocketAddress *address;
  GString        *output;
  gboolean        disconnected;
} uds_peer_t;

void        N_UDSInit(uds_t *uds, const gchar *socket_path,
                                  uds_handle_data handle_data,
                                  uds_handle_oob handle_oob,
                                  gboolean service_manually);
void        N_UDSFree(uds_t *uds);
void        N_UDSConnect(uds_t *uds, const gchar *socket_path);
void        N_UDSService(uds_t *uds);
uds_peer_t* N_UDSGetPeer(uds_t *uds, const gchar *peer_address);
GIOChannel* N_UDSGetIOChannel(uds_t *uds);
void        N_UDSBroadcast(uds_t *uds, const gchar *data);
gboolean    N_UDSSendTo(uds_t *uds, const gchar *peer_address,
                                    const gchar *data);
void        N_UDSPeerSendTo(uds_peer_t *peer, const gchar *data);

#endif

/* vi: set et ts=2 sw=2: */

