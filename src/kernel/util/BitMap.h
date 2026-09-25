//
// Created by Mxsxll on 22.09.2026.
//

#ifndef ICARUS_memmap_H
#define ICARUS_memmap_H
#pragma once
#include <stdint.h>

typedef unsigned char byte;
typedef struct BitMap bit_map;

int memmap_set_value(bit_map *map, uint64_t index, char value);
int memmap_set(bit_map *map, uint64_t index);
int memmap_clear(bit_map *map, uint64_t index);
byte memmap_get(bit_map *map, uint64_t index);
int memmap_set_len(bit_map *map, uint64_t startIndex, uint64_t len);
int memmap_clear_len(bit_map *map, uint64_t startIndex, uint64_t len);
void show_mmap(bit_map *map);
int show_mmap_range(bit_map *map, uint64_t start, uint64_t end);
#endif //ICARUS_memmap_H
