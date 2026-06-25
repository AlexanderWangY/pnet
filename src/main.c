#include <bpf/libbpf.h>
#include <locale.h>
#include <ncurses.h>
#include <net/if.h>
#include <signal.h>
#include <stdbool.h>
#include <unistd.h>
#include "ui.h"
#include "state.h"
#include "kern.skel.h"

#define UNUSED(x) (void)(x)

void init_colors() {
  start_color();
  use_default_colors();
  if (can_change_color()) {
    init_color(COLOR_BLACK, 0, 0, 0);
    init_color(COLOR_WHITE, 1000, 1000, 1000);
    init_color(17, 350, 350, 350);
  }
  init_pair(CP_DEFAULT,   -1,            -1);
  init_pair(CP_INVERTED,  COLOR_BLACK,   COLOR_WHITE);
  init_pair(CP_GREEN,     COLOR_GREEN,   -1);
  init_pair(CP_RED,       COLOR_RED,     -1);
  init_pair(CP_YELLOW,    COLOR_YELLOW,  -1);
  init_pair(CP_CYAN,      COLOR_CYAN,    -1);
  init_pair(CP_BLUE,      COLOR_BLUE,    -1);
  init_pair(CP_MAGENTA,   COLOR_MAGENTA, -1);
  init_pair(CP_GREEN_INV, COLOR_BLACK,   COLOR_GREEN);
  init_pair(CP_RED_INV,   COLOR_WHITE,   COLOR_RED);
  init_pair(CP_SELECTED,  COLOR_WHITE,   17);
}

static volatile bool running = true;

void signal_handler(int sig) {
  UNUSED(sig);
  running = false;
}

int main() {
  setlocale(LC_ALL, "");

  static AppState state;
  state__init_default_app(&state);

  struct kern_bpf *skel;
  int err;

  signal(SIGINT, signal_handler);
  signal(SIGTERM, signal_handler);

  skel = kern_bpf__open_and_load();
  if (!skel) {
    fprintf(stderr, "Failed to load skeleton.\n");
    err = 1;
    goto cleanup;
  }

  err = kern_bpf__attach(skel);
  if (err) {
    fprintf(stderr, "Failed to attach skeleton.\n");
    goto cleanup;
  }

  initscr();
  cbreak();
  noecho();
  keypad(stdscr, TRUE);
  timeout(100);
  curs_set(0);
  init_colors();

  WINDOW *header = newwin(1, COLS, 0, 0);
  WINDOW *summary = newwin(1, COLS, 1, 0);
  WINDOW *col_header = newwin(1, COLS, 3, 0);
  WINDOW *footer = newwin(1, COLS, LINES - 1, 0);

  int pad_height = MAX_PROC_LIST_SIZE;
  WINDOW *proclist = newpad(pad_height, COLS);
  WINDOW *detail = newwin(LINES - 2, COLS, 1, 0);

  int list_top = 4;
  int list_bottom = LINES - 2;
  int visible_rows = list_bottom - list_top + 1;

  while (running) {
    state__poll_ebpf(&state, skel);

    if (state.view == PROC_LIST) {
      draw_proclist_header(header);
      draw_proclist_summary(summary, &state);
      draw_proclist_col_header(col_header);
      draw_proclist(proclist, &state, COLS);
      draw_proclist_footer(footer);

      prefresh(proclist, state.scroll_offset, 0,
               list_top, 0, list_bottom, COLS - 1);
    } else {
      draw_proclist_header(header);
      draw_detail_view(detail, &state);
      draw_detail_footer(footer);
    }
    doupdate();

    int c = getch();
    switch (c) {
    case ERR:
      continue;
    case 'q':
      if (state.view == DETAIL) {
        state.view = PROC_LIST;
        touchwin(summary);
        touchwin(col_header);
      } else {
        running = false;
      }
      break;
    case '\n':
    case KEY_ENTER:
      if (state.view == PROC_LIST) {
        state.view = DETAIL;
        werase(summary);
        werase(col_header);
      }
      break;
    case 27:
    case KEY_BACKSPACE:
    case 127:
      if (state.view == DETAIL) {
        state.view = PROC_LIST;
        touchwin(summary);
        touchwin(col_header);
      }
      break;
    case KEY_UP:
    case 'k':
      if (state.view == PROC_LIST && state.selected > 0) {
        state.selected--;
        if (state.selected < state.scroll_offset)
          state.scroll_offset = state.selected;
      }
      break;
    case KEY_DOWN:
    case 'j':
      if (state.view == PROC_LIST && state.selected < state.proc_count - 1) {
        state.selected++;
        if (state.selected >= state.scroll_offset + visible_rows)
          state.scroll_offset = state.selected - visible_rows + 1;
      }
      break;
    default:
      break;
    }
  }

  cleanup:
  kern_bpf__destroy(skel);
  endwin();
  return 0;
}
