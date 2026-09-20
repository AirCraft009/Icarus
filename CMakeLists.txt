# I have never used CMAKE to compile a .s file (TS MIGHT NOT BE GOOD)
cmake_minimum_required(VERSION 3.20)
project(Icarus C ASM_NASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_ASM_NASM_OBJECT_FORMAT elf64)

set_source_files_properties(src/boot/boot.s PROPERTIES LANGUAGE ASM_NASM)
set_source_files_properties(src/kernel/isr.s PROPERTIES LANGUAGE ASM_NASM)

add_executable(Icarus_kernel
        src/boot/boot.s
        src/kernel/kernel.c
        src/kernel/isr.s
        src/kernel/IDT.c
        src/kernel/ISR_handlers.c
        src/kernel/IDT.h
)

target_compile_options(Icarus_kernel PRIVATE
        $<$<COMPILE_LANGUAGE:C>:
        -m64
        -ffreestanding
        -fno-pie
        -fno-pic
        -fno-stack-protector
        -fno-asynchronous-unwind-tables
        -mno-red-zone
        -mgeneral-regs-only
        -Wall -Wextra
        >
)

target_link_options(Icarus_kernel PRIVATE
        -m64
        -nostdlib
        -static
        -no-pie
        -z max-page-size=0x1000
        -T${PROJECT_SOURCE_DIR}/src/linker.ld
)

# Relink when the linker script changes
set_target_properties(Icarus_kernel PROPERTIES
        LINK_DEPENDS ${PROJECT_SOURCE_DIR}/src/linker.ld
)

# Stage the ISO tree
add_custom_command(TARGET Icarus_kernel POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/iso/boot/grub
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:Icarus_kernel> ${CMAKE_BINARY_DIR}/iso/boot/Icarus_kernel.elf
        COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/grub.cfg ${CMAKE_BINARY_DIR}/iso/boot/grub/grub.cfg
)

add_custom_target(iso
        COMMAND grub-mkrescue -o ${CMAKE_BINARY_DIR}/icarus.iso ${CMAKE_BINARY_DIR}/iso
        DEPENDS Icarus_kernel
)