CC := gcc
CPP_CC := g++
MAKE := make
CFLAGS := -MMD -g -Wall -Wextra -Werror -Wno-missing-field-initializers -Wno-missing-braces
CPPFLAGS := -Iinclude -Ilibs/emm -D__VK_BACKEND
LDFLAGS := -Lbuild -Llibs/emm/bin
LIBS := -lSDL3 -lvulkan -lm -lemm

BUILD := build
SRC_DIR := src
SRCS := $(shell find $(SRC_DIR) -name "*.c")
OBJS := $(SRCS:$(SRC_DIR)/%.c=$(BUILD)/%.o)

ENGINE_LIB := $(BUILD)/libengine.a
MATH_LIB := libs/emm/bin/libemm.a
EXAMPLE := $(BUILD)/example
SHADERS := shaders/tri-frag.spv shaders/tri-vert.spv

.PHONY: all clean shaders example

all: shaders $(ENGINE_LIB)

shaders: $(SHADERS)

%.spv: shaders/tri.hlsl
	dxc -T $(if $(findstring frag,$@),ps_6_0,vs_6_0) -E $(if $(findstring frag,$@),MainFS,MainVS) -spirv $< -Fo $@

$(ENGINE_LIB): $(OBJS) $(BUILD)/vma.o
	ar rcs $@ $^

$(MATH_LIB): libs/emm/Makefile
	$(MAKE) -C libs/emm

$(BUILD)/vma.o: src/vk/cpp/vk_mem_alloc.cpp
	@mkdir -p $(dir $@)
	$(CPP_CC) $(CPPFLAGS) -g -c -o $@ $<

$(BUILD)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) -std=c23 $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

example: shaders $(EXAMPLE)
	./$(EXAMPLE)

$(EXAMPLE): examples/main.c $(ENGINE_LIB) $(MATH_LIB)
	@mkdir -p $(dir $@)
	$(CPP_CC) $(CPPFLAGS) $(CFLAGS) $< -o $@ $(LDFLAGS) -lengine $(LIBS)

clean:
	rm -rf $(BUILD) $(SHADERS)

-include $(OBJS:.o=.d)
