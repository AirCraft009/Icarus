//
// Created by cocon on 25.09.2026.
//

#ifndef ICARUS_MEM_MAP_H
#define ICARUS_MEM_MAP_H
#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "kernel/util/BitMap.h"

#define FRAME_ALLOC_FAILED UINT64_MAX
#define MAX_MEM_REGIONS 32


typedef struct MemRegion {
    uint64_t start_addr;
    uint64_t len_bytes;
    uint64_t bitmap_start_addr;
    uint64_t free_frames;
}mem_region;

/**
 * struct for representing free memory frames.
 * underlying bitmap (0 = free, 1 = set)
 */
typedef struct MemMap {
    uint64_t mem_size;
    uint8_t region_count;
    mem_region regions[MAX_MEM_REGIONS];
    bit_map bitmap;
}mem_map;

typedef uint64_t phys_addr_t;
typedef uint64_t virt_addr_t;

/**
 * return value of alloc_frames and input to dealloc frames
 * (the struct_page itself is the first allocated frames
 */
typedef struct StructPage {
    uint64_t page_count;
    uint64_t page_size;
    phys_addr_t page_addr[];
}struct_page_t;


void memmap_register_region(mem_map *mmap, uint64_t start, uint64_t end);
void show_mem_map(mem_map *mmap, bool verbose);
phys_addr_t map_alloc_kframe(mem_map *mmap, uint64_t size);
int map_dealloc_frame(mem_map *mmap, phys_addr_t phys_addr, uint64_t page_size);
int zero_mem_region(mem_map *mmap, uint64_t start_addr, uint64_t end_addr);
int set_mem_region(mem_map *mmap, uint64_t start_addr, uint64_t end_addr);
void zero_mem_map(mem_map *mmap);
phys_addr_t ialloc_kframe(uint64_t pages, uint64_t page_size);
struct_page_t *ialloc_frames(uint64_t count, uint64_t page_size);
int idealloc_frames(struct_page_t *info_page);

#endif //ICARUS_MEM_MAP_H
