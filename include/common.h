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

#endif
