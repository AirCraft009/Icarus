//
// Created by cocon on 27.09.2026.
//

#ifndef ICARUS_ALLOC_H
#define ICARUS_ALLOC_H
#pragma once
#define MAX_ALLOCATION_SIZE
#include <stdint.h>
#include "../../page_definitions.h"
#include "PMM/mem_map.h"


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
typedef struct Allocator {
    uint64_t heapStart;
    uint64_t heapSize;
}alloc_t;

alloc_t *init_custom_alloc (uint64_t initial_size, void * heap_start, struct_page_t * info_page) ;
alloc_t * init_alloc(uint64_t initial_size, void * heap_start);
void * imalloc_alocator(alloc_t * allocator, uint64_t size);
void free(void * addr);

#endif //ICARUS_ALLOC_H
