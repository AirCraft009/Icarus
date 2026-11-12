./build.sh clean &&\
./build.sh full_iso &&\
qemu-system-x86_64 -cdrom build/icarus.iso -boot d -m 2048 -machine q35 -no-reboot -no-shutdown
