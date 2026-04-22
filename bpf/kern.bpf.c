#include "vmlinux.h"
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_endian.h>

struct proc_meta {
  __u64 bytes_sent;
  __u64 bytes_recv;
  __u64 pkts_sent;
  __u64 pkts_recv;
  __u64 last_seen_ns;
  char comm[16];
};

struct {
  __uint(type, BPF_MAP_TYPE_HASH);
  __uint(max_entries, 4096);
  __type(key, __u32);
  __type(value, struct proc_meta);
} proc_stats SEC(".maps");


SEC("fexit/tcp_sendmsg")
int fexit__tcp_sendmsg(struct sock *sk, struct msghdr *msg, size_t size, int retval) {
  // retval is the # of bytes sent in this connection
  if (retval <= 0) {
    return 0;
  }

  __u64 pid = bpf_get_current_pid_tgid();
  __u32 tgid = pid << 32;

  struct proc_meta *pmeta = bpf_map_lookup_elem(&proc_stats, &tgid);
  if (pmeta) {
        __sync_fetch_and_add(&pmeta->bytes_sent, retval);
  } else {
    struct proc_meta new = {};
    new.bytes_sent = retval;
    bpf_get_current_comm(&new.comm, sizeof(new.comm));
    bpf_map_update_elem(&proc_stats, &tgid, &new, BPF_NOEXIST);
  }
  return 0;
}

char _license[] SEC("license") = "GPL";
