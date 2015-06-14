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

typedef enum {
  ECI_REQ_NONE,
  ECI_REQ_READING,
  ECI_REQ_EXECUTING,
  ECI_REQ_RESPONDING,
  ECI_REQ_FINISHED,
  ECI_REQ_MAX
} eci_req_state_e;

typedef struct eci_request_s {
  eci_req_state_e  state;
  GSocketAddress  *address;
  GString         *input;
  GString         *output;
} eci_request_t;

typedef struct external_command_interface_s {
  char      *socket_path;
  GSocket   *socket;
  GPtrArray *requests;
  GString   *exception;
} external_command_interface_t;

static external_command_interface_t *eci = NULL;

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

static void add_eci_request(void) {
  eci_request_t *req = malloc(sizeof(eci_request_t));

  req->state   = ECI_REQ_READING;
  req->address = NULL;
  req->input   = g_string_new("");
  req->output  = g_string_new("");

  g_ptr_array_add(eci->requests, req);
}

static void read_eci_exception(void) {
  gssize bytes_read;
  GError *error = NULL;
  gssize exception_size;

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

  *(eci->exception->str + bytes_read) = 0;
}

static void read_eci_request(eci_request_t *req) {
  gssize bytes_read;
  GError *error = NULL;
  gssize message_size;

  if (!can_read_eci())
    return;

  message_size = g_socket_get_available_bytes(eci->socket);

  if (message_size == -1)
    I_Error("Error getting message size from ECI socket");

  if (req->input->len <= message_size)
    g_string_set_size(req->input, message_size + 1);

  bytes_read = g_socket_receive_from(
    eci->socket,
    &req->address,
    req->input->str,
    req->input->len,
    NULL,
    &error
  );

  if (bytes_read == -1) {
    C_Printf("Error getting message from ECI socket: %s\n", error->message);
    req->state = ECI_REQ_FINISHED;
    return;
  }

  if (bytes_read == 0) {
    C_Echo("Connection closed");
    req->state = ECI_REQ_FINISHED;
    return;
  }

  *(req->input->str + bytes_read) = 0;

  if (!req->address) {
    C_Echo("Client socket not bound");
    req->state = ECI_REQ_FINISHED;
    return;
  }

  req->state = ECI_REQ_EXECUTING;
}

static void execute_eci_request(eci_request_t *req) {
  int starting_stack_size = X_GetStackSize(X_GetState());

  if (!X_Eval(X_GetState(), req->input->str)) {
    g_string_assign(req->output, X_GetError(X_GetState()));
  }
  else {
    int result_count = X_GetStackSize(X_GetState()) - starting_stack_size;

    printf("Got %d results\n", result_count);

    g_string_erase(req->output, 0, -1);

    for (int i = -1; -i <= result_count; i--) {
      char *stack_member = X_ToString(X_GetState(), i);

      g_string_append_printf(req->output, "%s\n", stack_member);
      free(stack_member);
    }
  }

  printf("Results:\n%s", req->output->str);

  req->state = ECI_REQ_RESPONDING;
}

static void write_eci_request_response(eci_request_t *req) {
  GError *error = NULL;
  gssize bytes_written;

  if (!req->output->len) {
    req->state = ECI_REQ_FINISHED;
    return;
  }

  if (!can_write_eci())
    return;

  bytes_written = g_socket_send_to(
    eci->socket,
    req->address,
    req->output->str,
    req->output->len,
    NULL,
    &error
  );

  if (bytes_written == -1 && error->code != G_IO_ERROR_WOULD_BLOCK)
    C_Printf("Error sending ECI request response: %s\n", error->message);

  g_string_erase(req->output, 0, bytes_written);

  if (!req->output->len)
    req->state = ECI_REQ_FINISHED;
}

static void handle_eci_request(gpointer data, gpointer user_data) {
  eci_request_t *req = (eci_request_t *)data;

  switch (req->state) {
    case ECI_REQ_READING:
      read_eci_request(req);
    break;
    case ECI_REQ_EXECUTING:
      execute_eci_request(req);
    break;
    case ECI_REQ_RESPONDING:
      write_eci_request_response(req);
    break;
    case ECI_REQ_FINISHED:
    break;
    default:
      I_Error("handle_eci_request: Unknown request state %d\n", req->state);
    break;
  }
}

static void free_eci_request(gpointer data) {
  eci_request_t *req = (eci_request_t *)data;

  g_object_unref(req->address);
  g_string_free(req->input, true);
  g_string_free(req->output, true);

  free(req);
}

static void cleanup_eci(void) {
  GError *error = NULL;

  if (!g_socket_close(eci->socket, &error)) {
    C_Printf("Error closing ECI socket: %s\n", error->message);
  }

  puts("Unref'ing socket");
  g_object_unref(eci->socket);

  if (!M_DeleteFile(eci->socket_path))
    C_Printf("Error removing ECI socket file: %s\n", M_GetFileError());

  if (eci->requests)
    g_ptr_array_free(eci->requests, true);

  if (eci->exception)
    g_string_free(eci->exception, true);

  free(eci);
}

void C_InitExternalCommandInterface(void) {
  char *current_folder = M_GetCurrentFolder();
  GSocketAddress *socket_address;
  GError *error = NULL;
  bool bound;

  free(current_folder);

  eci = calloc(1, sizeof(external_command_interface_t));

  eci->socket_path = M_PathJoin(current_folder, D2K_SERVER_SOCKET_NAME);

  eci->socket = g_socket_new(
    G_SOCKET_FAMILY_UNIX,
    G_SOCKET_TYPE_DATAGRAM,
    G_SOCKET_PROTOCOL_DEFAULT,
    &error
  );

  if (!eci->socket) {
    I_Error("C_InitExternalCommandInterface: Error creating ECI socket: %s\n",
      error->message
    );
  }

  if (M_PathExists(eci->socket_path)) {
    if (M_IsFile(eci->socket_path)) {
      C_Printf("Removing stale socket file %s\n", eci->socket_path);

      if (!M_DeleteFile(eci->socket_path)) {
        I_Error("Error removing stale socket %s: %s",
          eci->socket_path, M_GetFileError()
        );
      }
    }
    else {
      I_Error("ECI socket file %s exists, but is not a file",
        eci->socket_path
      );
    }
  }

  error = NULL;

  if (!g_initable_init((GInitable *)eci->socket, NULL, &error)) {
    I_Error(
      "C_InitExternalCommandInterface: Error initializing ECI socket: %s\n",
      error->message
    );
  }

  socket_address = g_unix_socket_address_new(eci->socket_path);

  error = NULL;

  bound = g_socket_bind(eci->socket, socket_address, false, &error);

  atexit(cleanup_eci);

  if (!bound) {
    I_Error("C_InitExternalCommandInterface: Error binding ECI socket: %s\n",
      error->message
    );
  }

  g_socket_set_blocking(eci->socket, false);

  eci->requests = g_ptr_array_new_with_free_func(free_eci_request);
  eci->exception = g_string_new("");
}

void C_ServiceExternalCommandInterface(void) {
  int i = 0;

  if (eci_has_urgent_data()) {
    read_eci_exception();
    C_Printf("Exceptional data received from ECI: %s\n", eci->exception->str);
  }

  if (can_read_eci())
    add_eci_request();

  g_ptr_array_foreach(eci->requests, handle_eci_request, NULL);

  while (i < eci->requests->len) {
    eci_request_t *req = (eci_request_t *)g_ptr_array_index(eci->requests, i);

    if (req->state == ECI_REQ_FINISHED)
      g_ptr_array_remove_index_fast(eci->requests, i);
    else
      i++;
  }
}

/* vi: set et ts=2 sw=2: */

