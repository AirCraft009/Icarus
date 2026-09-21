# I have never used CMAKE to compile a .s file (TS MIGHT NOT BE GOOD)
cmake_minimum_required(VERSION 3.20)
project(Icarus C ASM_NASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_ASM_NASM_OBJECT_FORMAT elf64)

set_source_files_properties(src/boot/boot.s PROPERTIES LANGUAGE ASM_NASM)
set_source_files_properties(src/kernel/interrupts/isr.s PROPERTIES LANGUAGE ASM_NASM)

add_executable(Icarus_kernel
        src/boot/boot.s
        src/kernel/kernel.c
        src/kernel/interrupts/isr.s
        src/kernel/interrupts/IDT.c
        src/kernel/interrupts/ISR_handlers.c
        src/kernel/interrupts/IDT.h
        src/shell/shellio.c
        src/shell/shellio.h
        src/kernel/MMU/memory_mapping.c
        src/kernel/MMU/memory_mapping.h
        src/lib/string.c
        src/lib/string.h
        src/kernel/interrupts/TSS.c
        src/kernel/interrupts/TSS.h
        src/kernel/MMU/page_definitions.c
        src/kernel/MMU/page_definitions.h
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
        COMMAND ${CMAKE_COMMAND} -E echo "Binary dir:        ${CMAKE_BINARY_DIR}"
        COMMAND grub-mkrescue -o ${CMAKE_BINARY_DIR}/icarus.iso ${CMAKE_BINARY_DIR}/iso
        DEPENDS Icarus_kernel
)
