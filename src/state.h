#ifndef STATE_H
#define STATE_H

#include "process.h"

typedef enum View {
  PROC_LIST,
  DETAIL,  
} View ;

typedef struct {
  View view;
  Process *proc_list;
} AppState;

#endif
