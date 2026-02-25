CC := g++
CFLAGS := -MMD -g -Wall -Wextra -Werror -Wno-missing-field-initializers
CPPFLAGS := -Iinclude -Ithirdparty/include -D__VK_BACKEND
LDFLAGS := -Lbuild
LIBS := -lSDL3 -lvulkan

BUILD := build
SRC_DIR := src
SRCS := $(shell find $(SRC_DIR) -name "*.c")
OBJS := $(SRCS:$(SRC_DIR)/%.c=$(BUILD)/%.o)

ENGINE_LIB := $(BUILD)/libengine.a
EXAMPLE := $(BUILD)/example
SHADERS := shaders/tri-frag.spv shaders/tri-vert.spv

.PHONY: all clean shaders example

all: shaders $(ENGINE_LIB)

shaders: $(SHADERS)

%.spv: shaders/tri.hlsl
	dxc -T $(if $(findstring frag,$@),ps_6_0,vs_6_0) -E $(if $(findstring frag,$@),MainFS,MainVS) -spirv $< -Fo $@

$(ENGINE_LIB): $(OBJS) $(BUILD)/vma.o
	ar rcs $@ $^

$(BUILD)/vma.o: src/vk/cpp/vk_mem_alloc.cpp
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) -g -c -o $@ $<

$(BUILD)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

example: shaders $(EXAMPLE)
	./$(EXAMPLE)

$(EXAMPLE): examples/main.c $(ENGINE_LIB)
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) $< -o $@ $(LDFLAGS) -lengine $(LIBS)

clean:
	rm -rf $(BUILD) $(SHADERS)

-include $(OBJS:.o=.d)
