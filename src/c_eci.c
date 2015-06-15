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

#include <gio/gio.h>
#include <gio/gunixsocketaddress.h>

#include "c_main.h"
#include "lprintf.h"
#include "m_file.h"
#include "x_main.h"

#define D2K_SERVER_SOCKET_NAME "d2k.sock"
#define D2K_ECI_CLIENT_ANNOUNCE "HAIL SATAN"

typedef struct eci_client_s {
  GSocketAddress *address;
  GString        *output;
  bool            disconnected;
} eci_client_t;

typedef struct external_command_interface_s {
  char       *socket_path;
  GSocket    *socket;
  GHashTable *clients;
  GString    *input;
  GString    *exception;
} external_command_interface_t;

static external_command_interface_t *eci = NULL;

static char* socket_address_to_string(GSocketAddress *addr) {
  return strdup(g_unix_socket_address_get_path(G_UNIX_SOCKET_ADDRESS(addr)));
}

static void check_eci_socket_condition(GIOCondition condition) {
  if ((condition & G_IO_ERR) == G_IO_ERR)
    I_Error("External command interface socket returned an error");
  if ((condition & G_IO_HUP) == G_IO_HUP)
    I_Error("External command interface socket hung up");
  if ((condition & G_IO_NVAL) == G_IO_NVAL)
    I_Error("External command interface socket closed");
}

static bool eci_has_urgent_data(void) {
  GIOCondition cond = g_socket_condition_check(eci->socket, G_IO_PRI);

  check_eci_socket_condition(cond);
  return (cond & G_IO_PRI) == G_IO_PRI;
}

static bool can_read_eci(void) {
  GIOCondition cond = g_socket_condition_check(eci->socket, G_IO_IN);
  
  check_eci_socket_condition(cond);
  return (cond & G_IO_IN) == G_IO_IN;
}

static bool can_write_eci(void) {
  GIOCondition cond = g_socket_condition_check(eci->socket, G_IO_OUT);
  
  check_eci_socket_condition(cond);
  return (cond & G_IO_OUT) == G_IO_OUT;
}

static void free_eci_client(eci_client_t *client) {
  g_object_unref(client->address);
  g_string_free(client->output, true);
  free(client);
}

static void read_eci_exception(void) {
  gssize bytes_read;
  GError *error = NULL;
  gssize exception_size;

  if (!eci_has_urgent_data())
    return;

  exception_size = g_socket_get_available_bytes(eci->socket);

  if (exception_size == -1)
    I_Error("Error getting exception size from ECI socket");

  if (eci->exception->len <= exception_size)
    g_string_set_size(eci->exception, exception_size + 1);

  bytes_read = g_socket_receive(
    eci->socket,
    eci->exception->str,
    eci->exception->len,
    NULL,
    &error
  );

  if (bytes_read == -1)
    I_Error("Error getting exception from ECI socket: %s", error->message);

  g_string_set_size(eci->exception, bytes_read);

  C_Printf("Exceptional data received from ECI: %s\n", eci->exception->str);
}

static void service_eci(void) {
  GSocketAddress *address;
  eci_client_t *client;
  gssize message_size;
  gssize bytes_read;
  int starting_stack_size;
  char *address_string;
  GError *error = NULL;
  bool added_client = false;

  if (!can_read_eci())
    return;

  g_string_erase(eci->input, 0, -1);

  message_size = g_socket_get_available_bytes(eci->socket);

  if (message_size == -1)
    I_Error("Error getting message size from ECI socket");

  if (eci->input->len <= message_size)
    g_string_set_size(eci->input, message_size + 1);

  bytes_read = g_socket_receive_from(
    eci->socket, &address, eci->input->str, eci->input->len, NULL, &error
  );

  if (!address) {
    C_Echo("Client socket not bound");
    return;
  }

  if (bytes_read == -1) {
    C_Printf("Error getting message from ECI socket: %s\n", error->message);
    return;
  }

  if (bytes_read == 0) {
    C_Echo("Connection closed");
    return;
  }

  g_string_set_size(eci->input, bytes_read);

  address_string = socket_address_to_string(address);
  client = g_hash_table_lookup(eci->clients, address_string);

  if (!client) {
    /*
     * CG: If a client sends no data, it cannot get an address.  Therefore, we
     *     use a pre-configured announce.  If the announce isn't what we
     *     expect, just ignore the client.
     */

    if (strcmp(D2K_ECI_CLIENT_ANNOUNCE, eci->input->str) != 0)
      return;

    client = malloc(sizeof(eci_client_t));

    if (!client)
      I_Error("Error allocating ECI client");

    client->address      = address;
    client->output       = g_string_new("");
    client->disconnected = false;

    if (!g_hash_table_insert(eci->clients, address_string, client)) {
      C_Printf("Client at %s already exists!\n", address_string);
      free_eci_client(client);

      return;
    }

    added_client = true;
  }

  starting_stack_size = X_GetStackSize(X_GetState());

  if (C_HandleInput(eci->input->str)) {
    int result_count = X_GetStackSize(X_GetState()) - starting_stack_size;

    for (int i = -1; -i <= result_count; i--) {
      char *stack_member = X_ToString(X_GetState(), i);

      g_string_append_printf(client->output, "%s\n", stack_member);
      free(stack_member);
    }
  }

  g_string_erase(eci->input, 0, -1);
}

static void send_eci_client_data(eci_client_t *client) {
  GError *error = NULL;
  gssize bytes_written;

  if (!client->output->len)
    return;

  if (!can_write_eci())
    return;

  bytes_written = g_socket_send_to(
    eci->socket,
    client->address,
    client->output->str,
    client->output->len,
    NULL,
    &error
  );

  if (bytes_written == -1 && error->code != G_IO_ERROR_WOULD_BLOCK)
    C_Printf("Error sending ECI request response: %s\n", error->message);

  if (bytes_written > 0)
    g_string_erase(client->output, 0, bytes_written);
}

static void service_eci_client_cb(gpointer key, gpointer value,
                                                gpointer user_data) {
  send_eci_client_data((eci_client_t *)value);
}

static gboolean eci_client_disconnected_cb(gpointer key, gpointer value,
                                                         gpointer user_data) {
  eci_client_t *client = (eci_client_t *)value;

  return client->disconnected;
}

static void eci_client_write_cb(gpointer key, gpointer value, gpointer user_data) {
  eci_client_t *client = (eci_client_t *)value;
  const char *output = (const char *)user_data;

  g_string_append(client->output, output);
}

static void free_eci_client_address_cb(gpointer data) {
  char *client_address = (char *)data;

  free(client_address);
}

static void free_eci_client_cb(gpointer data) {
  free_eci_client((eci_client_t *)data);
}

static void cleanup_eci(void) {
  GError *error = NULL;

  if (!g_socket_close(eci->socket, &error))
    C_Printf("Error closing ECI socket: %s\n", error->message);

  g_object_unref(eci->socket);

  if (!M_DeleteFile(eci->socket_path))
    C_Printf("Error removing ECI socket file: %s\n", M_GetFileError());

  if (eci->clients)
    g_hash_table_destroy(eci->clients);

  if (eci->input)
    g_string_free(eci->input, true);

  if (eci->exception)
    g_string_free(eci->exception, true);

  free(eci);

  eci = NULL;
}

void C_ECIInit(void) {
  char *current_folder = M_GetCurrentFolder();
  GSocketAddress *socket_address;
  GError *error = NULL;
  bool bound;

  eci = calloc(1, sizeof(external_command_interface_t));

  if (!eci)
    I_Error("C_ECIInit: Error allocating eci");

  current_folder = M_GetCurrentFolder();
  eci->socket_path = M_PathJoin(current_folder, D2K_SERVER_SOCKET_NAME);

  free(current_folder);

  eci->socket = g_socket_new(
    G_SOCKET_FAMILY_UNIX,
    G_SOCKET_TYPE_DATAGRAM,
    G_SOCKET_PROTOCOL_DEFAULT,
    &error
  );

  if (!eci->socket)
    I_Error("C_ECIInit: Error creating ECI socket: %s", error->message);

  if (M_PathExists(eci->socket_path)) {
    if (!M_IsFile(eci->socket_path))
      I_Error("ECI socket file [%s] exists, but isn't a file", eci->socket_path);

    if (!M_DeleteFile(eci->socket_path)) {
      I_Error("Error removing stale socket %s: %s",
        eci->socket_path, M_GetFileError()
      );
    }

    C_Printf("Removed stale socket file %s\n", eci->socket_path);
  }

  error = NULL;

  if (!g_initable_init((GInitable *)eci->socket, NULL, &error))
    I_Error("C_ECIInit: Error initializing ECI socket: %s", error->message);

  socket_address = g_unix_socket_address_new(eci->socket_path);

  error = NULL;

  bound = g_socket_bind(eci->socket, socket_address, false, &error);

  atexit(cleanup_eci);

  if (!bound)
    I_Error("C_ECIInit: Error binding ECI socket: %s", error->message);

  g_socket_set_blocking(eci->socket, false);

  eci->clients = g_hash_table_new_full(
    g_str_hash, g_str_equal, free_eci_client_address_cb, free_eci_client_cb
  );
  eci->input = g_string_new("");
  eci->exception = g_string_new("");
}

void C_ECIService(void) {
  if (!eci)
    return;

  if (!eci->clients)
    return;

  read_eci_exception();
  service_eci();
  g_hash_table_foreach(eci->clients, service_eci_client_cb, NULL);
  g_hash_table_foreach_remove(eci->clients, eci_client_disconnected_cb, NULL);
}

void C_ECIWrite(const char *output) {
  if (!eci)
    return;

  if (!eci->clients)
    return;

  g_hash_table_foreach(eci->clients, eci_client_write_cb, (gpointer)output);
}

/* vi: set et ts=2 sw=2: */

