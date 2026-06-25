#include "process.h"
#include <stdio.h>
#include <string.h>

int resolve_proc_name(uint32_t tgid, char *buf, size_t len) {
  char path[64];
  snprintf(path, sizeof(path), "/proc/%u/comm", tgid);

  FILE *fp = fopen(path, "r");
  if (!fp)
    return -1;

  char comm[64];
  if (!fgets(comm, sizeof(comm), fp)) {
    fclose(fp);
    return -1;
  }
  fclose(fp);

  size_t n = strlen(comm);
  if (n > 0 && comm[n - 1] == '\n')
    comm[n - 1] = '\0';

  strncpy(buf, comm, len - 1);
  buf[len - 1] = '\0';
  return 0;
}

void format_bytes(uint64_t bytes, char *buf, size_t len) {
  if (bytes >= 1073741824ULL)
    snprintf(buf, len, "%.1f GB", bytes / 1073741824.0);
  else if (bytes >= 1048576ULL)
    snprintf(buf, len, "%.1f MB", bytes / 1048576.0);
  else if (bytes >= 1024ULL)
    snprintf(buf, len, "%.1f KB", bytes / 1024.0);
  else
    snprintf(buf, len, "%llu B", (unsigned long long)bytes);
}

void format_rate(uint64_t bytes_per_sec, char *buf, size_t len) {
  if (bytes_per_sec >= 1073741824ULL)
    snprintf(buf, len, "%.1f GB/s", bytes_per_sec / 1073741824.0);
  else if (bytes_per_sec >= 1048576ULL)
    snprintf(buf, len, "%.1f MB/s", bytes_per_sec / 1048576.0);
  else if (bytes_per_sec >= 1024ULL)
    snprintf(buf, len, "%.1f KB/s", bytes_per_sec / 1024.0);
  else
    snprintf(buf, len, "%llu B/s", (unsigned long long)bytes_per_sec);
}

void format_ip(uint32_t ip, char *buf, size_t len) {
  snprintf(buf, len, "%u.%u.%u.%u",
           ip & 0xFF, (ip >> 8) & 0xFF,
           (ip >> 16) & 0xFF, (ip >> 24) & 0xFF);
}

int add(int x, int y) {
  return x + y;
}
