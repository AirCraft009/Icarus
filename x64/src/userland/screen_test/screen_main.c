//
// Created by cocon on 21.11.2026.
//
#include <stdlib.h>

volatile uint8_t * videoMemStart = (uint8_t *)  0xB8000UL;
int main(){
    while (1) {
        videoMemStart[0] = 0xFF;
        videoMemStart[1] = 0x11;
    }
}