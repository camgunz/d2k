#include <stdlib.h>
#include <string.h>

#include <glib.h>
#include <glib/gstdio.h>

#include <gio/gio.h>
#include <gio/gunixsocketaddress.h>

#include <ncurses.h>

#include <linebreak.h>

#include "uds.h"

#define LANGUAGE "en"

#define MESSAGE_INTERVAL 1000

#define INPUT_TIMEOUT 10

#define INPUT_HEIGHT  1
#define TOP_MARGIN    1
#define BOTTOM_MARGIN 1
#define LEFT_MARGIN   1
#define RIGHT_MARGIN  1
#define INPUT_HEIGHT  1

#define ENABLE_CURSES 1

typedef struct server_console_s {
  uds_t    uds;
  WINDOW  *input_window;
  WINDOW  *output_window;
  GString *input;
  GString *output;
  GString *line_broken_output;
  char    *line_breaks;
  gsize    cursor;
  gsize    scroll;
  gsize    line_count;
} server_console_t;

static server_console_t server_console;

static void sc_refresh_output(server_console_t *server_console) {
  box(server_console->output_window, 0, 0);
  wrefresh(server_console->output_window);
}

static void sc_refresh_input(server_console_t *server_console) {
  box(server_console->input_window, 0, 0);
  wrefresh(server_console->input_window);
}

static void sc_refresh(server_console_t *server_console) {
  sc_refresh_output(server_console);
  sc_refresh_input(server_console);
  refresh();
}

static void sc_get_output_dimensions(server_console_t *server_console,
                                     int *cols, int *rows) {
  int startx;
  int starty;
  int endx;
  int endy;

  getbegyx(server_console->output_window, starty, startx);
  getmaxyx(server_console->output_window, endy, endx);

  *cols = endx - startx;
  *rows = endy - starty;
}

static void sc_layout_output(server_console_t *server_console) {
  int     cols;
  int     rows;
  gchar  *first_line;
  gchar **lines;

  sc_get_output_dimensions(server_console, &cols, &rows);
  first_line = g_strrstr(server_console->line_broken_output->str, "\n");

  if (!first_line) {
    gsize total_window_size = rows * cols;
    gsize offset = 0;

    if (server_console->line_broken_output->len > total_window_size)
      offset = server_console->line_broken_output->len - total_window_size;

    mvwprintw(
      server_console->output_window,
      TOP_MARGIN,
      LEFT_MARGIN,
      "%s",
      server_console->line_broken_output->str + offset
    );

    goto refresh;
  }

  for (int i = 0; i < server_console->scroll; i++) {
    first_line = g_strrstr_len(
      server_console->line_broken_output->str,
      first_line - server_console->line_broken_output->str,
      "\n"
    );

    if (!first_line) {
      first_line = server_console->line_broken_output->str;
      break;
    }
  }

  while (rows--) {
    first_line = g_strrstr_len(
      server_console->line_broken_output->str,
      first_line - server_console->line_broken_output->str,
      "\n"
    );

    if (!first_line) {
      first_line = server_console->line_broken_output->str;
      break;
    }
  }

  lines = g_strsplit(first_line, "\n", rows - TOP_MARGIN);

  for (gchar **line = lines; *line; line++) {
    mvwprintw(
      server_console->output_window,
      TOP_MARGIN + (line - lines),
      LEFT_MARGIN,
      "%s",
      *line
    );
  }

  g_strfreev(lines);

refresh:

  sc_refresh_output(server_console);
  refresh();
}

static void sc_update_layout(server_console_t *server_console, gsize offset) {
  int    cols;
  int    rows;
  gsize  col;
  gchar *os;
  gchar *ss;
  gchar *cs;
  gchar *last_line_break;

  if (!server_console->line_breaks) {
    server_console->line_breaks = g_malloc0_n(
      server_console->output->len, sizeof(char)
    );
  }
  else {
    server_console->line_breaks = g_realloc_n(
      server_console->line_breaks, server_console->output->len, sizeof(char)
    );
  }

  set_linebreaks_utf8(
    server_console->output->str + offset,
    server_console->output->len - offset,
    LANGUAGE,
    server_console->line_breaks + offset
  );

#if ENABLE_CURSES
  sc_get_output_dimensions(server_console, &cols, &rows);
#else
  rows = 20;
  cols = 78;
#endif

  os = server_console->output->str;
  ss = server_console->output->str + offset;
  cs = ss;
  col = 0;
  last_line_break = NULL;

  while ((cs - os) < server_console->output->len) {
    glong lb_index = g_utf8_pointer_to_offset(os, cs);
    char lb = server_console->line_breaks[lb_index];

    if (lb == LINEBREAK_MUSTBREAK) {
      if (cs > ss) {
        g_string_insert_len(
          server_console->line_broken_output, -1, ss, (cs - ss) + 1
        );
      }

      g_string_append_c(server_console->line_broken_output, '\n');

      last_line_break = NULL;
      cs = g_utf8_next_char(cs);
      ss = cs;
      col = 0;

      continue;
    }

    if (lb == LINEBREAK_ALLOWBREAK)
      last_line_break = cs;

    col++;

    if (col == cols) {
      if (cs > ss) {
        g_string_insert_len(
          server_console->line_broken_output, -1, ss, (cs - ss) + 1
        );
      }

      g_string_append_c(server_console->line_broken_output, '\n');

      if (last_line_break)
        cs = last_line_break;

      ss = g_utf8_next_char(cs);

      col = 0;
    }

    cs = g_utf8_next_char(cs);
  }

  sc_layout_output(server_console);
}

static void sc_rebuild_layout(server_console_t *server_console) {
  g_string_erase(server_console->line_broken_output, 0, -1);
  sc_update_layout(server_console, 0);
}

static void sc_add_output(server_console_t *server_console, GString *data) {
  gsize starting_length = server_console->output->len;

  if (!data->len)
    return;

  if (!g_utf8_validate(data->str, data->len, NULL))
    g_string_printf(data, "<Invalid UTF-8 data>");

  g_string_append(server_console->output, data->str);

  sc_update_layout(server_console, starting_length);
}

static void handle_data(uds_t *uds, uds_peer_t *peer) {
  sc_add_output(&server_console, uds->input);
}

static void handle_exception(uds_t *uds) {
  sc_add_output(&server_console, g_string_prepend(uds->exception,
    "Network Exception: "
  ));
}

static void cleanup(void) {
  uds_free(&server_console.uds);
#if ENABLE_CURSES
  endwin();
#endif
}

static gboolean task_send_data(gpointer user_data) {
  static guint counter = 1;
  static GString *s = NULL;
  
  server_console_t *server_console = (server_console_t *)user_data;
  uds_t *uds = &server_console->uds;
  uds_peer_t *server = uds_get_peer(uds, SERVER_SOCKET_NAME);

  if (!s)
    s = g_string_new("");

  if (!server) {
    g_printerr("No server peer!\n");
    return G_SOURCE_REMOVE;
  }

  g_string_printf(s, "Client message %u", counter);
  uds_peer_sendto(server, s->str);

  counter++;

  return G_SOURCE_CONTINUE;
}

static void sc_handle_key(server_console_t *server_console, int key) {
}

static gboolean task_read_input(gpointer user_data) {
  server_console_t *server_console = (server_console_t *)user_data;
  int key = getch();

  if (key != ERR)
    sc_handle_key(server_console, key);

  return G_SOURCE_CONTINUE;
}

static WINDOW* add_window(int x, int y, int width, int height) {
  WINDOW *w = newwin(height, width, y, x);

  box(w, 0, 0);
  wrefresh(w);

  return w;
}

static void remove_window(WINDOW *w) {
  wborder(w, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
  wrefresh(w);
  delwin(w);
}

int main(int argc, char **argv) {
  int output_width;
  int output_height;
  int input_width;
  int input_height;
  GMainContext *mc;
  GMainLoop *loop;

#if ENABLE_CURSES
  initscr();
  cbreak();
  noecho();
  timeout(INPUT_TIMEOUT);
  // nodelay(stdscr, TRUE);
  keypad(stdscr, TRUE);
  refresh();
#endif

  output_width = COLS;
  output_height = LINES - (INPUT_HEIGHT + 2);
  input_width = COLS;
  input_height = INPUT_HEIGHT + 2;

  memset(&server_console, 0, sizeof(server_console_t));

  uds_init(
    &server_console.uds,
    CLIENT_SOCKET_NAME,
    handle_data,
    handle_exception,
    FALSE
  );

  atexit(cleanup);

#if ENABLE_CURSES
  server_console.input_window = add_window(
    0, output_height, input_width, input_height
  );
  server_console.output_window = add_window(
    0, 0, output_width, output_height
  );
#endif

  server_console.input = g_string_new("");
  server_console.output = g_string_new("");
  server_console.line_broken_output = g_string_new("");
  server_console.line_breaks = NULL;

  server_console.cursor = 0;
  server_console.scroll = 0;
  server_console.line_count = 0;

#if ENABLE_CURSES
  wrefresh(server_console.output_window);
  wrefresh(server_console.input_window);
  refresh();
#endif

  uds_connect(&server_console.uds, SERVER_SOCKET_NAME);

  mc = g_main_context_default();
  loop = g_main_loop_new(mc, FALSE);

  g_idle_add(task_read_input, &server_console);
  // g_timeout_add(INPUT_TIMEOUT, task_read_input, &server_console);
  g_timeout_add(MESSAGE_INTERVAL, task_send_data, &server_console);

  g_main_loop_run(loop);

  return EXIT_SUCCESS;
}

/* vi: set et ts=2 sw=2: */

