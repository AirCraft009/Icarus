//
// Created by cocon on 27.09.2026.
//

#include "alloc.h"

#include "memory_mapping.h"
#include "../../lib/mem_utils.h"
#include "../util/kernel_helper.h"

/**
 * initialize a heap by creating an allocator
 * @param heap_start the virtual address of the heap Page aligned
 * @param initial_size the size of the heap in bytes
 * @return ptr to the allocator (same as alloc_page)
 */
alloc_t *init_heap (uint64_t initial_size, uint64_t heap_start) {
    uint64_t pages = ALIGN_UP(initial_size, DEFAULT_PAGE_SIZE) / DEFAULT_PAGE_SIZE;
    initial_size = pages * DEFAULT_PAGE_SIZE;
    alloc_t * allocator = (alloc_t *) ialloc_frames(1, DEFAULT_PAGE_SIZE);
    heap_start = ALIGN_DOWN(heap_start, DEFAULT_PAGE_SIZE);

    allocator->heapStart = heap_start;
    allocator->heapSize = initial_size;

    struct_page_t * info_page = ialloc_frames(pages, DEFAULT_PAGE_SIZE);
    Imemcpy(&allocator->info_page.page_addr, &info_page->page_addr, sizeof(phys_addr_t) * info_page->page_count);
    allocator->info_page.page_count = info_page->page_count;
    allocator->info_page.page_size = info_page->page_size;

    for (uint64_t i = 0; i < pages; i++) {
        page_in(
            (page_map_l4_entry *) read_cr3(),
            (void *) heap_start + i,
            (void *) info_page->page_addr[i + 1],
            DEFAULT_PAGE_SIZE
        );
    }

    alloc_meta_data_t * header = (alloc_meta_data_t *) heap_start;
    alloc_meta_data_t * footer = (alloc_meta_data_t *) ((heap_start + initial_size) - sizeof(alloc_meta_data_t));

    header->alloc_segments = initial_size / MIN_ALLOC_SIZE;
    header->allocated = 0;
    footer->alloc_segments = initial_size / MIN_ALLOC_SIZE;
    footer->allocated = 0;

    return allocator;
}

void free_heap (alloc_t * allocator) {
    idealloc_frames(&allocator->info_page);
}

void * ialloc(alloc_t * allocator, uint64_t size) {
    size = ALIGN_UP(size, MIN_ALLOC_SIZE);

    alloc_meta_data_t * header = (alloc_meta_data_t *) allocator->heapStart;
    if (!header->allocated) {

    }
}

