First time working with makefiles and not Cmake

define with := (no " needed)\
````makefile
TARGET := x86_64-icarus
# to access values of vars 
$(TARGET)
````

use 'wildcard' to have extra pattern matching\
````makefile
SOURCES := $(wildcard x64/src/icarus-libc/*.c) $(wildcard x64/src/icarus-libc/*/*.c)
````

use .PHONY to specify a target/ rule to avoid confusion w/ files
```makefile
.PHONY: all install
```

define a rule/ target\
with dependencies & running code
```makefile
target_name: dependency
#   <- tab here
    echo "hello world"
````

makefile selection-flow
```makefile
# define all with dep build-libc/libc.a
# try to start
all: build-libc/libc.a

# build-libc/libc.a depends on OBJECTS 
# so try to build next: build-libc/%.o
build-libc/libc.a: $(OBJECTS)
	$(AR) rcs $@ $(OBJECTS)

# build-libc/%.o depends on the directory x64/src/icarus-libc/%.c
# this exists so this rule starts; then build-libc/libc.a; and finally all returns 
build-libc/%.o: x64/src/icarus-libc/%.c
	mkdir -p build-libc
	$(CC) -ffreestanding -O2 -I "x64/include/" -c $< -o $@

# install isn't touched by make all
install: build-libc/libc.a
	mkdir -p $(PREFIX)/$(TARGET)/include $(PREFIX)/$(TARGET)/lib
	rm -rf $(PREFIX)/$(TARGET)/include/*
	cp -r x64/include/icarus-libc/* $(PREFIX)/$(TARGET)/include/
	cp build-libc/libc.a $(PREFIX)/$(TARGET)/lib/

clean:
	rm -rf build
```

### Basic Usage

just ``make`` will start the first rule (e.g all)\
all rules not affected by this rule have to be called separately
```shell
make && make all
```
