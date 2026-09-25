//
// Created by Mxsxll on 22.09.2026.
//

#ifndef ICARUS_memmap_H
#define ICARUS_memmap_H
#pragma once
#include <stdint.h>

typedef unsigned char byte;
typedef struct MemMap {
    uint64_t size;
    uint64_t mem_size;
    uint64_t offset;
    uint64_t last_accessed;
    // use an array not a ptr (bc the array is part of the struct) I learned my lesson
    byte	data[];
}mem_map;

int memmap_set_value(mem_map *map, uint64_t index, char value);
int memmap_set(mem_map *map, uint64_t index);
int memmap_clear(mem_map *map, uint64_t index);
byte memmap_get(mem_map *map, uint64_t index);
int memmap_set_len(mem_map *map, uint64_t startIndex, uint64_t len);
int memmap_clear_len(mem_map *map, uint64_t startIndex, uint64_t len);
void show_mmap(mem_map *map);
void show_mmap_range(mem_map *map, uint64_t start, uint64_t end);
#endif //ICARUS_memmap_H
