#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <glib.h>
#include <glib/gstdio.h>
#include <gio/gio.h>
#include <gio/gunixsocketaddress.h>

#include "uds.h"

static void check_uds_socket_condition(GIOCondition condition) {
  if ((condition & G_IO_ERR) == G_IO_ERR) {
    g_printerr("External command interface socket returned an error\n");
    exit(EXIT_FAILURE);
  }
  if ((condition & G_IO_HUP) == G_IO_HUP) {
    g_printerr("External command interface socket hung up\n");
    exit(EXIT_FAILURE);
  }
  if ((condition & G_IO_NVAL) == G_IO_NVAL) {
    g_printerr("External command interface socket closed\n");
    exit(EXIT_FAILURE);
  }
}

static gboolean uds_has_urgent_data(uds_t *uds) {
  GIOCondition cond = g_socket_condition_check(uds->socket, G_IO_PRI);

  check_uds_socket_condition(cond);
  return (cond & G_IO_PRI) == G_IO_PRI;
}

static gboolean uds_readable(uds_t *uds) {
  GIOCondition cond = g_socket_condition_check(uds->socket, G_IO_IN);
  
  check_uds_socket_condition(cond);
  return (cond & G_IO_IN) == G_IO_IN;
}

static gboolean uds_writable(uds_t *uds) {
  GIOCondition cond = g_socket_condition_check(uds->socket, G_IO_OUT);
  
  check_uds_socket_condition(cond);
  return (cond & G_IO_OUT) == G_IO_OUT;
}

static void uds_send_peer_data(uds_t *uds, uds_peer_t *peer) {
  GError *error = NULL;
  gssize bytes_written;

  if (!peer->output->len)
    return;

  if (!uds_writable(uds))
    return;

  bytes_written = g_socket_send_to(
    uds->socket,
    peer->address,
    peer->output->str,
    peer->output->len,
    NULL,
    &error
  );

  if (bytes_written == -1 && error->code != G_IO_ERROR_WOULD_BLOCK)
    g_print("Error sending UDS request response: %s\n", error->message);

  if (bytes_written > 0)
    g_string_erase(peer->output, 0, bytes_written);
}

static void uds_service_peer_cb(gpointer key, gpointer value,
                                              gpointer user_data) {
  uds_send_peer_data((uds_t *)user_data, (uds_peer_t *)value);
}

static gboolean uds_peer_disconnected_cb(gpointer key, gpointer value,
                                                       gpointer user_data) {
  uds_peer_t *peer = (uds_peer_t *)value;

  return peer->disconnected;
}

static void uds_peer_write_cb(gpointer key, gpointer value,
                                            gpointer user_data) {
  uds_peer_t *peer = (uds_peer_t *)value;
  const gchar *output = (const gchar *)user_data;

  g_string_append(peer->output, output);
}

static void uds_peer_free_address_cb(gpointer data) {
  gchar *peer_address = (gchar *)data;

  g_free(peer_address);
}

static void uds_peer_free_cb(gpointer data) {
  uds_peer_free((uds_peer_t *)data);
}

static void uds_read_exception(uds_t *uds) {
  gssize bytes_read;
  GError *error = NULL;
  gssize exception_size;

  if (!uds_has_urgent_data(uds))
    return;

  exception_size = g_socket_get_available_bytes(uds->socket);

  if (exception_size == -1) {
    g_printerr("Error getting exception size from UDS\n");
    exit(EXIT_FAILURE);
  }

  if (uds->exception->len <= exception_size)
    g_string_set_size(uds->exception, exception_size + 1);

  bytes_read = g_socket_receive(
    uds->socket,
    uds->exception->str,
    uds->exception->len,
    NULL,
    &error
  );

  if (bytes_read == -1) {
    g_printerr("Error getting exception from UDS: %s\n",
      error->message
    );
    exit(EXIT_FAILURE);
  }

  g_string_set_size(uds->exception, bytes_read);

  g_print("Exceptional data received from UDS: %s\n", uds->exception->str);
}

static uds_peer_t* uds_add_peer(uds_t *uds, GSocketAddress *address) {
  uds_peer_t *peer;
  gchar *address_string = socket_address_to_string(address);

  if (g_hash_table_contains(uds->peers, address_string)) {
    g_free(address_string);

    g_print("Peer at %s already exists!\n", address_string);

    return NULL;
  }

  peer = g_malloc(sizeof(uds_peer_t));

  if (!peer) {
    g_printerr("Error allocating UDS peer\n");
    exit(EXIT_FAILURE);
  }

  peer->address      = address;
  peer->output       = g_string_new("");
  peer->disconnected = FALSE;

  g_hash_table_insert(uds->peers, address_string, peer);

  g_print("Added peer at %s\n", address_string);

  return peer;
}

static void uds_handle_new_peers(uds_t *uds) {
  GSocketAddress *address;
  uds_peer_t *peer;
  gssize message_size;
  gssize bytes_read;
  gchar *address_string;
  GError *error = NULL;
  gboolean added_peer = FALSE;

  if (!uds_readable(uds))
    return;

  g_string_erase(uds->input, 0, -1);

  message_size = g_socket_get_available_bytes(uds->socket);

  if (message_size == -1) {
    g_printerr("Error getting message size from UDS\n");
    exit(EXIT_FAILURE);
  }

  if (uds->input->len <= message_size)
    g_string_set_size(uds->input, message_size + 1);

  bytes_read = g_socket_receive_from(
    uds->socket, &address, uds->input->str, uds->input->len, NULL, &error
  );

  if (!address) {
    g_print("Client socket not bound\n");
    return;
  }

  if (bytes_read == -1) {
    g_print("Error getting message from UDS: %s\n", error->message);
    return;
  }

  if (bytes_read == 0) {
    g_print("Connection closed\n");
    return;
  }

  g_string_set_size(uds->input, bytes_read);

  address_string = socket_address_to_string(address);

  peer = uds_get_peer(uds, address_string);

  if (!peer)
    peer = uds_add_peer(uds, address);

  if (peer)
    uds->handle_new_data(uds, peer);
  else
    g_printerr("Error adding new peer\n");

  g_string_erase(uds->input, 0, -1);
}

static gboolean uds_check_connection(uds_t *uds) {
  GError *error = NULL;

  if (!uds_writable(uds))
    return FALSE;

  if (!g_socket_check_connect_result(uds->socket, &error)) {
    g_printerr("Error connecting: %s\n", error->message);
    exit(EXIT_FAILURE);
  }

  uds->waiting_for_connection = FALSE;

  return TRUE;
}

gchar* socket_address_to_string(GSocketAddress *addr) {
  return g_strdup(g_unix_socket_address_get_path(G_UNIX_SOCKET_ADDRESS(addr)));
}

void uds_free(uds_t *uds) {
  GError *error = NULL;

  if (!g_socket_close(uds->socket, &error)) {
    g_printerr("Error closing UDS: %s\n", error->message);
    exit(EXIT_FAILURE);
  }

  g_object_unref(uds->socket);

  if (g_file_test(uds->socket_path, G_FILE_TEST_EXISTS)) {
    if (g_unlink(uds->socket_path) == -1) {
      g_printerr("Error removing UDS file: %s\n", strerror(errno));
      exit(EXIT_FAILURE);
    }
  }

  if (uds->peers)
    g_hash_table_destroy(uds->peers);

  if (uds->input)
    g_string_free(uds->input, TRUE);

  if (uds->exception)
    g_string_free(uds->exception, TRUE);
}

void uds_peer_free(uds_peer_t *peer) {
  g_object_unref(peer->address);
  g_string_free(peer->output, TRUE);
  g_free(peer);
}

void uds_connect(uds_t *uds, const gchar *socket_path) {
  GSocketAddress *socket_address = g_unix_socket_address_new(socket_path);
  GError *error = NULL;

  if (!g_socket_connect(uds->socket, socket_address, NULL, &error)) {
    g_printerr(error->message);
    exit(EXIT_FAILURE);
  }

  uds_add_peer(uds, socket_address);
}

uds_peer_t* uds_get_peer(uds_t *uds, const gchar *peer_address) {
  return g_hash_table_lookup(uds->peers, peer_address);
}

void uds_service(uds_t *uds) {
  uds_read_exception(uds);

  if (uds->waiting_for_connection) {
    uds_check_connection(uds);
    return;
  }

  uds_handle_new_peers(uds);
  g_hash_table_foreach(uds->peers, uds_service_peer_cb, uds);
  g_hash_table_foreach_remove(uds->peers, uds_peer_disconnected_cb, NULL);
}

void uds_sendto(uds_t *uds, uds_peer_t *peer, const gchar *data) {
  g_string_append(peer->output, data);
}

void uds_broadcast(uds_t *uds, const gchar *data) {
  g_hash_table_foreach(uds->peers, uds_peer_write_cb, (gpointer)data);
}

void uds_init(uds_t *uds, const gchar *socket_path,
                          uds_handle_new_data handle_new_data) {
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
    g_printerr("Error creating UDS: %s\n", error->message);
    exit(EXIT_FAILURE);
  }

  if (g_file_test(uds->socket_path, G_FILE_TEST_EXISTS)) {
    if (g_file_test(uds->socket_path, G_FILE_TEST_IS_DIR |
                                      G_FILE_TEST_IS_SYMLINK)) {
      g_printerr("UDS file [%s] exists, but isn't a file\n",
        uds->socket_path
      );
      exit(EXIT_FAILURE);
    }

    if (g_unlink(uds->socket_path) == -1) {
      g_printerr("Error removing stale socket %s: %s\n",
        uds->socket_path, strerror(errno)
      );
      exit(EXIT_FAILURE);
    }

    g_print("Removed stale socket file %s\n", uds->socket_path);
  }

  error = NULL;

  if (!g_initable_init((GInitable *)uds->socket, NULL, &error)) {
    g_printerr("Error initializing UDS: %s\n", error->message);
    exit(EXIT_FAILURE);
  }

  socket_address = g_unix_socket_address_new(uds->socket_path);

  error = NULL;

  bound = g_socket_bind(uds->socket, socket_address, FALSE, &error);

  if (!bound) {
    g_printerr("Error binding UDS: %s\n", error->message);
    g_printerr("Socket path is [%s]\n", uds->socket_path);
    exit(EXIT_FAILURE);
  }

  g_socket_set_blocking(uds->socket, FALSE);

  uds->peers = g_hash_table_new_full(
    g_str_hash, g_str_equal, uds_peer_free_address_cb, uds_peer_free_cb
  );
  uds->input = g_string_new("");
  uds->exception = g_string_new("");
  uds->handle_new_data = handle_new_data;
  uds->waiting_for_connection = FALSE;
}

/* vi: set et ts=2 sw=2: */

