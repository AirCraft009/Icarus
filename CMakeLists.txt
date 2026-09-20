cmake_minimum_required(VERSION 3.20)
project(Icarus C)

set(CMAKE_C_STANDARD 11)

add_executable(Icarus_kernel
        src/boot/boot.s
        src/kernel/kernel.c
)

# GCC options used when compiling the C source.
target_compile_options(Icarus_kernel PRIVATE
        -m32
        -ffreestanding
        -fno-pie
        -fno-stack-protector
)

# Link the kernel using our custom linker script.
target_link_options(Icarus_kernel PRIVATE
        -m32
        -T${CMAKE_SOURCE_DIR}/linker.ld
        -nostdlib
)

# Put the resulting kernel in the ISO directory.
add_custom_command(TARGET Icarus_kernel POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory
        ${CMAKE_BINARY_DIR}/iso/boot

        COMMAND ${CMAKE_COMMAND} -E copy
        $<TARGET_FILE:Icarus_kernel>
        ${CMAKE_BINARY_DIR}/iso/boot/Icarus_kernel.elf
)

# Copy GRUB's configuration into the ISO tree.
add_custom_command(TARGET Icarus_kernel POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory
        ${CMAKE_BINARY_DIR}/iso/boot/grub

        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/grub.cfg
        ${CMAKE_BINARY_DIR}/iso/boot/grub/grub.cfg
)

# Build the actual ISO.
add_custom_target(iso
        COMMAND grub-mkrescue
        -o ${CMAKE_BINARY_DIR}/icarus.iso
        ${CMAKE_BINARY_DIR}/iso

        DEPENDS Icarus_kernel
)