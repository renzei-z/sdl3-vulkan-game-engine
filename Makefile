CC=gcc
CFLAGS=-g
INCLUDE=-Iinclude
LIBS=-lSDL3 -lvulkan

BUILD=build

SRCS=$(wildcard src/*.c)
OBJS=$(SRCS:src/%.c=$(BUILD)/%.o)

.PHONY: all run clean

all: game

run: all
	./game

game: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

$(BUILD)/%.o: src/%.c
	@mkdir -p $(BUILD)
	$(CC) $(INCLUDE) $(CFLAGS) -c -o $@ $^

clean:
	rm -rf $(BUILD)
	rm -f game
