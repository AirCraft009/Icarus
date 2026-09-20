//
// Created by cocon on 20.09.2026.
//

// temporary file while working w/ legacy text mode
// text input output for debugging & similar
// All text will be in white on black bg

#include "shellio.h"
#include <stdint.h>


const uint64_t videoMemStart =  0xB8000;
const uint16_t COLS = 80;
const uint16_t ROWS= 25;


cursor init_shellio(char * starting_text);
void move_cursor(uint16_t offset, cursor *cur);
void new_lines(uint16_t count, cursor *cur);
void write_c(char c, cursor *cur);
void write_str(char *str, cursor *cur);

void move_cursor(uint16_t offset, cursor *cur) {
    // two bytes bc first is ASCII second is color & bg
    offset = offset * 2;


    uint16_t rows = offset / ROWS;
    uint16_t cols = offset % ROWS;

    cur->x += cols;
    cur->y += rows;
}

void new_lines(uint16_t count, cursor *cur) {
    move_cursor(count * ROWS, cur);
}

void write_c(char c, cursor *cur) {
    if (c == '\n') {
        new_lines(1, cur);
    }
    char * vidptr = (char *) videoMemStart + cur->x + cur->y * ROWS;
    *vidptr = c;
    move_cursor(1, cur);
}

void write_str(char *str, cursor *cur) {
    while (*str) {
        write_c(*str++, cur);
    }
}

void write_f(char *str,  cursor *cur) {

}

cursor init_shellio(char * starting_text){
    cursor cur;
    write_str(starting_text, &cur);
    // TODO: loop here after keyboard support is enabled
    return cur;
}