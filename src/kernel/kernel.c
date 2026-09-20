// rolling main kernel image wo/ .h files. No linking & they will always have to be 100% included anyway (I think)
#include "IDT.c"

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

