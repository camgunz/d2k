#include <stdlib.h>
#include <string.h>

#include <glib.h>
#include <glib/gstdio.h>
#include <gio/gio.h>
#include <gio/gunixsocketaddress.h>

#include "uds.h"

static uds_t uds;

static void handle_new_data(uds_t *uds, uds_peer_t *peer) {
  g_string_append(peer->output, uds->input->str);
}

static void cleanup(void) {
  uds_free(&uds);
}

int main(int argc, char **argv) {
  memset(&uds, 0, sizeof(uds_t));

  atexit(cleanup);

  uds_init(&uds, SERVER_SOCKET_NAME, handle_new_data);

  while (1)
    uds_service(&uds);

  return EXIT_SUCCESS;
}

/* vi: set et ts=2 sw=2: */

