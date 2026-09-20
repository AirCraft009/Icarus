#include "interrupts/IDT.h"
#include "../shell/shellio.h"

void kmain(void) {


    // initialize the Interrupt Descriptor table
    idt_init();

    cursor cur = init_shellio("ICARUS");
    while (1){}
}

