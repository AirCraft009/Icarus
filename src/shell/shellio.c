//
// Created by cocon on 20.09.2026.
//

// temporary file while working w/ legacy text mode
// text input output for debugging & similar
// All text will be in white on black bg

#include "shellio.h"

#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>


const uint64_t videoMemStart =  0xB8000;
const uint16_t COLS = 80;
const uint16_t ROWS= 25;



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

/**
 *
 * very basic printf supporting
 *
 * c - char
 * s - string
 * i - int/whole number (up to 20 digits)
 *
 * @param cur
 * @param format
 * @param ...
 */
void mprintf(cursor *cur,  char *format, ...) {
    if (format == NULL) {
        return;
    }

    va_list args;
    va_start(args, format);

    bool format_next = false;
    while (*format) {
        char c = *format++;

        if (c == '%') {
            format_next = true;
            continue;
        }

        if (format_next) {
            format_next = false;
            switch (c) {
                case 'i':
                    char buf[20];
                    int int_arg = va_arg(args, int);
                    int len = 0;
                    while (int_arg > 0) {
                        buf[len++] = '0' + int_arg % 10;
                        int_arg /= 10;
                    }
                    for (int i = len-1; i >= 0; i--) {
                        write_c(buf[i], cur);
                    }
                    break;
                case 's':
                    write_str(va_arg(args, char *), cur);
                    break;
                case 'c':
                    write_c(va_arg(args, int), cur);
                    break;
                default:
                    write_c('%', cur);
                    write_c(c, cur);
                    break;
            }
            continue;
        }
        write_c(c, cur);
    }
    va_end(args);
}

cursor init_shellio(char * starting_text){
    cursor cur = (cursor) {0,0};
    mprintf( &cur, "Init Shell: %s", starting_text);
    // TODO: loop here after keyboard support is enabled
    return cur;
}