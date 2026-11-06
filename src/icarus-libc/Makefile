TARGET  := x86_64-icarus
PREFIX  := $(HOME)/opt/icarus-cross
CC      := $(PREFIX)/bin/$(TARGET)-gcc
AR      := $(PREFIX)/bin/$(TARGET)-ar

SOURCES := $(wildcard src/*.c)
OBJECTS := $(SOURCES:src/%.c=build/%.o)

all: build/libc.a

build/libc.a: $(OBJECTS)
	$(AR) rcs $@ $(OBJECTS)

build/%.o: src/%.c
	mkdir -p build
	$(CC) -ffreestanding -O2 -Iinclude -c $< -o $@

install: build/libc.a
	mkdir -p $(PREFIX)/$(TARGET)/include $(PREFIX)/$(TARGET)/lib
	cp -r include/* $(PREFIX)/$(TARGET)/include/
	cp build/libc.a $(PREFIX)/$(TARGET)/lib/

clean:
	rm -rf build