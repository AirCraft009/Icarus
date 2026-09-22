rm -rf ./build
cmake ./ -B ./build/
cmake --build ./build --target iso
qemu-system-x86_64 -cdrom build/icarus.iso -boot d -m 2048