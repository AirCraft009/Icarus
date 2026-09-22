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
static cursor curs;



void move_cursor(uint16_t offset, cursor *cur) {
    // two bytes bc first is ASCII second is color & bg
    offset = offset * 2;


    uint16_t rows = offset / ROWS;
    uint16_t cols = offset % ROWS;

    cur->x = (cur->x + cols);
    cur->y = (cur->y + rows);
}

void new_lines(uint16_t count, cursor *cur) {
    cur->y += count * 2;
    cur->x = 0;
}

void write_c(char c, cursor *cur) {
    if (c == '\n') {
        new_lines(1, cur);
        return;
    }
    char * vidptr = (char *) videoMemStart + cur->x + cur->y * COLS;
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
void mprintf(cursor *cur, char *format, ...) {
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
                case 'i': {
                    int int_arg = va_arg(args, int);
                    char buf[20];
                    int len = 0;
                    bool neg = int_arg < 0;
                    unsigned int uval = neg ? -(unsigned int)int_arg : (unsigned int)int_arg;
                    do {
                        buf[len++] = '0' + uval % 10;
                        uval /= 10;
                    } while (uval > 0);
                    if (neg) write_c('-', cur);
                    for (int i = len - 1; i >= 0; i--) {
                        write_c(buf[i], cur);
                    }
                    break;
                }
                case 'h': {
                    int int16_arg = va_arg(args, int); /* promoted */
                    char hbuf[20];
                    int hlen = 0;
                    bool neg = int16_arg < 0;
                    unsigned int uval = neg ? -(unsigned int)int16_arg : (unsigned int)int16_arg;
                    do {
                        hbuf[hlen++] = '0' + uval % 10;
                        uval /= 10;
                    } while (uval > 0);
                    if (neg) write_c('-', cur);
                    for (int i = hlen - 1; i >= 0; i--) {
                        write_c(hbuf[i], cur);
                    }
                    break;
                }
                case 'l': {
                    uint64_t long_arg = va_arg(args, uint64_t);
                    char lbuf[20];
                    int llen = 0;
                    do {
                        lbuf[llen++] = '0' + long_arg % 10;
                        long_arg /= 10;
                    } while (long_arg > 0);
                    for (int i = llen - 1; i >= 0; i--) {
                        write_c(lbuf[i], cur);
                    }
                    break;
                }
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

void cons_move_cursor(uint16_t offset) {
    move_cursor(offset, &curs);
}

void cons_new_lines(uint16_t count) {
    new_lines(count, &curs);
}
void cons_write_c(char c) {
    write_c(c, &curs);
}
void cons_write_str(char *str) {
    write_str(str, &curs);
}

cursor *init_shellio(char * starting_text){
    curs = (cursor) {0,0};
    mprintf( &curs, "Init Shell: %s", starting_text);
    // TODO: loop here after keyboard support is enabled
    return &curs;
}