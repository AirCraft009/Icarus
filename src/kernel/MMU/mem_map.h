//
// Created by cocon on 25.09.2026.
//

#ifndef ICARUS_MEM_MAP_H
#define ICARUS_MEM_MAP_H
#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "../util/BitMap.h"


typedef struct MemRegion {
    uint64_t start_addr;
    uint64_t end_addr;
    uint64_t free_frames;
    bit_map bitmap;
}mem_region;

/**
 * struct for representing free memory frames.
 * underlying bitmap (0 = free, 1 = set)
 */
typedef struct MemMap {
    uint64_t mem_size;
    uint32_t region_count;
    mem_region regions[];
}mem_map;


void memmap_register_region(mem_map *mmap, uint64_t start, uint64_t end);
void show_mem_map(mem_map *mmap, bool verbose);
void * alloc_frame(mem_map *mmap, uint64_t page_size);

#endif //ICARUS_MEM_MAP_H
