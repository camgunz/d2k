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

#include <glib/gstdio.h>
#include <gio/gio.h>
#include <gio/gunixsocketaddress.h>

#include "n_uds.h"

static gchar* socket_address_to_string(GSocketAddress *addr) {
  return g_strdup(g_unix_socket_address_get_path(G_UNIX_SOCKET_ADDRESS(addr)));
}

static gboolean uds_readable(uds_t *uds) {
  return (uds->condition & G_IO_IN) == G_IO_IN;
}

static gboolean uds_writable(uds_t *uds) {
  return (uds->condition & G_IO_OUT) == G_IO_OUT;
}

static uds_peer_t* uds_add_peer(uds_t *uds, GSocketAddress *address) {
  uds_peer_t *peer;
  gchar *address_string = socket_address_to_string(address);

  if (g_hash_table_contains(uds->peer_directory, address_string)) {
    D_MsgLocalWarn("Peer at %s already exists!\n", address_string);
    g_free(address_string);
    return NULL;
  }

  peer = g_malloc0(sizeof(uds_peer_t));

  if (!peer) {
    D_MsgLocalError("Error allocating UDS peer\n");
    exit(EXIT_FAILURE);
  }

  peer->uds          = uds;
  peer->address      = address;
  peer->output       = g_string_new("");
  peer->disconnected = false;

  g_hash_table_insert(uds->peer_directory, address_string, peer);
  g_ptr_array_add(uds->peers, peer);

  // D_MsgLocalWarn("Added peer at %s\n", address_string);

  return peer;
}

static void uds_peer_free(uds_peer_t *peer) {
  g_object_unref(peer->address);
  g_string_free(peer->output, true);
  g_free(peer);
}

static void uds_check_for_connection(uds_t *uds) {
  GError *error = NULL;

  if (!g_socket_check_connect_result(uds->socket, &error)) {
    D_MsgLocalError("Error connecting: %s\n", error->message);
    exit(EXIT_FAILURE);
  }

  uds->waiting_for_connection = false;
}

static gboolean uds_read_data(uds_t *uds) {
  GSocketAddress *address;
  uds_peer_t *peer;
  gssize message_size;
  gssize bytes_read;
  gchar *address_string;
  GError *error = NULL;

  g_string_erase(uds->input, 0, -1);

  message_size = g_socket_get_available_bytes(uds->socket);

  if (message_size == -1) {
    D_MsgLocalError("Error getting message size from UDS\n");
    return false;
  }

  if (uds->input->len <= message_size)
    g_string_set_size(uds->input, message_size + 1);

  bytes_read = g_socket_receive_from(
    uds->socket, &address, uds->input->str, uds->input->len, NULL, &error
  );

  if (!address) {
    D_MsgLocalError("Client socket not bound\n");
    return false;
  }

  if (bytes_read == -1) {
    D_MsgLocalError("Error getting message from UDS: %s\n", error->message);
    return false;
  }

  if (bytes_read == 0) {
    D_MsgLocalError("Connection closed\n");
    return false;
  }

  g_string_set_size(uds->input, bytes_read);

  address_string = socket_address_to_string(address);

  peer = N_UDSGetPeer(uds, address_string);

  if (!peer)
    peer = uds_add_peer(uds, address);

  if (!peer) {
    return false;
  }

  g_string_set_size(uds->input, bytes_read);
  uds->handle_data(uds, peer);
  g_string_erase(uds->input, 0, -1);

  return true;
}

static gboolean uds_write_data(uds_peer_t *peer) {
  GError *error = NULL;
  gssize bytes_written;

  if (!peer->output->len)
    return false;

  bytes_written = g_socket_send_to(
    peer->uds->socket,
    peer->address,
    peer->output->str,
    peer->output->len,
    NULL,
    &error
  );

  if (bytes_written < 0) {
    if (error->code != G_IO_ERROR_WOULD_BLOCK) {
      D_MsgLocalInfo("Disconnecting client: %s\n", error->message);
      peer->disconnected = true;
      return false;
    }

    peer->uds->has_pending_data = true;
    return false;
  }

  if (bytes_written == 0) {
    peer->uds->has_pending_data = true;
    return false;
  }

  g_string_erase(peer->output, 0, bytes_written);

  if (peer->output->len)
    peer->uds->has_pending_data = true;

  return true;
}

static gboolean uds_read_oob(uds_t *uds) {
  gssize bytes_read;
  GError *error = NULL;
  gssize oob_size;

  oob_size = g_socket_get_available_bytes(uds->socket);

  if (oob_size == -1) {
    D_MsgLocalError("Error getting oob size from UDS\n");
    return false;
  }

  if (uds->oob->len <= oob_size)
    g_string_set_size(uds->oob, oob_size + 1);

  bytes_read = g_socket_receive(
    uds->socket,
    uds->oob->str,
    uds->oob->len,
    NULL,
    &error
  );

  if (bytes_read == -1) {
    D_MsgLocalError("Error getting oob from UDS: %s\n", error->message);
    return false;
  }

  if (bytes_read == 0)
    return false;

  g_string_set_size(uds->oob, bytes_read);
  uds->handle_oob(uds);

  return true;
}

static gboolean uds_read_oob_check(uds_t *uds) {
  if ((uds->condition & G_IO_PRI) != G_IO_PRI)
    return true;

  return uds_read_oob(uds);
}

static gboolean uds_peer_disconnected_cb(gpointer key, gpointer value,
                                                       gpointer user_data) {
  uds_peer_t *peer = (uds_peer_t *)value;

  return peer->disconnected;
}

static void uds_peer_sendto_cb(gpointer key, gpointer value,
                                             gpointer user_data) {
  uds_peer_t *peer = (uds_peer_t *)value;

  N_UDSPeerSendTo(peer, (const gchar *)user_data);
}

static void uds_peer_free_address_cb(gpointer data) {
  gchar *peer_address = (gchar *)data;

  g_free(peer_address);
}

static void uds_peer_free_cb(gpointer data) {
  uds_peer_free((uds_peer_t *)data);
}

static void uds_remove_disconnected_peers(uds_t *uds) {
  if (!uds->peers->len)
    return;

  for (int i = uds->peers->len - 1; i >= 0; i--) {
    uds_peer_t *peer = (uds_peer_t *)g_ptr_array_index(uds->peers, i);

    if (peer->disconnected)
      g_ptr_array_remove_index_fast(uds->peers, i);
  }

  g_hash_table_foreach_remove(
    uds->peer_directory, uds_peer_disconnected_cb, NULL
  );
}

static void uds_write_data_cb(gpointer data, gpointer user_data) {
  uds_write_data((uds_peer_t *)data);
}

static void uds_update_condition(uds_t *uds) {
  uds->condition = g_socket_condition_check(uds->socket,
    G_IO_IN   |
    G_IO_OUT  |
    G_IO_PRI  |
    G_IO_ERR  |
    G_IO_HUP  |
    G_IO_NVAL
  );
}

static void uds_read_data_check(uds_t *uds) {
  if (!uds_readable(uds))
    return;

  uds_read_data(uds);
}

static void uds_flush_output(uds_t *uds) {
  uds->has_pending_data = false;
  g_ptr_array_foreach(uds->peers, uds_write_data_cb, uds);
  uds_remove_disconnected_peers(uds);
}

static void uds_flush_output_check(uds_t *uds) {
  if (!uds_writable(uds))
    return;

  uds_flush_output(uds);
}

static gboolean uds_flush_output_cb(GIOChannel *source, GIOCondition condition,
                                                        gpointer data) {
  uds_t *uds = (uds_t *)data;

  uds_flush_output(uds);

  return uds->has_pending_data;
}

static gboolean uds_handle_input_cb(GIOChannel *source, GIOCondition condition,
                                                        gpointer data) {
  uds_read_data((uds_t *)data);
  return true;
}

static gboolean uds_handle_oob_cb(GIOChannel *source,
                                  GIOCondition condition,
                                  gpointer data) {
  uds_read_oob((uds_t *)data);
  return true;
}

void N_UDSInit(uds_t *uds, const gchar *socket_path,
                           uds_handle_data handle_data,
                           uds_handle_oob handle_oob,
                           gboolean service_manually) {
  GSocketAddress *socket_address;
  GError *error = NULL;
  gboolean bound;

  uds->socket_path = g_strdup(socket_path);

  uds->socket = g_socket_new(
    G_SOCKET_FAMILY_UNIX,
    G_SOCKET_TYPE_DATAGRAM,
    G_SOCKET_PROTOCOL_DEFAULT,
    &error
  );

  if (!uds->socket) {
    D_MsgLocalError("Error creating UDS: %s\n", error->message);
    exit(EXIT_FAILURE);
  }

  if (g_file_test(uds->socket_path, G_FILE_TEST_EXISTS)) {
    if (g_file_test(uds->socket_path, G_FILE_TEST_IS_DIR |
                                      G_FILE_TEST_IS_SYMLINK)) {
      D_MsgLocalError("UDS file [%s] exists, but isn't a file\n",
        uds->socket_path
      );
      exit(EXIT_FAILURE);
    }

    if (g_unlink(uds->socket_path) == -1) {
      D_MsgLocalError("Error removing stale socket %s: %s\n",
        uds->socket_path, strerror(errno)
      );
      exit(EXIT_FAILURE);
    }
  }

  error = NULL;

  if (!g_initable_init((GInitable *)uds->socket, NULL, &error)) {
    D_MsgLocalError("Error initializing UDS: %s\n", error->message);
    exit(EXIT_FAILURE);
  }

  socket_address = g_unix_socket_address_new(uds->socket_path);

  error = NULL;

  bound = g_socket_bind(uds->socket, socket_address, false, &error);

  if (!bound) {
    D_MsgLocalError("Error binding UDS to %s: %s\n",
      uds->socket_path,
      error->message
    );
    exit(EXIT_FAILURE);
  }

  g_socket_set_blocking(uds->socket, false);

  uds->peers = g_ptr_array_new_with_free_func(uds_peer_free_cb);
  uds->peer_directory = g_hash_table_new_full(
    g_str_hash, g_str_equal, uds_peer_free_address_cb, NULL
  );
  uds->input = g_string_new("");
  uds->oob = g_string_new("");
  uds->handle_data = handle_data;
  uds->handle_oob = handle_oob;
  uds->waiting_for_connection = false;
  uds->has_pending_data = false;
  uds->service_manually = service_manually;

  if (!uds->service_manually) {
    g_io_add_watch(
      N_UDSGetIOChannel(uds),
      G_IO_IN,
      uds_handle_input_cb,
      uds
    );
    g_io_add_watch(
      N_UDSGetIOChannel(uds),
      G_IO_PRI,
      uds_handle_oob_cb,
      uds
    );
  }
}

void N_UDSFree(uds_t *uds) {
  GError *error = NULL;

  if (!g_socket_close(uds->socket, &error)) {
    D_MsgLocalError("Error closing UDS: %s\n", error->message);
    exit(EXIT_FAILURE);
  }

  g_object_unref(uds->socket);

  if (g_file_test(uds->socket_path, G_FILE_TEST_EXISTS)) {
    if (g_unlink(uds->socket_path) == -1) {
      D_MsgLocalError("Error removing UDS file: %s\n", strerror(errno));
      exit(EXIT_FAILURE);
    }
  }

  if (uds->peer_directory)
    g_hash_table_destroy(uds->peer_directory);

  if (uds->input)
    g_string_free(uds->input, true);

  if (uds->oob)
    g_string_free(uds->oob, true);
}

void N_UDSConnect(uds_t *uds, const gchar *socket_path) {
  GSocketAddress *socket_address = g_unix_socket_address_new(socket_path);
  GError *error = NULL;

  if (!g_socket_connect(uds->socket, socket_address, NULL, &error)) {
    D_MsgLocalError("%s", error->message);
    exit(EXIT_FAILURE);
  }

  uds_add_peer(uds, socket_address);
}

void N_UDSService(uds_t *uds) {
  uds_update_condition(uds);

  uds_read_oob_check(uds);

  if (uds->waiting_for_connection) {
    uds_check_for_connection(uds);
    return;
  }

  uds_read_data_check(uds);
  uds_flush_output_check(uds);
}

uds_peer_t* N_UDSGetPeer(uds_t *uds, const gchar *peer_address) {
  return g_hash_table_lookup(uds->peer_directory, peer_address);
}

GIOChannel* N_UDSGetIOChannel(uds_t *uds) {
  return g_io_channel_unix_new(g_socket_get_fd(uds->socket));
}

void N_UDSBroadcast(uds_t *uds, const gchar *data) {
  g_hash_table_foreach(
    uds->peer_directory, uds_peer_sendto_cb, (gpointer)data
  );
}

gboolean N_UDSSendTo(uds_t *uds, const gchar *peer_address, const gchar *data) {
  uds_peer_t *peer = g_hash_table_lookup(uds->peer_directory, peer_address);

  if (!peer)
    return false;

  N_UDSPeerSendTo(peer, data);
  return true;
}

void N_UDSPeerSendTo(uds_peer_t *peer, const gchar *data) {
  g_string_append(peer->output, data);

  peer->uds->has_pending_data = true;

  if (!peer->uds->service_manually) {
    g_io_add_watch(
      N_UDSGetIOChannel(peer->uds),
      G_IO_OUT,
      uds_flush_output_cb,
      peer->uds
    );
  }
}

/* vi: set et ts=2 sw=2: */

