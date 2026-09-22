#include <stddef.h>

#include "multiboot2.h"
#include "interrupts/IDT.h"
#include "../shell/shellio.h"

void kmain(uint32_t magic, struct multiboot_info *mboot) {
    cursor cur = init_shellio("ICARUS\n");
    mprintf(&cur, "Multiboot magic: %i\n", magic);

    if (magic != MULTIBOOT2_HEADER_MAGIC) {
        mprintf(&cur, "ILLEGAL MAGIC\nKERNEL WASN'T BOOTED W/ Multiboot2");
    }
    mprintf(&cur, "Multiboot size: %i\n", mboot->total_size);

    // initialize the Interrupt Descriptor table
    idt_init();
    //parse the struct given to use from the multiboot2 header
    handle_multiboot2(mboot);
    while (1){}
}
