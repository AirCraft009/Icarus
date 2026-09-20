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

#endif //ICARUS_SHELLIO_H
