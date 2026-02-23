CC=g++
CFLAGS_NOWARN=-g
CFLAGS=$(CFLAGS_NOWARN) -Wall -Wextra -Werror -Wno-missing-field-initializers
BUILD=build
INCLUDE=-Iinclude -Ithirdparty/include
LIBS_NOVMA=-lSDL3 -lvulkan
LIBS=-L$(BUILD) $(LIBS_NOVMA) -lvma

SRCS=$(shell find src -name "*.c")
OBJS=$(SRCS:src/%.c=$(BUILD)/%.o)

VMA_TARGET=build/libvma.a

# NOTE: This is temporary for just compiling triangle shaders.
# TODO: We need a more robust way of compiling all shaders.
SHADERS=shaders/tri-frag.spv shaders/tri-vert.spv

.PHONY: all run clean shaders

all: $(VMA_TARGET) shaders game

shaders: $(SHADERS)

run: all
	./game

$(VMA_TARGET): src/vk/cpp/vk_mem_alloc.cpp
	g++ $(INCLUDE) -D__VK_BACKEND $(CFLAGS_NOWARN) -c -o build/vma.o src/vk/cpp/vk_mem_alloc.cpp $(LIBS_NOVMA)
	ar rcs build/libvma.a build/vma.o

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
