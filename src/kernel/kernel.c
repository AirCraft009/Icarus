#include <stddef.h>
#include <stdint.h>

#include "kernel_helper.h"
#include "page_definitions.h"
#include "kernel/GDT/gdt.h"
#include "kernel/interrupts/IDT.h"
#include "kernel/util/mem_utils.h"
#include "kernel/util/shellio.h"
#include "Memory/memory_mapping.h"
#include "util/multiboot2.h"
#include "kernel/Memory/alloc.h"
#include "kernel/Memory/PMM/mem_map.h"

extern uint64_t *gdt_descriptor;


static page_map_l4_entry Kernel_PML4_TABLE[512] __attribute__((aligned(4096)));
static alloc_t * allocator;


void kmain(uint32_t magic, struct multiboot_info *mboot) {
    cursor *cur = init_shellio("ICARUS\n");

    // initialize the Interrupt Descriptor table
    idt_init();

    Imemset(&Kernel_PML4_TABLE[0], 0, sizeof(Kernel_PML4_TABLE));

    // call lgdt from high addr again (addr: 0x1088FD)
    gdt_init();
    //parse the struct given to use from the multiboot2 header
    //init the bitmap for free memory
    if (handle_multiboot2(magic, mboot, &Kernel_PML4_TABLE[0]) != 0) {
        cons_mprintf("init_multiboot2 failed, error while parsing struct\n");
        __asm__ volatile ("cli; hlt"); // Completely hangs the computer
    };
    test_walk_table(&Kernel_PML4_TABLE[0], (phys_addr_t *) KERNEL_PHYS_TO_VIRT(0x0100000));
    cons_mprintf("RAAAAHH\n");
    write_cr3((uint64_t) KERNEL_VIRT_TO_PHYS(&Kernel_PML4_TABLE[0]));


    cons_mprintf("KERNEL SETUP CONCLUDED: %x\n", &Kernel_PML4_TABLE[0]);

    allocator = init_alloc(DEFAULT_PAGE_SIZE * 510, (void *) KERNEL_HEAP_ADDR);

    // cons_mprintf("KERNEL ENDING");
    while (1){}
}
//235372403 = 256809072