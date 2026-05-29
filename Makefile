CC := clang
CLANG := clang
BPFTOOL := bpftool

# Directories
SRC_DIR := src
BUILD_DIR := build
BPF_DIR := bpf
TEST_DIR := test
UNITY_DIR := unity

# Architecture (for BPF CO-RE)
ARCH := $(shell uname -m | sed 's/x86_64/x86/' | sed 's/aarch64/arm64/')

# Userspace flags
CFLAGS := -Wall -Wextra -O2
PKG_CFLAGS := $(shell pkg-config --cflags ncursesw)
PKG_LIBS := $(shell pkg-config --libs ncursesw)
LIBBPF_CFLAGS := $(shell pkg-config --cflags libbpf)
LIBBPF_LIBS := $(shell pkg-config --libs libbpf)

# BPF flags
BPF_CFLAGS := \
	-target bpf \
	-O2 -g \
	-D__TARGET_ARCH_$(ARCH) \
	-I$(BPF_DIR) \
	-Iinclude \
	$(LIBBPF_CFLAGS)

# Sources
SRCS := $(wildcard $(SRC_DIR)/*.c)
TEST_SRCS := $(wildcard $(TEST_DIR)/*.c)
OBJS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))
TEST_BINS := $(patsubst $(TEST_DIR)/%.c,$(BUILD_DIR)/%,$(TEST_SRCS))
TARGET := $(BUILD_DIR)/pnet

# BPF sources
BPF_SRC := $(BPF_DIR)/kern.bpf.c
BPF_OBJ := $(BUILD_DIR)/kern.bpf.o
BPF_SKEL := $(SRC_DIR)/kern.skel.h

# ──────────────────────────────────────────────
.PHONY: all run clean test

all: $(TARGET)

# 1. Compile BPF object
$(BPF_OBJ): $(BPF_SRC) $(BPF_DIR)/vmlinux.h | $(BUILD_DIR)
	$(CLANG) $(BPF_CFLAGS) -c $< -o $@

# 2. Generate skeleton from BPF object
$(BPF_SKEL): $(BPF_OBJ)
	$(BPFTOOL) gen skeleton $< > $@

# 3. Compile userspace objects (depend on skeleton)
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c $(BPF_SKEL) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(PKG_CFLAGS) $(LIBBPF_CFLAGS) -Iinclude -I$(SRC_DIR) -c $< -o $@

# 4. Link final binary
$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ $(PKG_LIBS) $(LIBBPF_LIBS) -lelf -lz

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/test_%: $(TEST_DIR)/test_%.c $(SRC_DIR)/process.c $(UNITY_DIR)/unity.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -I$(SRC_DIR) -I$(UNITY_DIR) $^ -o $@

test: $(TEST_BINS)
	@for t in $(TEST_BINS); do echo "=== $$t ==="; ./$$t; done

run: $(TARGET)
	sudo $(TARGET)

clean:
	rm -rf $(BUILD_DIR) $(BPF_SKEL)
