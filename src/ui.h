#ifndef UI_H
#define UI_H

#include <ncurses.h>

#define CP_DEFAULT    1
#define CP_INVERTED   2
#define CP_GREEN      3
#define CP_RED        4
#define CP_YELLOW     5
#define CP_CYAN       6
#define CP_BLUE       7
#define CP_MAGENTA    8
#define CP_GREEN_INV  9
#define CP_RED_INV   10

void draw_proclist_header(WINDOW *win);
void draw_proclist_summary(WINDOW *win);
void draw_proclist_col_header(WINDOW *win);
void draw_proclist(WINDOW *win);
void draw_proclist_footer(WINDOW *win);

#endif
