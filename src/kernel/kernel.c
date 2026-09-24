#include <stddef.h>

#include "kernel_helper.h"
#include "kernel_info.h"
#include "multiboot2.h"
#include "interrupts/IDT.h"
#include "../shell/shellio.h"
#include "GDT/gdt.h"
#include "MMU/memory_mapping.h"
#include "MMU/page_definitions.h"
// currently paged in 2MiB page
// 0xFFFFFF7F80000000 – 0xFFFFFF7F801FFFFF


extern uint64_t *gdt_descriptor;
static page_map_l4_entry Kernel_PML4_TABLE[512];


void kmain(uint32_t magic, struct multiboot_info *mboot) {
    cursor *cur = init_shellio("ICARUS\n");

    // initialize the Interrupt Descriptor table
    idt_init();

    //parse the struct given to use from the multiboot2 header
    //init the bitmap for free memory
    handle_multiboot2(magic, mboot);

    //call lgdt from high addr again
    gdt_init();


    // drop old pml4 table and switch to new one.
    // currently just map 17 huge pages.
    for (uint64_t i = 0; i < 17; i++) {
        page_in(&Kernel_PML4_TABLE[0], (void *) (KERNEL_VMA + i * HUGE_PS), (void *) (i * HUGE_PS), HUGE_PS);
    }
    mprintf(cur, "KERNEL SETUP COMPLETE1");
    write_cr3((uint64_t) Kernel_PML4_TABLE);

    mprintf(cur, "KERNEL SETUP COMPLETE2");
    while (1){}
}
