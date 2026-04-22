CC := gcc
CFLAGS := -Wall -Wextra -O2

PKG_CFLAGS := $(shell pkg-config --cflags ncursesw)
PKG_LIBS := $(shell pkg-config --libs ncursesw)

# BPF settings
CLANG := clang
BPFTOOL := bpftool
ARCH := $(shell uname -m | sed 's/x86_64/x86/' | sed 's/aarch64/arm64/')
BPF_CFLAGS := -target bpf -O2 -g -D__TARGET_ARCH_$(ARCH)

SRC_DIR := src
BUILD_DIR := build
BPF_DIR := bpf

SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))
TARGET := $(BUILD_DIR)/pnet

BPF_SRC := $(BPF_DIR)/kern.bpf.c
BPF_OBJ := $(BUILD_DIR)/kern.bpf.o
BPF_SKEL := $(SRC_DIR)/kern.skel.h

all: $(TARGET)

# BPF compilation
$(BPF_OBJ): $(BPF_SRC) $(BPF_DIR)/vmlinux.h | $(BUILD_DIR)
	$(CLANG) $(BPF_CFLAGS) -I$(BPF_DIR) -c $< -o $@

$(BPF_SKEL): $(BPF_OBJ)
	$(BPFTOOL) gen skeleton $< > $@

# Userspace compilation (depends on skeleton)
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c $(BPF_SKEL) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(PKG_CFLAGS) -c $< -o $@

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ $(PKG_LIBS) -lbpf -lelf -lz

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -rf $(BUILD_DIR) $(BPF_SKEL)

.PHONY: all run clean
