#include <stddef.h>

#include "kernel_info.h"
#include "multiboot2.h"
#include "interrupts/IDT.h"
#include "../shell/shellio.h"
#include "GDT/gdt.h"
// currently paged in 2MiB page
// 0xFFFFFF7F80000000 – 0xFFFFFF7F801FFFFF


extern uint64_t *gdt_descriptor;
extern uint64_t pml4[512], pdpt[512], pd[512], pt[512];


void kmain(uint32_t magic, struct multiboot_info *mboot) {
    cursor *cur = init_shellio("ICARUS\n");

    // initialize the Interrupt Descriptor table
    idt_init();

    //parse the struct given to use from the multiboot2 header
    handle_multiboot2(magic, mboot);

    //call lgdt from high addr again
    gdt_init();

    mprintf(cur, "KERNEL SETUP COMPLETE");
    while (1){}
}
