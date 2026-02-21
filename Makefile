CC=gcc
CFLAGS=-g -Wall -Wextra -Werror
INCLUDE=-Iinclude
LIBS=-lSDL3 -lvulkan

BUILD=build

SRCS=$(shell find src -name "*.c")
OBJS=$(SRCS:src/%.c=$(BUILD)/%.o)

# NOTE: This is temporary for just compiling triangle shaders.
# TODO: We need a more robust way of compiling all shaders.
SHADERS=shaders/tri-frag.spv shaders/tri-vert.spv

.PHONY: all run clean shaders

all: game shaders

shaders: $(SHADERS)

run: all
	./game

shaders/tri-frag.spv: shaders/tri.hlsl
	dxc -T ps_6_0 -E MainFS -spirv $^ -Fo $@

shaders/tri-vert.spv: shaders/tri.hlsl
	dxc -T vs_6_0 -E MainVS -spirv $^ -Fo $@

game: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

$(BUILD)/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(INCLUDE) -D__VK_BACKEND $(CFLAGS) -c -o $@ $^

clean:
	rm -f $(SHADERS)
	rm -rf $(BUILD)
	rm -f game
