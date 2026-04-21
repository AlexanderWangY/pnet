#include "vmlinux.h"
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_endian.h>

struct {
  __uint(type, BPF_MAP_TYPE_HASH);
  __uint(max_entries, 1024);
  __type(key, __u32);
  __type(value, __u64);
} packet_count SEC(".maps");

SEC("xdp")
int count_packets(struct xdp_md *ctx) {
  __u32 key = 0;
  __u64 *val = bpf_map_lookup_elem(&packet_count, &key);

  if (val) {
    __sync_fetch_and_add(val, 1);
  } else {
    __u64 init = 1;
    bpf_map_update_elem(&packet_count, &key, &init, BPF_ANY);
  }
  return XDP_DROP;
}

char _license[] SEC("license") = "GPL";
