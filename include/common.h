#ifndef __PNET_COMMON_H
#define __PNET_COMMON_H

#ifndef __VMLINUX_H__
#include <linux/types.h>
#endif

struct proc_meta {
  __u64 bytes_sent;
  __u64 bytes_recv;
  __u64 send_calls;
  __u64 recv_calls;
  __u64 last_seen_ns;
  char comm[16];
};

struct conn_key {
  __u32 tgid;
  __u32 src_ip;
  __u32 dst_ip;
  __u16 src_port;
  __u16 dst_port;
};

struct conn_meta {
  __u64 bytes_sent;
  __u64 bytes_recv;
};

#endif
