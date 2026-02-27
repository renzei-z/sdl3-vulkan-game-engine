CC := gcc
CFLAGS := -DEMM_ASSERT_ALIGNMENTS -Iinclude -std=c23 -g -Wall -Wextra -Werror -Wno-missing-field-initializers -Wno-missing-braces
LIBS := -lm

SRC_DIR := src
SRCS := $(shell find $(SRC_DIR) -name "*.c")
BIN := bin
BUILD := build
OBJS := $(SRCS:$(SRC_DIR)/%.c=$(BUILD)/%.o)

TARGET := $(BIN)/libemm.a

.PHONY: lib example clean

lib: $(TARGET)

example: lib examples/main.c
	$(CC) $(CFLAGS) -o bin/example examples/main.c $(LIBS) -Lbin -lemm
	./bin/example

$(TARGET): $(OBJS)
	@mkdir -p $(dir $@)
	ar rcs $@ $<

$(BUILD)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf $(BIN)
	rm -rf $(BUILD)
