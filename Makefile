CC        := gcc
STD       := -std=c11
WARN      := -Wall -Wextra -Wpedantic -Wshadow -Wstrict-prototypes \
             -Wmissing-prototypes -Wno-unused-parameter
INCLUDE   := -Iinclude
COMMON    := $(STD) $(WARN) $(INCLUDE) -D_POSIX_C_SOURCE=200809L

CFLAGS_RELEASE := $(COMMON) -O2 -DNDEBUG

CFLAGS_DEBUG   := $(COMMON) -O0 -g3 -fsanitize=address,undefined \
                   -fno-omit-frame-pointer

CFLAGS    ?= $(CFLAGS_RELEASE)
LDFLAGS   ?=

ifeq ($(MAKECMDGOALS),debug)
CFLAGS  := $(CFLAGS_DEBUG)
LDFLAGS := -fsanitize=address,undefined
endif

SRC_DIR := src
INC_DIR := include
OBJ_DIR := build
BIN_DIR := bin

SOURCES := $(wildcard $(SRC_DIR)/*.c)
OBJECTS := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SOURCES))
TARGET  := $(BIN_DIR)/jpdict

.PHONY: all release debug run clean help

all: release

release: $(TARGET)
debug:   $(TARGET)

$(TARGET): $(OBJECTS) | $(BIN_DIR)
	@echo "  LINK  $@"
	@$(CC) $(OBJECTS) -o $@ $(LDFLAGS)
	@echo "  ✓ Built $@"

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	@echo "  CC    $<"
	@$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR) $(BIN_DIR):
	@mkdir -p $@

run: $(TARGET)
	@./$(TARGET)

clean:
	@echo "  RM    $(OBJ_DIR)  $(BIN_DIR)"
	@rm -rf $(OBJ_DIR) $(BIN_DIR)

help:
	@echo "可用目標:"
	@echo "  make           編譯 release 版（預設）"
	@echo "  make debug     編譯 debug 版（含 sanitizers）"
	@echo "  make run       編譯並執行 demo"
	@echo "  make clean     清除所有編譯產物"
	@echo "  make help      顯示此說明"

$(OBJECTS): $(wildcard $(INC_DIR)/*.h)