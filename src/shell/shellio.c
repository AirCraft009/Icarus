//
// Created by cocon on 20.09.2026.
//

// temporary file while working w/ legacy text mode
// text input output for debugging & similar
// All text will be in white on black bg

#include "shellio.h"
#include "../lib/mem.h"

#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


const uint16_t COLS = 80;
const uint16_t ROWS= 25;
const uint64_t videoMemStart =  0xB8000;
const uint64_t videoMemEnd =  0xB8000 + COLS * ROWS;
static cursor curs;



void move_cursor(uint16_t offset, cursor *cur) {

    uint16_t rows = offset / ROWS;
    uint16_t cols = offset % ROWS;

    cur->x = (cur->x + cols);
    cur->y = (cur->y + rows);

    uint64_t ptr = videoMemStart + (cur->x + cur->y * COLS) * 2;
    if (ptr > videoMemEnd - COLS * 2) {
        uint64_t needed_offset = ptr - videoMemEnd - COLS * 2;
        uint64_t needed_rows = needed_offset / (COLS * 2) + (needed_offset % (COLS * 2))? 1 : 0;
        uint64_t needed_bytes = needed_rows * COLS * 2;
        Imemcpy((uint64_t *)videoMemStart,(uint64_t *) (videoMemStart + needed_bytes), needed_bytes);
        Imemset((uint64_t *)(videoMemEnd - needed_bytes), 0, needed_bytes);
    }
}

void new_lines(uint16_t count, cursor *cur) {
    cur->y += count;
    cur->x = 0;
}

void write_c(char c, cursor *cur) {
    if (c == '\n') {
        new_lines(1, cur);
        return;
    }
    char * vidptr = (char *) videoMemStart + (cur->x + cur->y * COLS) * 2;
    *vidptr = c;
    move_cursor(1, cur);
}

void write_str(char *str, cursor *cur) {
    while (*str) {
        write_c(*str++, cur);
    }
}

void vprintf(cursor *cur, char *format, va_list args) {
    if (format == NULL) {
        return;
    }

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
    va_list args;
    va_start(args, format);
    vprintf(cur, format, args);
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

void cons_mprintf(char *format, ...) {
    va_list args;
    va_start(args, format);
    vprintf(&curs, format, args);
    va_end(args);
}

cursor *init_shellio(char * starting_text){
    curs = (cursor) {0,0};
    mprintf( &curs, "Init Shell: %s", starting_text);
    // TODO: loop here after keyboard support is enabled
    return &curs;
}