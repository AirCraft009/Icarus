TARGET  := x86_64-icarus
PREFIX  := $(HOME)/opt/icarus-cross
CC      := $(PREFIX)/bin/$(TARGET)-gcc
AR      := $(PREFIX)/bin/$(TARGET)-ar

SOURCES := $(wildcard x64/src/icarus-libc/*.c) $(wildcard x64/src/icarus-libc/*/*.c)
OBJECTS := $(SOURCES:src/%.c=build/%.o)

all: build-libc/libc.a

build-libc/libc.a: $(OBJECTS)
	@mkdir -p $(dir $@)
	rm -f $@
	$(AR) rcs $@ $^

build/%.o: x64/src/icarus-libc
	mkdir -p build-libc
	$(CC) -ffreestanding -O2 -Iinclude -c $< -o $@

install: build-libc/libc.a
	mkdir -p $(PREFIX)/$(TARGET)/include $(PREFIX)/$(TARGET)/lib
	rm -rf $(PREFIX)/$(TARGET)/include/*
	cp -r x64/include/icarus-libc/* $(PREFIX)/$(TARGET)/include/
	cp build-libc/libc.a $(PREFIX)/$(TARGET)/lib/

clean:
	rm -rf build