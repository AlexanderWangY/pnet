#ifndef PROCESS_H
#define PROCESS_H

#include <stdint.h>

typedef struct {
  char name[64];
  uint64_t tx_rate; // Bytes per second
  uint64_t rx_rate; // ^^^^^
  uint64_t tx_bytes;
  uint64_t rx_bytes;
} Process;

#endif
