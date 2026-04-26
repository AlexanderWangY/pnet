# pnet

**Work in Progress**

An eBPF-powered terminal network monitor for Linux. Uses `fexit` hooks on `tcp_sendmsg_locked` and `tcp_cleanup_rbuf` to track per-process TCP traffic, then displays it in an interactive ncurses TUI — similar to `htop` but for network I/O.

Tracks per-process: bytes sent, bytes received, send call count, recv call count.

## Requirements

- Linux kernel 5.15+
- clang, gcc, libbpf, bpftool, ncurses, zlib

## Build

```
make
```

## Usage

```
sudo ./build/pnet
```

## License

GPL
