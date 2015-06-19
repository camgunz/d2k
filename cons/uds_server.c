#include <stdlib.h>
#include <string.h>

#include <glib.h>
#include <glib/gstdio.h>
#include <gio/gio.h>
#include <gio/gunixsocketaddress.h>

#include "uds.h"

#define MESSAGE_INTERVAL 1000

typedef struct server_app_s {
  uds_t uds;
  GMainContext *mc;
  GMainLoop *loop;
} server_app_t;

static server_app_t app;

static void handle_data(uds_t *uds, uds_peer_t *peer) {
  g_print("Got data [%s]\n", uds->input->str);
  g_string_append(peer->output, uds->input->str);
}

static void handle_exception(uds_t *uds) {
  g_printerr("Got exception [%s]\n", uds->exception->str);
}

static gboolean task_broadcast_data(gpointer user_data) {
  static guint counter = 1;
  static GString *s = NULL;
  
  server_app_t *app = (server_app_t *)user_data;

  if (!s)
    s = g_string_new("");

  g_string_printf(s, "Server message %u\n", counter);
  uds_broadcast(&app->uds, s->str);

  counter++;

  g_source_attach(uds_get_write_source(&app->uds), app->mc);

  return TRUE;
}

static void cleanup(void) {
  uds_free(&app.uds);
}

int main(int argc, char **argv) {
  memset(&app, 0, sizeof(server_app_t));

  uds_init(
    &app.uds, SERVER_SOCKET_NAME, handle_data, handle_exception
  );

  atexit(cleanup);

  app.mc = g_main_context_default();
  app.loop = g_main_loop_new(app.mc, FALSE);
  g_timeout_add(MESSAGE_INTERVAL, task_broadcast_data, &app);
  g_source_attach(uds_get_read_source(&app.uds), app.mc);
  g_source_attach(uds_get_exception_source(&app.uds), app.mc);

  g_main_loop_run(app.loop);

  return EXIT_SUCCESS;
}

/* vi: set et ts=2 sw=2: */

