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
static alloc_t * allocator;


void kmain(uint32_t magic, struct multiboot_info *mboot) {
    cursor *cur = init_shellio("ICARUS\n");

    // initialize the Interrupt Descriptor table
    idt_init();

    Imemset(&Kernel_PML4_TABLE[0], 0, sizeof(Kernel_PML4_TABLE));
    //parse the struct given to use from the multiboot2 header
    //init the bitmap for free memory
    if (handle_multiboot2(magic, mboot, &Kernel_PML4_TABLE[0]) != 0) {
        cons_mprintf("init_multiboot2 failed, error while parsing struct\n");
        __asm__ volatile ("cli; hlt"); // Completely hangs the computer
    };
    //call lgdt from high addr again (addr: 0x1088FD)
    gdt_init();
    test_walk_table(&Kernel_PML4_TABLE[0], (phys_addr_t *) KERNEL_PHYS_TO_VIRT(0x0100000));
    write_cr3((uint64_t) KERNEL_VIRT_TO_PHYS(&Kernel_PML4_TABLE[0]));
    cons_mprintf("KERNEL SETUP CONCLUDED: %x\n", &Kernel_PML4_TABLE[0]);

    allocator = init_alloc(DEFAULT_PAGE_SIZE * 5, (void *) KERNEL_HEAP_ADDR);
    cons_mprintf("init allocator\n");
    uint64_t * allocated_a = imalloc_alocator(allocator, 10);
    cons_mprintf("allocated block: %x\n", allocated_a);
    uint64_t * allocated_b = imalloc_alocator(allocator, 10);
    cons_mprintf("allocated block: %x\n", allocated_b);
    uint64_t * allocated_c = imalloc_alocator(allocator, 10);
    cons_mprintf("allocated block: %x\n", allocated_c);
    uint64_t * allocated_d = imalloc_alocator(allocator, 10);
    cons_mprintf("allocated block: %x\n", allocated_d);
    free (allocated_b);
    free (allocated_c);
    uint64_t * allocated_e = imalloc_alocator(allocator, 20);
    cons_mprintf("free filling block: %x\n", allocated_e);


    // cons_mprintf("KERNEL ENDING");
    while (1){}
}
//235372403 = 256809072