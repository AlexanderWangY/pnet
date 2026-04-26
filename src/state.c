#include "state.h"
#include <string.h>

void state__init_default_app(AppState *a) {
  a->view = PROC_LIST;
  a->proc_list_idx = 0;
  memset(a->proc_list, 0, sizeof(a->proc_list));
}
