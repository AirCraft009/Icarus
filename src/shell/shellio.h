//
// Created by cocon on 20.09.2026.
//

#ifndef ICARUS_SHELLIO_H
#define ICARUS_SHELLIO_H
#pragma once
#include <stdint.h>

typedef struct Cursor {
    uint16_t x;
    uint16_t y;
} cursor;

cursor init_shellio(char * starting_text);
void move_cursor(uint16_t offset, cursor *cur);
void new_lines(uint16_t count, cursor *cur);
void write_c(char c, cursor *cur);
void write_str(char *str, cursor *cur);
void mprintf(cursor *cur,  char *format, ...);

#endif //ICARUS_SHELLIO_H
