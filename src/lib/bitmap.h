//
// Created by Mxsxll on 22.09.2026.
//

#ifndef ICARUS_BITMAP_H
#define ICARUS_BITMAP_H
#pragma once
#include <stdint.h>

typedef unsigned char byte;
typedef struct Bitmap {
    uint64_t size;
    byte	*data;
}bitmap;

int bitmap_set_value(bitmap *map, uint64_t index, char value);
int bitmap_set(bitmap *map, uint64_t index);
int bitmap_clear(bitmap *map, uint64_t index);
byte bitmap_get(bitmap *map, uint64_t index);
int bitmap_set_values(bitmap *map, uint64_t startIndex, uint64_t len);
int bitmap_clear_values(bitmap *map, uint64_t startIndex, uint64_t len);
#endif //ICARUS_BITMAP_H
