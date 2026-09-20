#include "IDT.h"
#include "../shell/shellio.h"

void kmain(void) {


    cursor cur = init_shellio("ICARUS");

    // initialize the Interrupt Descriptor table
    idt_init();
    while (1){}
}

