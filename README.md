# pnet

`pnet` is a lightweight eBPF (Extended Berkley Packet Filter) based process networking monitoring tool. It shows you in-depth information on packet transfer rates, totals, and more for each process.

## Prereqs

You must be on Linux for this to work. Preferably running kernel 5.15+ just to be 100% compatible with eBPF. Most modern distros work.

Furthermore, you must have llvm, clang, build-essential (gcc), libbpf, zlib1g, and bpftool installed on your system. You can probably find these through your package manager.


## Build

Everything needed to build can be found in the `Makefile`.
