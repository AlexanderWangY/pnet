#include "kern.skel.h"
#include <bpf/libbpf.h>
#include <ncurses.h>
#include <net/if.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>

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
    //TODO: Run through map with get next key, collect in array, then sort and print
  }


cleanup:
  kern_bpf__destroy(skel);
  return 0;
}
