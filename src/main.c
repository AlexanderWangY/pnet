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
}

static volatile bool running = true;

void signal_handler(int sig) {
  UNUSED(sig);
  running = false;
}

int main() {
  setlocale(LC_ALL, "");

  // Init app state
  AppState *state = {0};
  state__init_default_app(state);

  // Init eBPF skeleton
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

  while (running) {
    
    draw_proclist_header(header);
    draw_proclist_summary(summary);
    draw_proclist_col_header(col_header);
    draw_proclist_footer(footer);
    doupdate();
    
    char c = getch();
    switch (c) {
    case ERR:
      continue;
    case 'q':
      running = false;
      break;
    default:
      continue;
    }
  }

  cleanup:
  kern_bpf__destroy(skel);
  endwin();
  return 0;
}

// // Used to supress GCC/CLANG warnings
// //
// https://stackoverflow.com/questions/3599160/how-can-i-suppress-unused-parameter-warnings-in-c/3599170#3599170
// #define UNUSED(x) (void)(x)

// static volatile bool running = true;

// void signal_handler(int sig) {
//   UNUSED(sig);
//   running = false;
// }

// int main(int argc, char **argv) {
//   // These will be used later for CLI flags
//   UNUSED(argc);
//   UNUSED(argv);

//   struct kern_bpf *skel;
//   int err;

//   signal(SIGINT, signal_handler);
//   signal(SIGTERM, signal_handler);

//   skel = kern_bpf__open_and_load();
//   if (!skel) {
//     fprintf(stderr, "Failed to load skeleton.\n");
//     err = 1;
//     goto cleanup;
//   }

//   err = kern_bpf__attach(skel);
//   if (err) {
//     fprintf(stderr, "Failed to attach skeleton.\n");
//     goto cleanup;
//   }

//   while (running) {
//     __u32 key, next_key;
//     __u32 *prev_key = NULL;

//     while (bpf_map__get_next_key(skel->maps.proc_stats, prev_key, &next_key,
//     sizeof(next_key)) == 0) {
//       key = next_key;

//       struct proc_meta pmeta;
//       err = bpf_map__lookup_elem(skel->maps.proc_stats, &key, sizeof(key),
//       &pmeta, sizeof(pmeta), 0); if (err) {
//         prev_key = &key;
//         continue;
//       }

//       printf("[TGID %u] comm=%-16s bytes_sent=%-10llu bytes_recv=%-10llu "
//              "send_calls=%-8llu recv_calls=%-8llu last_seen_ns=%llu\n",
//              key, pmeta.comm,
//              (unsigned long long)pmeta.bytes_sent,
//              (unsigned long long)pmeta.bytes_recv,
//              (unsigned long long)pmeta.send_calls,
//              (unsigned long long)pmeta.recv_calls,
//              (unsigned long long)pmeta.last_seen_ns);

//       prev_key = &key;
//     }
//     printf("---\n");
//   }

// cleanup:
//   kern_bpf__destroy(skel);
//   return 0;
// }
