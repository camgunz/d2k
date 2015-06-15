#include <stdlib.h>
#include <string.h>

#include <glib.h>
#include <glib/gstdio.h>
#include <gio/gio.h>
#include <gio/gunixsocketaddress.h>

#include "uds.h"

static uds_t uds;

static void handle_new_data(uds_t *uds, uds_peer_t *peer) {
  g_print("Got [%s] from %s\n",
    uds->input->str,
    socket_address_to_string(peer->address)
  );
}

static void cleanup(void) {
  uds_free(&uds);
}

int main(int argc, char **argv) {
  uds_peer_t *server;

  memset(&uds, 0, sizeof(uds_t));

  uds_init(&uds, CLIENT_SOCKET_NAME, handle_new_data);

  atexit(cleanup);

  uds_connect(&uds, SERVER_SOCKET_NAME);

  server = uds_get_peer(&uds, SERVER_SOCKET_NAME);

  if (!server) {
    g_printerr("No server peer!\n");
    exit(EXIT_FAILURE);
  }

  uds_sendto(&uds, server, UDS_CLIENT_ANNOUNCE);
  uds_service(&uds);
  uds_sendto(&uds, server, "and this is just junk");

  while (1)
    uds_service(&uds);

  return EXIT_SUCCESS;
}

/* vi: set et ts=2 sw=2: */

