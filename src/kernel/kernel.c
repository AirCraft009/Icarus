#include <stddef.h>

#include "../lib/mem_utils.h"
#include "util/kernel_helper.h"
#include "util/kernel_info.h"
#include "util/multiboot2.h"
#include "interrupts/IDT.h"
#include "../shell/shellio.h"
#include "GDT/gdt.h"
#include "Memory/memory_mapping.h"
#include "Memory/page_definitions.h"
#include "Memory/alloc.h"
// currently paged in 2MiB page
// 0xFFFFFF7F80000000 – 0xFFFFFF7F801FFFFF


extern uint64_t *gdt_descriptor;


static page_map_l4_entry Kernel_PML4_TABLE[512] __attribute__((aligned(4096)));
//static alloc_t * allocator;


void kmain(uint32_t magic, struct multiboot_info *mboot) {
    cursor *cur = init_shellio("ICARUS\n");

    // initialize the Interrupt Descriptor table
    idt_init();

    Imemset(&Kernel_PML4_TABLE[0], 0, sizeof(Kernel_PML4_TABLE));
    //parse the struct given to use from the multiboot2 header
    //init the bitmap for free memory
    handle_multiboot2(magic, mboot, &Kernel_PML4_TABLE[0]);
    //call lgdt from high addr again (addr: 0x1088FD)
    gdt_init();
    cons_mprintf("pml4 %x\n", &Kernel_PML4_TABLE[0]);
    uint8_t * instruction_data = (uint8_t*) 0xffffffff80106184;
    cons_mprintf("fulldata=");
    for (int i = 0; i < 10; i++) {
        cons_mprintf("%x ", instruction_data[i]);
    }
    write_cr3((uint64_t) KERNEL_VIRT_TO_PHYS(&Kernel_PML4_TABLE[0]));
    cons_mprintf("KERNEL SETUP CONCLUDED");
    //
    // allocator = init_alloc(HUGE_PS, KERNEL_HEAP_ADDR);
    // uint64_t * allocated_b = imalloc(allocator, 10);
    // cons_mprintf("allocated bloc: %x\n", allocated_b);
    //
    // cons_mprintf("KERNEL ENDING");
    while (1){}
}
//235372403 = 256809072