//
// Created by Mxsxll on 22.09.2026.
//

#ifndef ICARUS_bitmap_H
#define ICARUS_bitmap_H
#pragma once
#include <stdint.h>

typedef unsigned char byte;
typedef struct BitMap{
    uint64_t size;
    // use an array not a ptr (bc the array is part of the struct) I learned my lesson
    byte	data[];
}bit_map;


int bitmap_set_value(bit_map *map, uint64_t index, char value);
int bitmap_set(bit_map *map, uint64_t index);
int bitmap_clear(bit_map *map, uint64_t index);
byte bitmap_get(bit_map *map, uint64_t index);
int bitmap_set_len(bit_map *map, uint64_t startIndex, uint64_t len);
int bitmap_clear_len(bit_map *map, uint64_t startIndex, uint64_t len);
void show_bit_map(bit_map *map);
int show_bit_map_range(bit_map *map, uint64_t start, uint64_t end);
#endif //ICARUS_bitmap_H
