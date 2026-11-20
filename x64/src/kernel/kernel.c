#include <stddef.h>
#include <stdint.h>

#include "kernel_helper.h"
#include "page_definitions.h"
#include "kernel/GDT/gdt.h"
#include "kernel/interrupts/APIC.h"
#include "kernel/interrupts/IDT.h"
#include <string.h>
#include "kernel/util/shellio.h"
#include "../../include/kernel/Memory/memory_mapping.h"
#include "kernel/Drivers/PS2/PS2_Controller.h"
#include "kernel/Drivers/PS2/PS2_Keyboard.h"
#include "kernel/util/multiboot2.h"
#include "kernel/util/cpuid_helpers.h"
#include "kernel/Memory/alloc.h"
#include "kernel/Memory/paging.h"
#include "kernel/Memory/DMA/MADT.h"
#include "kernel/Memory/PMM/phys_alloc.h"
#include "kernel/Processes/Process.h"
#include "kernel/Processes/thread.h"
#include "kernel/Processes/Scheduling/scheduler.h"
#include "kernel/util/Files/elf.h"
#include "kernel/util/Files/tar.h"
#include "syscall/syscall.h"

extern uint64_t *gdt_descriptor;


static page_map_l4_entry Kernel_PML4_TABLE[512] __attribute__((aligned(4096))) = {0};
static alloc_t * allocator;
static cpuid_info CPUID_TABLE = {0};

// 20 bytes, position independent, no stack use
// static const uint8_t fill_vga[] = {
//     0xB8, 0xFF, 0xFF, 0xFF, 0xFF,  // mov eax, 0xFFFFFFFF (outside of loop to check if regs are preserved)
//     0xFC,                          // cld
//     0xBF, 0x00, 0x80, 0x0B, 0x00,  // mov edi, 0xB8000
//     0xB9, 0xE8, 0x03, 0x00, 0x00,  // mov ecx, 1000        (1000 dwords = 4000 bytes = 80x25x2)
//     0xF3, 0xAB,                    // rep stosd
//     0xEB, 0xF1                     // jmp -15 (back to cld, clears forever)
// };
//
// static const uint8_t color_vga[] = {
//     0xB8, 0x11, 0x11, 0x11, 0x11,  // mov eax, 0x11111111
//     0xFC,                          // cld
//     0xBF, 0x00, 0x80, 0x0B, 0x00,  // mov edi, 0xB8000
//     0xB9, 0xE8, 0x03, 0x00, 0x00,  // mov ecx, 1000        (1000 dwords = 4000 bytes = 80x25x2)
//     0xF3, 0xAB,                    // rep stosd
//     0xEB, 0xF1                     // jmp -15 (back to cld, clears forever)
// };
// // 20 bytes, position independent, no stack use
//
// static const uint8_t clear_vga[] = {
//     0xB8, 0x20, 0x07, 0x20, 0x07,  // mov eax, 0x07200720  (two cells: ' ' + attribute 0x07)
//     0xFC,                          // cld
//     0xBF, 0x00, 0x80, 0x0B, 0x00,  // mov edi, 0xB8000
//     0xB9, 0xE8, 0x03, 0x00, 0x00,  // mov ecx, 1000        (1000 dwords = 4000 bytes = 80x25x2)
//     0xF3, 0xAB,                    // rep stosd
//     0xEB, 0xF1                     // jmp -15 (back to cld, clears forever)
// };

static const uint8_t clear_vga[] = {
    0xB8, 0x20, 0x07, 0x00, 0x00,  // mov eax, 0x0720      (one cell: ' ' + attribute 0x07)
    0xFC,                          // cld
    0xBF, 0x00, 0x80, 0x0B, 0x00,  // mov edi, 0xB8000     (top-left cell)
    0x66, 0xAB,                    // stosw                (store AX, one 16-bit cell, no rep)
    0xEB, 0xF6                     // jmp -10 (back to cld, repeats forever)
};

static const uint8_t fill_vga[] = {
    0xB8,  0xFF, 0xFF, 0xFF, 0xFF,  // mov eax, 0x0720      (one cell: ' ' + attribute 0x07)
    0xFC,                          // cld
    0xBF, 0x00, 0x80, 0x0B, 0x00,  // mov edi, 0xB8000     (top-left cell)
    0x66, 0xAB,                    // stosw                (store AX, one 16-bit cell, no rep)
    0xEB, 0xF6                     // jmp -10 (back to cld, repeats forever)
};

static const uint8_t color_vga[] = {
    0xB8, 0x11, 0x11, 0x11, 0x11,  // mov eax, 0x0720      (one cell: ' ' + attribute 0x07)
    0xFC,                          // cld
    0xBF, 0x00, 0x80, 0x0B, 0x00,  // mov edi, 0xB8000     (top-left cell)
    0x66, 0xAB,                    // stosw                (store AX, one 16-bit cell, no rep)
    0xEB, 0xF6                     // jmp -10 (back to cld, repeats forever)
};

static const uint8_t loop[] = {
    0xEB, 0xFE
};
//235372403 = 256809072
void kmain(uint32_t magic, struct multiboot_info *mboot) {
    //TODO: IMPLEMENT A PER CORE STACK INSTEAD OF GLOBAL
    cursor *cur = shellio_init("ICARUS\n");

    // initialize the Interrupt Descriptor table
    idt_init();

    memset(&Kernel_PML4_TABLE[0], 0, sizeof(Kernel_PML4_TABLE));

    //parse the struct given to use from the multiboot2 header
    //init the bitmap for free memory
    paging_init();
    if (handle_multiboot2(magic, mboot, &Kernel_PML4_TABLE[0]) != 0) {
        Iprintf("init_multiboot2 failed, error while parsing mulitboot2 struct\n");
        __asm__ volatile ("cli; hlt"); // Completely hangs the computer
    };

    // call lgdt from high addr again (addr: 0x1088FD)
    gdt_init();


    write_cr3((uint64_t) KERNEL_VIRT_TO_PHYS(&Kernel_PML4_TABLE[0]));

    allocator = alloc_init(DEFAULT_PAGE_SIZE * 510, (void *) KERNEL_HEAP_ADDR);
    handle_ACPI();
    parse_tar_module();

    init_cpuid(&CPUID_TABLE);
    if (!CHECK_CPUID_FEAT(CPUID_TABLE.features, CPUID_FEAT_EDX_MSR)) {
        Iprintf("MSR not supported: ICARUS cannot run\n");
        __asm__ volatile ("cli; hlt"); // Completely hangs the computer
    }


    // if (syscall_init() != 0) {
    //     Iprintf("Syscalls can't be executed! !!!halting!!!\n");
    //     __asm__ volatile ("cli; hlt"); // Completely hangs the computer
    // }
    enable_apic();
    enable_io_apics();

    if (ps2_controller_init() != 0)
        Iprintf("The PS2 Controller doesn't work\n");

    Iprintf("Ticks per Ms: (%i)\n", LAPIC_TICKS_PER_MS);
    page_in(Kernel_PML4_TABLE, (void *)0xB8000, (void *)0xB8000, DEFAULT_PAGE_SIZE, PT_WRITEABLE | 1 <<PT_USER_ACCESS_SHIFT);

    pcb_t * init = create_initial_process
    (
        0,
        true,
        1100,
        DEFAULT_PAGE_SIZE,
        0x40000
    );
    struct_page_t * ps = process_map(init, 0x40000, 1, DEFAULT_PAGE, PT_WRITEABLE);
    memcpy((void *) KERNEL_PHYS_TO_VIRT(ps->page_addr[0]), color_vga, sizeof(color_vga));

    char * data = (void *) files[0] + 512;
    if (load_elf((elf_header_t *) data, 0, true, 100) != 0)
        __asm__ volatile ("cli; hlt");

    Iprintf("_____________--------------------------------\n");
    __asm__ volatile ("sti");
    keyboard_ps2_init();
    scheduler_start(1000);


    //TODO: build a table to hold all important subtables and system_statistics
    while (1)
        ;
}
