# I have never used CMAKE to compile a .s file (TS MIGHT NOT BE GOOD)
cmake_minimum_required(VERSION 3.20)
project(Icarus C ASM_NASM)

set(CMAKE_C_STANDARD 23)
set(CMAKE_ASM_NASM_OBJECT_FORMAT elf64)

set_source_files_properties(x64/src/kernel/boot/boot.asm PROPERTIES LANGUAGE ASM_NASM)
set_source_files_properties(x64/src/kernel/interrupts/isr.asm PROPERTIES LANGUAGE ASM_NASM)

add_executable(Icarus_kernel
        x64/src/kernel/boot/boot.asm
        x64/src/kernel/kernel.c
        x64/src/kernel/interrupts/isr.asm
        x64/src/kernel/interrupts/IDT.c
        x64/src/kernel/interrupts/ISR_handlers.c
        x64/include/kernel/interrupts/IDT.h
        x64/src/kernel/util/shellio.c
        x64/include/kernel/util/shellio.h
        x64/src/kernel/Memory/memory_mapping.c
        x64/include/kernel/Memory/memory_mapping.h
        x64/src/kernel/GDT/TSS.c
        x64/src/kernel/GDT/TSS.h
        x64/include/page_definitions.h
        x64/src/kernel/util/multiboot2.c
        x64/include/kernel/util/multiboot2.h
        x64/src/kernel/util/BitMap.c
        x64/include/kernel/util/BitMap.h
        x64/src/kernel/GDT/gdt.c
        x64/include/kernel/GDT/gdt.h
        x64/include/kernel_helper.h
        x64/src/kernel/Memory/PMM/mem_map.c
        x64/include/kernel/Memory/PMM/mem_map.h
        x64/src/kernel/Memory/alloc.c
        x64/include/kernel/Memory/alloc.h
        x64/include/kernel/util/font.h
        x64/src/kernel/Drivers/USB/USB3.c
        x64/include/kernel/Drivers/USB/USB3.h
        x64/src/kernel/Memory/DMA/ACPI/ACPI.c
        x64/include/kernel/Memory/DMA/ACPI.h
        x64/src/kernel/Memory/DMA/MMIO.c
        x64/include/kernel/Memory/DMA/MMIO.h
        x64/include/kernel/Memory/DMA/ACPI_helpers.h
        x64/src/kernel/interrupts/APIC/APIC.c
        x64/include/kernel/interrupts/APIC.h
        x64/include/kernel/util/cpuid_helpers.h
        x64/src/kernel/interrupts/APIC/PIC.c
        x64/include/kernel/interrupts/PIC.h
        x64/src/kernel/Memory/DMA/ACPI/PCIE.c
        x64/include/kernel/Memory/DMA/PCIE.h
        x64/src/kernel/Memory/DMA/ACPI/MADT.c
        x64/include/kernel/Memory/DMA/MADT.h
        x64/src/kernel/Drivers/PS2/PS2_Keyboard.c
        x64/include/kernel/Drivers/PS2/PS2_Keyboard.h
        x64/src/kernel/Drivers/PS2/PS2_Controller.c
        x64/include/kernel/Drivers/PS2/PS2_Controller.h
        x64/src/kernel/Drivers/PS2/PS2_Scancode.h
        x64/include/kernel/Processes/thread.h
        x64/include/kernel/util/cpu_regs.h
        x64/src/kernel/Processes/Scheduling/scheduler.c
        x64/include/kernel/Processes/Scheduling/scheduler.h
        x64/src/kernel/Processes/thread.c
        x64/src/kernel/Memory/paging.c
        x64/include/kernel/Memory/paging.h
        x64/include/kernel/interrupts/ISR.h
        x64/src/kernel/Processes/Scheduling/set_context.asm
        x64/src/kernel/util/MasterRecordTable.c
        x64/src/kernel/util/MasterRecordTable.h
        x64/src/kernel/Processes/Process.c
        x64/include/kernel/Processes/Process.h
        x64/src/kernel/syscall/syscall.c
        x64/src/kernel/syscall/syscall.h
        x64/src/kernel/syscall/syscall_entry.asm
        x64/include/kernel/syscall/core_information_table.h
        x64/src/kernel/util/Files/tar_parse.c
        x64/include/kernel/util/Files/tar_parse.h
        x64/src/kernel/util/Files/elf_parse.c
        x64/include/kernel/util/Files/elf_parse.h
        x64/include/assert.h
        process_management.c
        process_management.h
        x64/src/kernel/util/Hashmap.c
        x64/src/kernel/util/Hashmap.h
        x64/src/kernel/Datapoints/Datapoint.c
        x64/include/kernel/Datapoints/Datapoint.h
        x64/include/kernel/Datapoints/Datapoint.h
)

target_include_directories(Icarus_kernel PRIVATE x64/include
        x64/include
        x64/include/icarus-libc
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
        LINK_DEPENDS x64/src/kernel/linker.ld
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
