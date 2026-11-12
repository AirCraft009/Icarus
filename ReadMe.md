# ICARUS

A x64 operating system (Am I like that?)\
I hope the name is obv enough

## Build

### Kernel

Build on linux / wsl for windows\
run ./build_sh iso

## Cross-Compiler

The icarus cross-compiler is based on gcc.\
A binutils and a GCC repo are available to build it.

- clone icarus-cc
- clone icarus-binutils
- setup a folder to house the icarus cross-compiler
  - mkdir -r ~/opt/icarus-cross
- //TODO: add explanation of how to build gcc and binutils
- add this as a script

``icarus-shell.env``
```bash
#!/bin/bash
export PREFIX="$HOME/opt/icarus-cross"
export TARGET=x86_64-icarus
export PATH="$PREFIX/bin:$PATH"
```
then
- source (path to)icarus-shell.env

### Building libc

- run ``make && make all``


## dependencies

- CMake
- grub-mkrescue
- qemu-system-x86_64
- nasm 
- (xorriso) - might be needed for grub2
- gcc
- sudo apt install --reinstall grub-pc-bin

## Credits

### Framebuffer fonts
- https://nothings.org/stb/font/

### GOATED SERIES
- https://www.youtube.com/watch?v=ADHfYnzjrkw&list=PLBTQvUDSl81dTG_5Uk2mycxZihfeAYTRm

## Usage (linux)
run `./build.sh`
