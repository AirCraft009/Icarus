#include <string.h>
#include <stdlib.h>

uint8_t * videoMemStart = (uint8_t *)  0xB8000UL;
int main(){
    while (1) {
        videoMemStart[0] = 0xFF;
        videoMemStart[1] = 0xFF;
    }
}