CC      := gcc
CFLAGS  := -std=c11 -O2 -Wall -Wextra -Iinclude -fopenmp
LDFLAGS := -fopenmp -lm

SRCS := $(wildcard src/*.c)
OBJS := $(patsubst src/%.c,build/%.o,$(SRCS))
BIN  := build/convoscale

.PHONY: all clean test

all: $(BIN)

$(BIN): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

build/%.o: src/%.c $(wildcard include/*.h) | build
	$(CC) $(CFLAGS) -c $< -o $@

build:
	mkdir -p build

clean:
	rm -rf build

test: $(BIN)
	$(BIN) self-test
