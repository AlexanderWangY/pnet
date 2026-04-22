#include "kern.skel.h"
#include <bpf/libbpf.h>
#include <ncurses.h>
#include <net/if.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

static volatile int running = 1;

void signal_handler(int _) {
  running = 0;
}

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <interface>\n", argv[0]);
    return 1;
  }

  const char *ifname = argv[1];
  int ifindex = if_nametoindex(ifname);
  if (!ifindex) {
    fprintf(stderr, "Failed to get interface index for %s.\n", ifname);
    return 1;
  }

  signal(SIGINT, signal_handler);

  struct kern_bpf *skel;

  skel = kern_bpf__open_and_load();
  if (!skel) {
    fprintf(stderr, "Failed to load skeleton.\n");
    return 1;
  }

  if (kern_bpf__attach(skel)) {
    fprintf(stderr, "Failed to attach skeleton.\n");
    goto cleanup;
  }

  // if (bpf_xdp_attach(ifindex, bpf_program__fd(skel->progs.count_packets), 0, NULL) < 0) {
  //   fprintf(stderr, "Failed to attach XDP to %s.\n", ifname);
  //   goto cleanup;
  // }

  // __u32 key = 0;
  // __u64 value = 0;

  // initscr();

  // while (running) {
  //   int err = bpf_map__lookup_elem(skel->maps.packet_count, &key, sizeof(key),
  //                                  &value, sizeof(value), 0);
  //   if (err != 0) continue;
  //   clear();
  //   printw("Packets processed: %llu", value);
  //   refresh();
  //   sleep(1);
  // }

  // endwin();

cleanup:
  // bpf_xdp_detach(ifindex, 0, NULL);
  kern_bpf__destroy(skel);
  return 0;
}
