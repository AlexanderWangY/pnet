#include "state.h"
#include "common.h"
#include <bpf/libbpf.h>
#include <bpf/bpf.h>
#include <string.h>

void state__init_default_app(AppState *a) {
  a->view = PROC_LIST;
  a->proc_count = 0;
  a->selected = 0;
  a->scroll_offset = 0;
  a->total_tx_bytes = 0;
  a->total_rx_bytes = 0;
  memset(a->proc_list, 0, sizeof(a->proc_list));
}

static int find_proc_by_pid(AppState *a, uint32_t pid) {
  for (int i = 0; i < a->proc_count; i++) {
    if (a->proc_list[i].pid == pid)
      return i;
  }
  return -1;
}

void state__poll_ebpf(AppState *a, struct kern_bpf *skel) {
  int prev_selected_pid = -1;
  if (a->proc_count > 0 && a->selected < a->proc_count)
    prev_selected_pid = (int)a->proc_list[a->selected].pid;

  a->proc_count = 0;
  a->total_tx_bytes = 0;
  a->total_rx_bytes = 0;

  int proc_fd = bpf_map__fd(skel->maps.proc_stats);
  __u32 key = 0, next_key = 0;

  while (bpf_map_get_next_key(proc_fd, &key, &next_key) == 0) {
    key = next_key;
    struct proc_meta pm;
    if (bpf_map_lookup_elem(proc_fd, &key, &pm) != 0)
      continue;

    if (a->proc_count >= MAX_PROC_LIST_SIZE)
      break;

    Process *p = &a->proc_list[a->proc_count];
    memset(p, 0, sizeof(*p));
    p->pid = key;
    p->tx_bytes = pm.bytes_sent;
    p->rx_bytes = pm.bytes_recv;
    p->conn_count = 0;

    if (resolve_proc_name(key, p->name, sizeof(p->name)) != 0)
      strncpy(p->name, pm.comm, sizeof(p->name) - 1);

    a->total_tx_bytes += pm.bytes_sent;
    a->total_rx_bytes += pm.bytes_recv;
    a->proc_count++;
  }

  int conn_fd = bpf_map__fd(skel->maps.conn_stats);
  struct conn_key ck = {}, next_ck = {};

  while (bpf_map_get_next_key(conn_fd, &ck, &next_ck) == 0) {
    ck = next_ck;
    struct conn_meta cm;
    if (bpf_map_lookup_elem(conn_fd, &ck, &cm) != 0)
      continue;

    int idx = find_proc_by_pid(a, ck.tgid);
    if (idx < 0)
      continue;

    Process *p = &a->proc_list[idx];
    if (p->conn_count >= MAX_CONNECTIONS)
      continue;

    Connection *c = &p->conns[p->conn_count++];
    c->src_ip = ck.src_ip;
    c->dst_ip = ck.dst_ip;
    c->src_port = ck.src_port;
    c->dst_port = ck.dst_port;
    c->bytes_sent = cm.bytes_sent;
    c->bytes_recv = cm.bytes_recv;
  }

  if (prev_selected_pid >= 0) {
    int new_idx = find_proc_by_pid(a, (uint32_t)prev_selected_pid);
    if (new_idx >= 0)
      a->selected = new_idx;
  }
  if (a->selected >= a->proc_count && a->proc_count > 0)
    a->selected = a->proc_count - 1;
}
