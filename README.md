# pnet

**Work in Progress**

eBPF-based per-process network monitor for Linux. Tracks TCP bytes sent/received per process using kernel hooks.

## Requirements

- Linux kernel 5.15+
- clang, gcc, libbpf, bpftool, ncurses, zlib

## Build

```
make
```

## Usage

```
sudo ./build/pnet <interface>
```

## License

GPL
