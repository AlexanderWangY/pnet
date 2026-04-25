#include "ui.h"
#include "ncurses.h"

void draw_proclist_header(WINDOW *win) {
  werase(win);
  wattron(win, COLOR_PAIR(CP_DEFAULT) | A_BOLD);
  mvwprintw(win, 0, 1, "pnet");
  wattroff(win, A_BOLD);
  wattron(win, COLOR_PAIR(CP_INVERTED));
  mvwprintw(win, 0, getmaxx(win) - 30, "Sort: [B] [N] [P]  [Q]uit");
  wattroff(win, COLOR_PAIR(CP_INVERTED));
  wnoutrefresh(win);
}

void draw_proclist_summary(WINDOW *win) {
  werase(win);
  wattron(win, COLOR_PAIR(CP_DEFAULT));
  mvwprintw(win, 0, 1, "Total Up: ");
  mvwprintw(win, 0, 11, "12 MB/s  ");
  mvwprintw(win, 0, 25, "Total Down: ");
  mvwprintw(win, 0, 37, "8 MB/s  ");
  mvwprintw(win, 0, 50, "Processes: 14");
  wnoutrefresh(win);
}

void draw_proclist_col_header(WINDOW *win) {
  werase(win);
  wattron(win, COLOR_PAIR(CP_DEFAULT));
  mvwprintw(win, 0, 1, "PID");
  mvwprintw(win, 0, 11, "Process");
  mvwprintw(win, 0, 25, "Up");
  mvwprintw(win, 0, 37, "Down");
  mvwprintw(win, 0, 50, "Total TX");
  mvwprintw(win, 0, 65, "Total RX");
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


