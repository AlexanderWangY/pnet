#include "ui.h"
#include "process.h"
#include <ncurses.h>
#include <string.h>

const col_def columns[COL_COUNT] = {
  [COL_PID]     = { "PID",      7,  0 },
  [COL_PROCESS] = { "PROCESS", 12,  1 },
  [COL_UP]      = { "UP",      16,  0 },
  [COL_DOWN]    = { "DOWN",    16,  0 },
  [COL_TX]      = { "TX",      16,  0 },
  [COL_RX]      = { "RX",      16,  0 },
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

void draw_proclist_summary(WINDOW *win, const AppState *state) {
  werase(win);

  char up_buf[32], down_buf[32];
  format_bytes(state->total_tx_bytes, up_buf, sizeof(up_buf));
  format_bytes(state->total_rx_bytes, down_buf, sizeof(down_buf));

  int offset = 1;

  wattron(win, COLOR_PAIR(CP_DEFAULT));
  mvwprintw(win, 0, offset, "Total:");
  wattroff(win, COLOR_PAIR(CP_DEFAULT));
  offset += 7;

  wattron(win, COLOR_PAIR(CP_GREEN));
  mvwprintw(win, 0, offset, "↑ %s", up_buf);
  wattroff(win, COLOR_PAIR(CP_GREEN));
  offset += 2 + (int)strlen(up_buf) + 3;

  wattron(win, COLOR_PAIR(CP_RED));
  mvwprintw(win, 0, offset, "↓ %s", down_buf);
  wattroff(win, COLOR_PAIR(CP_RED));
  offset += 2 + (int)strlen(down_buf) + 3;

  wattron(win, COLOR_PAIR(CP_CYAN));
  mvwprintw(win, 0, offset, "%d processes", state->proc_count);
  wattroff(win, COLOR_PAIR(CP_CYAN));

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

void draw_proclist(WINDOW *pad, const AppState *state, int pad_width) {
  werase(pad);

  int offsets[COL_COUNT], widths[COL_COUNT];
  col_layout(pad_width, offsets, widths);

  for (int i = 0; i < state->proc_count; i++) {
    const Process *p = &state->proc_list[i];

    if (i == state->selected) {
      wattron(pad, COLOR_PAIR(CP_SELECTED));
      for (int x = 0; x < pad_width; x++)
        mvwaddch(pad, i, x, ' ');
    }

    char buf[32];

    mvwprintw(pad, i, offsets[COL_PID] + 1, "%d", p->pid);
    mvwprintw(pad, i, offsets[COL_PROCESS] + 1, "%s", p->name);

    format_rate(p->tx_rate, buf, sizeof(buf));
    if (i != state->selected) wattron(pad, COLOR_PAIR(CP_GREEN));
    mvwprintw(pad, i, offsets[COL_UP] + 1, "%s", buf);
    if (i != state->selected) wattroff(pad, COLOR_PAIR(CP_GREEN));

    format_rate(p->rx_rate, buf, sizeof(buf));
    if (i != state->selected) wattron(pad, COLOR_PAIR(CP_RED));
    mvwprintw(pad, i, offsets[COL_DOWN] + 1, "%s", buf);
    if (i != state->selected) wattroff(pad, COLOR_PAIR(CP_RED));

    format_bytes(p->tx_bytes, buf, sizeof(buf));
    mvwprintw(pad, i, offsets[COL_TX] + 1, "%s", buf);

    format_bytes(p->rx_bytes, buf, sizeof(buf));
    mvwprintw(pad, i, offsets[COL_RX] + 1, "%s", buf);

    if (i == state->selected)
      wattroff(pad, COLOR_PAIR(CP_SELECTED));
  }
}

void draw_proclist_footer(WINDOW *win) {
  werase(win);
  wattron(win, COLOR_PAIR(CP_DEFAULT));
  mvwprintw(win, 0, 1, "↑↓ Navigate");
  mvwprintw(win, 0, 16, "Enter: detailed view");
  mvwprintw(win, 0, 40, "/: Filter");
  wnoutrefresh(win);
}

void draw_detail_view(WINDOW *win, const AppState *state) {
  werase(win);
  const Process *p = &state->proc_list[state->selected];
  char buf[32];
  int row = 1;
  int col = 3;

  wattron(win, COLOR_PAIR(CP_CYAN) | A_BOLD);
  mvwprintw(win, row, col, "%s", p->name);
  wattroff(win, A_BOLD);
  wprintw(win, "  (PID %d)", p->pid);
  wattroff(win, COLOR_PAIR(CP_CYAN));

  row += 2;
  wattron(win, COLOR_PAIR(CP_DEFAULT) | A_BOLD);
  mvwprintw(win, row, col, "── Total Traffic ──");
  wattroff(win, A_BOLD);

  row += 2;
  wattron(win, COLOR_PAIR(CP_GREEN));
  format_bytes(p->tx_bytes, buf, sizeof(buf));
  mvwprintw(win, row, col, "  Sent:        %s", buf);
  wattroff(win, COLOR_PAIR(CP_GREEN));

  row++;
  wattron(win, COLOR_PAIR(CP_RED));
  format_bytes(p->rx_bytes, buf, sizeof(buf));
  mvwprintw(win, row, col, "  Received:    %s", buf);
  wattroff(win, COLOR_PAIR(CP_RED));

  row += 2;
  wattron(win, COLOR_PAIR(CP_DEFAULT) | A_BOLD);
  mvwprintw(win, row, col, "── Connections (%d) ──", p->conn_count);
  wattroff(win, A_BOLD);

  row += 2;
  if (p->conn_count == 0) {
    wattron(win, COLOR_PAIR(CP_YELLOW));
    mvwprintw(win, row, col, "  No connections captured yet");
    wattroff(win, COLOR_PAIR(CP_YELLOW));
  }

  int max_row = getmaxy(win) - 1;
  for (int i = 0; i < p->conn_count && row < max_row; i++) {
    const Connection *c = &p->conns[i];
    char src_ip[16], dst_ip[16], sent[32], recv[32];
    format_ip(c->src_ip, src_ip, sizeof(src_ip));
    format_ip(c->dst_ip, dst_ip, sizeof(dst_ip));
    format_bytes(c->bytes_sent, sent, sizeof(sent));
    format_bytes(c->bytes_recv, recv, sizeof(recv));

    wattron(win, COLOR_PAIR(CP_YELLOW));
    mvwprintw(win, row, col, "  %s:%u", src_ip, c->src_port);
    wattroff(win, COLOR_PAIR(CP_YELLOW));

    wattron(win, COLOR_PAIR(CP_DEFAULT));
    wprintw(win, " → ");
    wattroff(win, COLOR_PAIR(CP_DEFAULT));

    wattron(win, COLOR_PAIR(CP_CYAN));
    wprintw(win, "%s:%u", dst_ip, c->dst_port);
    wattroff(win, COLOR_PAIR(CP_CYAN));

    row++;
    wattron(win, COLOR_PAIR(CP_GREEN));
    mvwprintw(win, row, col, "    ↑ %s", sent);
    wattroff(win, COLOR_PAIR(CP_GREEN));

    wattron(win, COLOR_PAIR(CP_RED));
    wprintw(win, "  ↓ %s", recv);
    wattroff(win, COLOR_PAIR(CP_RED));

    row += 2;
  }

  wnoutrefresh(win);
}

void draw_detail_footer(WINDOW *win) {
  werase(win);
  wattron(win, COLOR_PAIR(CP_DEFAULT));
  mvwprintw(win, 0, 1, "Esc/Backspace: back");
  mvwprintw(win, 0, 30, "[Q]uit");
  wattroff(win, COLOR_PAIR(CP_DEFAULT));
  wnoutrefresh(win);
}
