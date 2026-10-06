# I have never used CMAKE to compile a .s file (TS MIGHT NOT BE GOOD)
cmake_minimum_required(VERSION 3.20)
project(Icarus C ASM_NASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_ASM_NASM_OBJECT_FORMAT elf64)

set_source_files_properties(src/boot/boot.asm PROPERTIES LANGUAGE ASM_NASM)
set_source_files_properties(src/kernel/interrupts/isr.asm PROPERTIES LANGUAGE ASM_NASM)

add_executable(Icarus_kernel
        src/boot/boot.asm
        src/kernel/kernel.c
        src/kernel/interrupts/isr.asm
        src/kernel/interrupts/IDT.c
        src/kernel/interrupts/ISR_handlers.c
        include/kernel/interrupts/IDT.h
        src/kernel/util/shellio.c
        include/kernel/util/shellio.h
        src/kernel/Memory/memory_mapping.c
        include/kernel/Memory/memory_mapping.h
        src/kernel/GDT/TSS.c
        src/kernel/GDT/TSS.h
        include/page_definitions.h
        src/kernel/util/multiboot2.c
        include/kernel/util/multiboot2.h
        src/kernel/util/BitMap.c
        include/kernel/util/BitMap.h
        src/kernel/GDT/gdt.c
        include/kernel/GDT/gdt.h
        src/kernel/util/mem_utils.c
        include/kernel/util/mem_utils.h
        include/kernel_helper.h
        src/kernel/Memory/PMM/mem_map.c
        include/kernel/Memory/PMM/mem_map.h
        src/kernel/Memory/alloc.c
        include/kernel/Memory/alloc.h
        include/kernel/util/font.h
        src/kernel/Drivers/USB3.c
        include/kernel/Drivers/USB3.h
        src/kernel/Memory/DMA/ACPI/ACPI.c
        include/kernel/Memory/DMA/ACPI.h
        src/kernel/Memory/DMA/MMIO.c
        include/kernel/Memory/DMA/MMIO.h
        include/kernel/Memory/DMA/ACPI_helpers.h
        src/kernel/interrupts/APIC/APIC.c
        include/kernel/interrupts/APIC.h
        include/kernel/util/cpuid_helpers.h
        src/kernel/interrupts/APIC/PIC.c
        include/kernel/interrupts/PIC.h
        src/kernel/Memory/DMA/ACPI/PCIE.c
        include/kernel/Memory/DMA/PCIE.h
        src/kernel/Memory/DMA/ACPI/MADT.c
        include/kernel/Memory/DMA/MADT.h
)

target_include_directories(Icarus_kernel PRIVATE include
        ${PROJECT_SOURCE_DIR}/include
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
        -mcmodel=kernel
        >
)

target_link_options(Icarus_kernel PRIVATE
        -m64
        -nostdlib
        -static
        -no-pie
        -fno-pie
        -fno-pic
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
