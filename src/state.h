#ifndef STATE_H
#define STATE_H

#include "process.h"
#include <stdint.h>

#define MAX_PROC_LIST_SIZE 4096

typedef enum View {
  PROC_LIST,
  DETAIL,  
} View ;

typedef struct {
  View view;
  Process proc_list[MAX_PROC_LIST_SIZE];
  uint64_t proc_list_idx;
} AppState;

void state__init_default_app(AppState *a);

#endif
