#include "common.h"
#include "kern.skel.h"
#include <bpf/bpf_helpers.h>
#include <bpf/libbpf.h>
#include <ncurses.h>
#include <net/if.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

// Used to supress GCC/CLANG warnings
// https://stackoverflow.com/questions/3599160/how-can-i-suppress-unused-parameter-warnings-in-c/3599170#3599170
#define UNUSED(x) (void)(x)

static volatile bool running = true;

void signal_handler(int sig) {
  UNUSED(sig);
  running = false;
}

int main(int argc, char **argv) {
  // These will be used later for CLI flags
  UNUSED(argc);
  UNUSED(argv);

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

  while (running) {
    __u32 key, next_key;
    __u32 *prev_key = NULL;

    while (bpf_map__get_next_key(skel->maps.proc_stats, prev_key, &next_key, sizeof(next_key)) == 0) {
      key = next_key;

      struct proc_meta pmeta;
      err = bpf_map__lookup_elem(skel->maps.proc_stats, &key, sizeof(key), &pmeta, sizeof(pmeta), 0);
      if (err) {
        prev_key = &key;
        continue;
      }

      printf("[TGID %u] comm=%-16s bytes_sent=%-10llu bytes_recv=%-10llu "
             "send_calls=%-8llu recv_calls=%-8llu last_seen_ns=%llu\n",
             key, pmeta.comm,
             (unsigned long long)pmeta.bytes_sent,
             (unsigned long long)pmeta.bytes_recv,
             (unsigned long long)pmeta.send_calls,
             (unsigned long long)pmeta.recv_calls,
             (unsigned long long)pmeta.last_seen_ns);

      prev_key = &key;
    }
    printf("---\n");
  }

cleanup:
  kern_bpf__destroy(skel);
  return 0;
}
