#include <stdlib.h>
#include <string.h>

#include <glib.h>
#include <glib/gstdio.h>
#include <gio/gio.h>
#include <gio/gunixsocketaddress.h>

#include "uds.h"

#define MESSAGE_INTERVAL 1000

static uds_t uds;

static void handle_data(uds_t *uds, uds_peer_t *peer) {
  g_print("Got data [%s]\n", uds->input->str);
  uds_peer_sendto(peer, uds->input->str);
}

static void handle_exception(uds_t *uds) {
  g_printerr("Got exception [%s]\n", uds->exception->str);
}

static gboolean task_broadcast_data(gpointer user_data) {
  static guint counter = 1;
  static GString *s = NULL;
  
  uds_t *uds = (uds_t *)user_data;

  if (!s)
    s = g_string_new("");

  g_string_printf(s, "Server message %u", counter);
  uds_broadcast(uds, s->str);

  counter++;

  return TRUE;
}

static void cleanup(void) {
  uds_free(&uds);
}

int main(int argc, char **argv) {
  GMainContext *mc;
  GMainLoop    *loop;

  memset(&uds, 0, sizeof(uds_t));

  uds_init(
    &uds,
    SERVER_SOCKET_NAME,
    handle_data,
    handle_exception,
    FALSE
  );

  atexit(cleanup);

  mc = g_main_context_default();
  loop = g_main_loop_new(mc, FALSE);
  g_timeout_add(MESSAGE_INTERVAL, task_broadcast_data, &uds);

  g_main_loop_run(loop);

  return EXIT_SUCCESS;
}

/* vi: set et ts=2 sw=2: */

