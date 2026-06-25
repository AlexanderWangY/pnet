#ifndef STATE_H
#define STATE_H

#include "process.h"
#include "kern.skel.h"
#include <stdint.h>

#define MAX_PROC_LIST_SIZE 4096

typedef enum View {
  PROC_LIST,
  DETAIL,
} View ;

typedef struct {
  View view;
  Process proc_list[MAX_PROC_LIST_SIZE];
  int proc_count;
  int selected;
  int scroll_offset;
  uint64_t total_tx_bytes;
  uint64_t total_rx_bytes;
} AppState;

void state__init_default_app(AppState *a);
void state__poll_ebpf(AppState *a, struct kern_bpf *skel);

#endif
