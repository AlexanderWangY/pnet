#ifndef PROCESS_H
#define PROCESS_H

#include <stddef.h>
#include <stdint.h>

#define MAX_CONNECTIONS 64

typedef struct {
  uint32_t src_ip;
  uint32_t dst_ip;
  uint16_t src_port;
  uint16_t dst_port;
  uint64_t bytes_sent;
  uint64_t bytes_recv;
} Connection;

typedef struct {
  uint32_t pid;
  char name[64];
  uint64_t tx_rate;
  uint64_t rx_rate;
  uint64_t tx_bytes;
  uint64_t rx_bytes;
  Connection conns[MAX_CONNECTIONS];
  int conn_count;
} Process;

int resolve_proc_name(uint32_t tgid, char *buf, size_t len);
void format_bytes(uint64_t bytes, char *buf, size_t len);
void format_rate(uint64_t bytes_per_sec, char *buf, size_t len);
void format_ip(uint32_t ip, char *buf, size_t len);

int add(int x, int y);

#endif
