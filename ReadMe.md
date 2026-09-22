## ICARUS

An operating system made in one week \
I hope the name is obvious enough

## dependencies

- CMake
- grub-mkrescue
- qemu-system-x86_64
- nasm 
- (xorriso) - might be needed for grub2
- gcc

## Usage

run `./build.sh`
execute `qemu-system-x86_64 -cdrom build/icarus.iso -boot d -m 2048`