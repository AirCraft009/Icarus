#include <stdlib.h>

#define VIDEO_MEM ((volatile uint8_t *)0xB8000UL)
int main(){
    VIDEO_MEM[0] = 'C';
    VIDEO_MEM[1] = 0xF0;
    return 0;
}