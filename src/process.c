#include "process.h"
#include <stdio.h>

int resolve_proc_name(uint32_t tgid, char *buf, size_t len) {
  char buffer [100];
  int cx = snprintf(buffer, 100, "/proc/%d/cmdline", tgid);
  if (cx <= 0 || cx > 100) {
    return -1;
  }

  buffer[cx] = '\0';

  FILE *fp = fopen(buffer, "r");
  if (!fp) {
    return -1;
  }

  return 0;
}

int add(int x, int y) {
  return x + y;
}
