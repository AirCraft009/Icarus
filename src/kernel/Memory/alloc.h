//
// Created by cocon on 27.09.2026.
//

#ifndef ICARUS_ALLOC_H
#define ICARUS_ALLOC_H
#pragma once
#define MAX_ALLOCATION_SIZE
#include <stdint.h>

#include "mem_map.h"
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
#define ALIGNMENT 16

/**
 * Allocation Meta-data (header and footer)
 * in a header alloc_segments points forward
 * in a footer it points backwards
 */
typedef struct AllocMetaData {
    uint64_t alloc_segments : 31; // size in 16byte chunks
    uint64_t allocated : 1;
}__attribute__((packed)) alloc_meta_data_t;

typedef struct Allocator {
    uint64_t heapStart;
    uint64_t heapSize;
    struct_page_t info_page;
}alloc_t;

#endif //ICARUS_ALLOC_H
