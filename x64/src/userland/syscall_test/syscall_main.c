#include <stdlib.h>

#define VIDEO_MEM ((volatile uint8_t *)0xB8000UL)
int main(){
    while (1)
        // empty_call();
        write(0, "HELLO WORLD", 12, 0);
    return 0;
}