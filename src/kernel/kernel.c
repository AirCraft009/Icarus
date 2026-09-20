#include "IDT.h"

void kmain(void) {
    const char *str = "!!ICARUS!!";
    char *videoptr = (char*) 0xB8000;


    while (*str != '\0') {
        *videoptr = *str;
        videoptr+=2;
        str++;
    }

    // initialize the Interrupt Descriptor table
    idt_init();
    while (1){}
}

