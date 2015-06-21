#define _XOPEN_SOURCE_EXTENDED

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <wctype.h>

#include <glib.h>
#include <glib/gstdio.h>

#include <gio/gio.h>
#include <gio/gunixsocketaddress.h>

#include <curses.h>

#include <linebreak.h>

#include "uds.h"

#define LANGUAGE "en"

#define MESSAGE_INTERVAL 1000

#define INPUT_TIMEOUT 10

#define INPUT_LINE_HEIGHT  1
#define TOP_MARGIN    1
#define BOTTOM_MARGIN 1
#define LEFT_MARGIN   1
#define RIGHT_MARGIN  1

#define OUTPUT_WIDTH  (COLS)
#define OUTPUT_HEIGHT (LINES - INPUT_HEIGHT)
#define INPUT_WIDTH   (COLS)
#define INPUT_HEIGHT  (INPUT_LINE_HEIGHT + 2)
#define INPUT_Y       (OUTPUT_HEIGHT + 1)

typedef struct server_console_s {
  GMainContext *mc;
  GMainLoop    *loop;
  uds_t         uds;
  WINDOW       *input_window;
  WINDOW       *output_window;
  GString      *input;
  GString      *output;
  GString      *line_broken_output;
  gchar        *line_breaks;
  gsize         input_scroll;
  gsize         output_scroll;
  gsize         line_count;
} server_console_t;

static server_console_t server_console;

static void sc_refresh_input(server_console_t *server_console) {
  box(server_console->input_window, 0, 0);
  wrefresh(server_console->input_window);
}

static void sc_refresh_output(server_console_t *server_console) {
  box(server_console->output_window, 0, 0);
  wrefresh(server_console->output_window);
}

static void sc_clear_input(server_console_t *server_console) {
  werase(server_console->input_window);
  sc_refresh_input(server_console);
}

static void sc_clear_output(server_console_t *server_console) {
  werase(server_console->output_window);
  sc_refresh_output(server_console);
}

static void sc_get_cursor_position(server_console_t *server_console, int *x,
                                                                     int *y) {
  int cursx;
  int cursy;

  getyx(stdscr, cursy, cursx);

  *x = cursx;
  *y = cursy;
}

static void sc_set_cursor_position(server_console_t *server_console, int x,
                                                                     int y) {
  move(y, x);
}

static void sc_update_cursor(server_console_t *server_console) {
  int cury;
  int curx;
  int newy = -1;
  int newx = -1;

  getyx(stdscr, cury, curx);

  if (curx < LEFT_MARGIN)
    newx = LEFT_MARGIN;

  if (curx > INPUT_WIDTH)
    newx = INPUT_WIDTH;

  if (cury != INPUT_Y)
    newy = INPUT_Y;

  if ((newx != -1) || (newy != -1)) {
    if (newx == -1)
      newx = curx;
    if (newy == -1)
      newy = cury;

    move(newy, newx);
  }
}

static void sc_refresh(server_console_t *server_console) {
  sc_refresh_input(server_console);
  sc_refresh_output(server_console);
  sc_update_cursor(server_console);
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

static void sc_display_input(server_console_t *server_console) {
  static GString *buf;

  gsize  cols = INPUT_WIDTH;
  gsize  max_scroll;
  gsize  input_length;
  gchar *input_line;

  if (!buf)
    buf = g_string_new("");
  else
    g_string_erase(buf, 0, -1);

  if (cols <= server_console->input->len) {
    max_scroll = server_console->input->len - cols;
    input_length = cols;
  }
  else {
    max_scroll = 0;
    input_length = server_console->input->len;
  }

  server_console->input_scroll = MIN(server_console->input_scroll, max_scroll);
  input_line = server_console->input->str + server_console->input_scroll;
  g_string_insert_len(buf, 0, input_line, input_length);

  sc_clear_input(server_console);
  mvwprintw(
    server_console->input_window,
    TOP_MARGIN,
    LEFT_MARGIN,
    "%s",
    buf->str
  );

  move(INPUT_Y, buf->len + 1);
  sc_refresh_input(server_console);
  refresh();
}

static void sc_display_output(server_console_t *server_console) {
  int     cx;
  int     cy;
  int     cols;
  int     rows;
  gchar  *first_line;
  gchar **lines;

  sc_clear_output(server_console);
  sc_get_output_dimensions(server_console, &cols, &rows);

  sc_get_cursor_position(server_console, &cx, &cy);

  rows -= TOP_MARGIN + BOTTOM_MARGIN;

  first_line = g_utf8_strrchr(
    server_console->line_broken_output->str, -1, '\n'
  );

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

  for (int i = 0; i < server_console->output_scroll; i++) {
    if (!first_line)
      break;

    first_line = g_utf8_strrchr(
      server_console->line_broken_output->str,
      (first_line - server_console->line_broken_output->str) - 1,
      '\n'
    );
  }

  for (int i = 0; i < rows; i++) {
    if (!first_line)
      break;

    first_line = g_utf8_strrchr(
      server_console->line_broken_output->str,
      (first_line - server_console->line_broken_output->str) - 1,
      '\n'
    );
  }

  if (first_line)
    first_line = g_utf8_next_char(first_line);
  else
    first_line = server_console->line_broken_output->str;

  lines = g_strsplit(first_line, "\n", (rows - TOP_MARGIN) + 1);

  for (int i = 0; i < rows; i++) {
    if (!lines[i])
      break;

    if (!*lines[i])
      continue;

    mvwprintw(
      server_console->output_window,
      TOP_MARGIN + i,
      LEFT_MARGIN,
      "Line %u/%d: %s\n",
      i,
      rows,
      lines[i]
    );
  }

  g_strfreev(lines);

refresh:

  sc_refresh_output(server_console);
  sc_set_cursor_position(server_console, cx, cy);
  refresh();
}

static void sc_format_output(server_console_t *server_console, gsize offset) {
  int    cols;
  int    rows;
  gsize  col;
  gchar *os;
  gchar *ss;
  gchar *cs;
  gchar *last_line_break;

  if (!server_console->line_breaks) {
    server_console->line_breaks = g_malloc0_n(
      server_console->output->len, sizeof(gchar)
    );
  }
  else {
    server_console->line_breaks = g_realloc_n(
      server_console->line_breaks, server_console->output->len, sizeof(gchar)
    );
  }

  set_linebreaks_utf8(
    server_console->output->str + offset,
    server_console->output->len - offset,
    LANGUAGE,
    server_console->line_breaks + offset
  );

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
}

static void sc_reformat_output(server_console_t *server_console) {
  g_string_erase(server_console->line_broken_output, 0, -1);
  sc_format_output(server_console, 0);
}

static void sc_add_output(server_console_t *server_console, GString *data) {
  gsize starting_length = server_console->output->len;

  if (!data->len)
    return;

  if (!g_utf8_validate(data->str, data->len, NULL))
    g_string_printf(data, "<Invalid UTF-8 data>");

  g_string_append(server_console->output, data->str);

  sc_format_output(server_console, starting_length);
  sc_display_output(server_console);
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
  endwin();
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

static void sc_show_history_previous(server_console_t *server_console) {
}

static void sc_show_history_next(server_console_t *server_console) {
}

static void sc_move_cursor_left(server_console_t *server_console) {
}

static void sc_move_cursor_right(server_console_t *server_console) {
}

static void sc_move_cursor_home(server_console_t *server_console) {
}

static void sc_move_cursor_to_end(server_console_t *server_console) {
}

static void sc_delete_previous_char(server_console_t *server_console) {
}

static void sc_delete_next_char(server_console_t *server_console) {
}

static void sc_scroll_output_up(server_console_t *server_console) {
}

static void sc_scroll_output_down(server_console_t *server_console) {
}

static void resize_console(void) {
}

static void sc_handle_command(server_console_t *server_console) {
  if (g_strcmp0(server_console->input->str, ":q") == 0)
    exit(EXIT_SUCCESS);
  if (g_strcmp0(server_console->input->str, ":quit") == 0)
    exit(EXIT_SUCCESS);

  g_string_erase(server_console->input, 0, -1);
  sc_display_input(server_console);
}

static void sc_handle_key(server_console_t *server_console, wint_t key) {
  if (key == '\n') {
    sc_handle_command(server_console);
    return;
  }

  g_string_append_unichar(server_console->input, key);

  sc_display_input(server_console);
}

static void sc_handle_function_key(server_console_t *server_console,
                                   wint_t key) {
  switch (key) {
    case KEY_UP:
      sc_show_history_previous(server_console);
    break;
    case KEY_DOWN:
      sc_show_history_next(server_console);
    break;
    case KEY_LEFT:
      sc_move_cursor_left(server_console);
    break;
    case KEY_RIGHT:
      sc_move_cursor_right(server_console);
    break;
    case KEY_HOME:
      sc_move_cursor_home(server_console);
    break;
    case KEY_END:
      sc_move_cursor_to_end(server_console);
    break;
    case KEY_BACKSPACE:
      sc_delete_previous_char(server_console);
    break;
    case KEY_DC:
      sc_delete_next_char(server_console);
    break;
    case KEY_PPAGE:
      sc_scroll_output_up(server_console);
    break;
    case KEY_NPAGE:
      sc_scroll_output_down(server_console);
    break;
    case KEY_ENTER:
      sc_handle_command(server_console);
    break;
    default:
    break;
  }
}

static gboolean task_resize_console(gpointer user_data) {
  server_console_t *server_console = (server_console_t *)user_data;

  sc_clear_input(server_console);
  sc_clear_output(server_console);
  sc_reformat_output(server_console);
  sc_display_input(server_console);
  sc_display_output(server_console);

  return G_SOURCE_REMOVE;
}

static void handle_resize(int signal) {
  g_idle_add(task_resize_console, &server_console);
}

static gboolean task_read_input(gpointer user_data) {
  server_console_t *server_console = (server_console_t *)user_data;
  wint_t wch;
  int status = get_wch(&wch);

  switch (status) {
    case OK:
      sc_handle_key(server_console, wch);
    break;
    case KEY_CODE_YES:
      sc_handle_function_key(server_console, wch);
    break;
    default:
    break;
  }

  return G_SOURCE_CONTINUE;
}

static WINDOW* add_window(int x, int y, int width, int height) {
  WINDOW *w = newwin(height, width, y, x);

  box(w, 0, 0);
  wrefresh(w);

  return w;
}

int main(int argc, char **argv) {
  signal(SIGWINCH, handle_resize);

  initscr();
  cbreak();
  noecho();
  timeout(INPUT_TIMEOUT);
  keypad(stdscr, TRUE);
  refresh();

  memset(&server_console, 0, sizeof(server_console_t));

  uds_init(
    &server_console.uds,
    CLIENT_SOCKET_NAME,
    handle_data,
    handle_exception,
    FALSE
  );

  atexit(cleanup);

  server_console.input_window = add_window(
    0, OUTPUT_HEIGHT, INPUT_WIDTH, INPUT_HEIGHT
  );
  server_console.output_window = add_window(
    0, 0, OUTPUT_WIDTH, OUTPUT_HEIGHT
  );

  server_console.input = g_string_new("");
  server_console.output = g_string_new("");
  server_console.line_broken_output = g_string_new("");
  server_console.line_breaks = NULL;

  server_console.input_scroll = 0;
  server_console.output_scroll = 0;
  server_console.line_count = 0;

  wrefresh(server_console.input_window);
  wrefresh(server_console.output_window);
  sc_update_cursor(&server_console);
  refresh();

  uds_connect(&server_console.uds, SERVER_SOCKET_NAME);

  server_console.mc = g_main_context_default();
  server_console.loop = g_main_loop_new(server_console.mc, FALSE);

  g_idle_add(task_read_input, &server_console);
  g_timeout_add(MESSAGE_INTERVAL, task_send_data, &server_console);

  g_main_loop_run(server_console.loop);

  return EXIT_SUCCESS;
}

/* vi: set et ts=2 sw=2: */

