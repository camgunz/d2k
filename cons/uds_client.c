#include <stdlib.h>
#include <string.h>

#include <ncurses.h>

#include <glib.h>
#include <glib/gstdio.h>
#include <gio/gio.h>
#include <gio/gunixsocketaddress.h>

#include "uds.h"

#define MESSAGE_INTERVAL 1000

typedef struct display_s {
  WINDOW *server_output;
  WINDOW *console_input;
} display_t;

static uds_t uds;

static void handle_data(uds_t *uds, uds_peer_t *peer) {
  g_print("Got [%s] from %s\n",
    uds->input->str,
    socket_address_to_string(peer->address)
  );
}

static void handle_exception(uds_t *uds) {
  g_printerr("Got exception [%s]\n", uds->exception->str);
}

static void cleanup(void) {
  uds_free(&uds);
  // endwin();
}

static gboolean task_service_uds(gpointer user_data) {
  uds_t *uds = (uds_t *)user_data;

  uds_service(uds);

  return TRUE;
}

static gboolean task_send_data(gpointer user_data) {
  static guint counter = 1;
  static GString *s = NULL;
  
  uds_t *uds = (uds_t *)user_data;
  uds_peer_t *server = uds_get_peer(uds, SERVER_SOCKET_NAME);

  if (!s)
    s = g_string_new("");

  if (!server) {
    g_printerr("No server peer!\n");
    return FALSE;
  }

  g_string_printf(s, "Message %u\n", counter);
  uds_peer_sendto(server, s->str);

  counter++;

  return TRUE;
}

static gboolean task_refresh(gpointer user_data) {
  display_t *display = (display_t *)user_data;

  wrefresh(display->server_output);
  wrefresh(display->console_input);
  refresh();

  return TRUE;
}

static WINDOW* add_window(int x, int y, int width, int height) {
  WINDOW *w = newwin(height, width, y, x);

  box(w, 0, 0);
  // wborder(w, '|', '|', '-', '-', '+', '+', '+', '+');
  wrefresh(w);

  // g_print("Added window %dx%d+%d+%d\n", x, y, width, height);

  return w;
}

static void remove_window(WINDOW *w) {
  wborder(w, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
  wrefresh(w);
  delwin(w);
}

int main(int argc, char **argv) {
  display_t display;
  int output_width;
  int output_height;
  int input_width;
  int input_height;
  uds_peer_t *server;
  GMainContext *mc;
  GMainLoop *loop;

  /*
  initscr();
  cbreak();
  keypad(stdscr, TRUE);
  refresh();
  */

  output_width = COLS - 1;
  output_height = LINES - 3;
  input_width = COLS - 1;
  input_height = 3;

  /*
  display.server_output = add_window(0, 0, output_width, output_height);
  display.console_input = add_window(
    0, output_height, input_width, input_height
  );

  wrefresh(display.server_output);
  wrefresh(display.console_input);
  refresh();
  */

  memset(&uds, 0, sizeof(uds_t));

  uds_init(
    &uds, CLIENT_SOCKET_NAME, handle_data, handle_exception
  );

  atexit(cleanup);

  uds_connect(&uds, SERVER_SOCKET_NAME);

  mc = g_main_context_default();
  loop = g_main_loop_new(mc, FALSE);

  // g_idle_add(task_service_uds, &uds);
  // g_idle_add(task_refresh, &display);
  g_timeout_add(MESSAGE_INTERVAL, task_send_data, &uds);
  g_source_attach(uds_get_read_source(&uds), mc);
  g_source_attach(uds_get_write_source(&uds), mc);
  g_source_attach(uds_get_exception_source(&uds), mc);

  g_main_loop_run(loop);

  return EXIT_SUCCESS;
}

/* vi: set et ts=2 sw=2: */

