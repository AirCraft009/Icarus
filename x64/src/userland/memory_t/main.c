//
// Created by cocon on 22.11.2026.
//
#include <stdlib.h>

#define VIDEO_MEM ((volatile uint8_t *)0xB8000UL)
int main(){
    while (1) {
        VIDEO_MEM[0] = 'A';
        VIDEO_MEM[1] = 0xF0;
        // // for (int i = 1; i < 80 * 25 * 2; i+=2) {
        // //     VIDEO_MEM[i-1] = 'B';
        // //     VIDEO_MEM[i] = 0xF0;
        // // }
    }
}