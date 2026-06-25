#include "vmlinux.h"
#include <bpf/bpf_tracing.h>
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_endian.h>
#include <bpf/bpf_core_read.h>
#include "common.h"

struct {
  __uint(type, BPF_MAP_TYPE_HASH);
  __uint(max_entries, 4096);
  __type(key, __u32);
  __type(value, struct proc_meta);
} proc_stats SEC(".maps");

struct {
  __uint(type, BPF_MAP_TYPE_HASH);
  __uint(max_entries, 16384);
  __type(key, struct conn_key);
  __type(value, struct conn_meta);
} conn_stats SEC(".maps");

static __always_inline void update_conn(struct sock *sk, __u32 tgid, int sent, int recv) {
  __u16 family = sk->__sk_common.skc_family;
  if (family != 2)
    return;

  struct conn_key ck = {};
  ck.tgid = tgid;
  ck.src_ip = sk->__sk_common.skc_rcv_saddr;
  ck.dst_ip = sk->__sk_common.skc_daddr;
  ck.src_port = sk->__sk_common.skc_num;
  ck.dst_port = bpf_ntohs(sk->__sk_common.skc_dport);

  struct conn_meta *cm = bpf_map_lookup_elem(&conn_stats, &ck);
  if (cm) {
    if (sent > 0)
      __sync_fetch_and_add(&cm->bytes_sent, (__u64)sent);
    if (recv > 0)
      __sync_fetch_and_add(&cm->bytes_recv, (__u64)recv);
  } else {
    struct conn_meta new = {};
    if (sent > 0)
      new.bytes_sent = sent;
    if (recv > 0)
      new.bytes_recv = recv;
    bpf_map_update_elem(&conn_stats, &ck, &new, BPF_NOEXIST);
  }
}

SEC("fexit/tcp_sendmsg_locked")
int BPF_PROG(fexit__tcp_sendmsg, struct sock *sk, struct msghdr *msg, size_t size, int retval) {
  if (retval <= 0) return 0;

  __u64 pid = bpf_get_current_pid_tgid();
  __u32 tgid = pid >> 32;
  struct proc_meta *pmeta = bpf_map_lookup_elem(&proc_stats, &tgid);
  if (pmeta) {
    __sync_fetch_and_add(&pmeta->bytes_sent, (__u64)retval);
    __sync_fetch_and_add(&pmeta->send_calls, 1);
  } else {
    struct proc_meta new = {};
    new.bytes_sent = retval;
    new.send_calls = 1;
    bpf_get_current_comm(&new.comm, sizeof(new.comm));
    bpf_map_update_elem(&proc_stats, &tgid, &new, BPF_NOEXIST);
  }

  update_conn(sk, tgid, retval, 0);
  return 0;
}

SEC("fexit/tcp_cleanup_rbuf")
int BPF_PROG(fexit__tcp_cleanup_rbuf, struct sock *sk, int copied) {
  if (copied <= 0) return 0;

  __u64 pid = bpf_get_current_pid_tgid();
  __u32 tgid = pid >> 32;
  struct proc_meta *pmeta = bpf_map_lookup_elem(&proc_stats, &tgid);
  if (pmeta) {
    __sync_fetch_and_add(&pmeta->bytes_recv, copied);
    __sync_fetch_and_add(&pmeta->recv_calls, 1);
  } else {
    struct proc_meta new = {};
    new.bytes_recv = copied;
    new.recv_calls = 1;
    bpf_get_current_comm(&new.comm, sizeof(new.comm));
    bpf_map_update_elem(&proc_stats, &tgid, &new, BPF_NOEXIST);
  }

  update_conn(sk, tgid, 0, copied);
  return 0;
}

char _license[] SEC("license") = "GPL";
