#ifndef PROCESS_H
#define PROCESS_H

#include <stddef.h>
#include <stdint.h>

typedef struct {
  char name[64];
  uint64_t tx_rate; // Bytes per second
  uint64_t rx_rate; // ^^^^^
  uint64_t tx_bytes;
  uint64_t rx_bytes;
} Process;

int resolve_proc_name(uint32_t tgid, char* buf, size_t len);

int add(int x, int y);

#endif
