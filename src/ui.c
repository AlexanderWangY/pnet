#include "ui.h"
#include <ncurses.h>
#include <string.h>

const col_def columns[COL_COUNT] = {
  [COL_PID]     = { "PID",      7,  0 },
  [COL_PROCESS] = { "PROCESS", 12,  1 },
  [COL_UP]      = { "UP",      10,  0 },
  [COL_DOWN]    = { "DOWN",    10,  0 },
  [COL_TX]      = { "TX",      10,  0 },
  [COL_RX]      = { "RX",      10,  0 },
};

void col_layout(int total_width, int offsets[COL_COUNT], int widths[COL_COUNT]) {
  int fixed = 0;
  int total_weight = 0;
  for (int i = 0; i < COL_COUNT; i++) {
    if (columns[i].weight == 0)
      fixed += columns[i].min_width;
    else
      total_weight += columns[i].weight;
  }

  int flex = total_width - fixed;
  if (flex < 0) flex = 0;

  int pos = 0;
  for (int i = 0; i < COL_COUNT; i++) {
    offsets[i] = pos;
    if (columns[i].weight == 0)
      widths[i] = columns[i].min_width;
    else
      widths[i] = (total_weight > 0) ? (flex * columns[i].weight / total_weight) : columns[i].min_width;
    pos += widths[i];
  }
}

void draw_proclist_header(WINDOW *win) {
  werase(win);
  wattron(win, COLOR_PAIR(CP_DEFAULT) | A_BOLD);
  mvwprintw(win, 0, 1, "pnet");
  wattroff(win, A_BOLD);
  mvwprintw(win, 0, getmaxx(win) - 26, "Sort: [B] [N] [P]  [Q]uit");
  wnoutrefresh(win);
}

void draw_proclist_summary(WINDOW *win) {
  werase(win);

  //TODO: remove hard coded and add app state function
  const char *up_val = "12 MB/s";
  const char *down_val = "8 KB/s";
  int proc_count = 14;

  
  int offset = 1;

  // Label
  wattron(win, COLOR_PAIR(CP_DEFAULT));
  mvwprintw(win, 0, offset, "Total:");
  wattroff(win, COLOR_PAIR(CP_DEFAULT));

  offset += 7;

  // Up speed  
  wattron(win, COLOR_PAIR(CP_GREEN));
  mvwprintw(win, 0, offset, "↑ %s", up_val);
  wattroff(win, COLOR_PAIR(CP_GREEN));

  offset += 2 + strlen(up_val) + 3;

  wattron(win, COLOR_PAIR(CP_RED));
  mvwprintw(win, 0, offset, "↓ %s", down_val);
  wattroff(win, COLOR_PAIR(CP_RED));

  wnoutrefresh(win);
}

void draw_proclist_col_header(WINDOW *win) {
  int offsets[COL_COUNT], widths[COL_COUNT];
  col_layout(getmaxx(win), offsets, widths);

  werase(win);
  wattron(win, COLOR_PAIR(CP_INVERTED));

  int maxcol = getmaxx(win);
  for (int x = 0; x < maxcol; x++)
    mvwaddch(win, 0, x, ' ');

  for (int i = 0; i < COL_COUNT; i++)
    mvwprintw(win, 0, offsets[i] + 1, "%s", columns[i].label);

  wattroff(win, COLOR_PAIR(CP_INVERTED));
  wnoutrefresh(win);
}

void draw_proclist_footer(WINDOW *win) {
  werase(win);
  wattron(win, COLOR_PAIR(CP_DEFAULT));
  mvwprintw(win, 0, 1, "\u2191\u2193 Navigate");
  mvwprintw(win, 0, 16, "Enter: detailed view");
  mvwprintw(win, 0, 40, "/: Filter");
  wnoutrefresh(win);
}


