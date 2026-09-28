//
// Created by cocon on 27.09.2026.
//

#ifndef ICARUS_ALLOC_H
#define ICARUS_ALLOC_H
#pragma once
#define MAX_ALLOCATION_SIZE
#include <stdint.h>

#include "PMM/mem_map.h"
#include "page_definitions.h"


/*
 * Heap allocator
 *
 * Allocates Memory on the heap!
 * each allocated block also has it's total length
 * located at addr - 8b;
 *
 * It is used to free any the
 *
 */

#define MIN_ALLOC_SIZE 32
#define HEAP_ALIGNMENT 16
#define META_DATA_SIZE sizeof(alloc_meta_data_t)
#define NO_PREV_ENTRY 0


/**
 * Allocation Meta-data
 * takes role of header and footer
 * by having the size of the current block
 * as well as the offset to the prev's blocks metadata
 */
typedef struct AllocMetaData {
    uint64_t block_size : 63;
    uint64_t allocated : 1;
    uint64_t prev_block_data;       // offset to prev block from AllocMetadata * (0 for first block)
}__attribute__((packed)) alloc_meta_data_t;

typedef struct Allocator {
    uint64_t heapStart;
    uint64_t heapSize;
}alloc_t;

alloc_t *init_custom_alloc (uint64_t initial_size, uint64_t heap_start, struct_page_t * info_page);
alloc_t * init_alloc(uint64_t initial_size, uint64_t heap_start);
void * imalloc(alloc_t * allocator, uint64_t size);

#endif //ICARUS_ALLOC_H
