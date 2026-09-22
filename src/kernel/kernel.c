#include <stddef.h>

#include "multiboot2.h"
#include "interrupts/IDT.h"
#include "../shell/shellio.h"
#include "GDT/gdt.h"
// currently paged in 2MiB page
// 0xFFFFFF7F80000000 – 0xFFFFFF7F801FFFFF


extern uint64_t *gdt_descriptor;


void kmain(uint32_t magic, struct multiboot_info *mboot) {
    cursor cur = init_shellio("ICARUS\n");
    mprintf(&cur, "Multiboot magic: %i\n", magic);

    //parse the struct given to use from the multiboot2 header
    handle_multiboot2(magic, mboot);
    // initialize the Interrupt Descriptor table
    idt_init();

    //call lgdt from high addr again
    gdt_init();

    cur.x = 0;
    cur.y = 0;
    mprintf(&cur, "KERNEL SETUP COMPLETE");
    while (1){}
}
