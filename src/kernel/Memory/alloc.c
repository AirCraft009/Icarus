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
 * @param initial_size the size of the heap in bytes including a 4KiB frame for the allocator
 * @return ptr to the allocator (same as alloc_page)
 */
alloc_t *init_custom_alloc (uint64_t initial_size, uint64_t heap_start, struct_page_t * info_page) {
    page_map_l4_entry * pml4 = (page_map_l4_entry *) read_cr3();
    // page in the struct page bc otherwise all access to it will result in a #PF
    // it is at heap_start + 4096 bc the allocator itself is at heap_start
    if (
        page_in(
            pml4,
            (void *) heap_start + DEFAULT_PAGE_SIZE,
            (void *) info_page,
            DEFAULT_PAGE_SIZE) != 0)
    {
        return NULL;
    }
    info_page = (struct_page_t *)heap_start + DEFAULT_PAGE_SIZE;

    uint64_t pages = ALIGN_UP(initial_size, DEFAULT_PAGE_SIZE) / DEFAULT_PAGE_SIZE;
    initial_size = pages * DEFAULT_PAGE_SIZE;
    alloc_t * phys_allocator = (alloc_t *) ialloc_frames(1, DEFAULT_PAGE_SIZE); //TODO: map to virt at start and figure out how big we want the allocator to be
    heap_start = ALIGN_DOWN(heap_start, DEFAULT_PAGE_SIZE);

    if (
        page_in(
        pml4,
        (void *) heap_start,
        (void *) phys_allocator,
        DEFAULT_PAGE_SIZE) != 0 )
    {
        return NULL;
    }

    for (uint64_t i = 2; i < pages; i++) {
        if (
            page_in(
            pml4,
            (void *) heap_start + i * DEFAULT_PAGE_SIZE,
            (void *) info_page->page_addr[i + 1],
            DEFAULT_PAGE_SIZE) != 0)
        {
            return NULL;
        }
    }


    alloc_t * allocator = (alloc_t *) heap_start;
    allocator->heapStart = ALIGN_UP(heap_start + sizeof(struct Allocator), HEAP_ALIGNMENT);
    allocator->heapSize = initial_size;
    alloc_meta_data_t * header = (alloc_meta_data_t *) heap_start;

    header->allocated = 0;
    header->block_size = initial_size;
    header->prev_block_data = 0;

    return allocator;
}


/**
 * initializes the kernel_heap and heap allocator.
 *
 * @param initial_size size of the heap at t = 0 in bytes
 * @param heap_start the virtual addr of the kernel
 */
alloc_t * init_alloc(uint64_t initial_size, uint64_t heap_start) {
    uint64_t pages = ALIGN_UP(initial_size, DEFAULT_PAGE_SIZE) / DEFAULT_PAGE_SIZE;
    return init_custom_alloc(initial_size, heap_start, ialloc_frames(pages, DEFAULT_PAGE_SIZE));
}


int map_frame_into_heap(alloc_t * allocator, page_map_l4_entry *pml4,  struct_page_t *info_page) {
    for (uint64_t i = 0; i < info_page->page_count; i++) {
        if (
            page_in(
            pml4,
            (void *) allocator->heapStart + allocator->heapSize + i * DEFAULT_PAGE_SIZE,
            (void *) info_page->page_addr[i],
            DEFAULT_PAGE_SIZE) != 0)
        {
            return -1;
        }
    }
    return 0;
}

/**
 * first fit allocator
 */
void * imalloc(alloc_t * allocator, uint64_t size) {
    size += META_DATA_SIZE;      // make the total size
    size = ALIGN_UP(size, HEAP_ALIGNMENT);

    while (1) {
        alloc_meta_data_t * header = (alloc_meta_data_t *) allocator->heapStart;
        while ((uint64_t) header + header->block_size < allocator->heapStart + allocator->heapSize) {
            if (!header->allocated && header->block_size == size) {
                header->allocated = 1;
                // don't know why (void *) (uint64_t) should do anything (I don't think it does) but the CLANG warning goes away so
                return (void *) (uint64_t) header + META_DATA_SIZE;
            }
            if (!header->allocated && header->block_size > size) {
                alloc_meta_data_t *next_block = (alloc_meta_data_t *) header + header->block_size;
                uint64_t remaining_s = header->block_size - size;               //remaining_s can't be < 16 (bc it's all aligned to 16)
                next_block->allocated = 0;
                next_block->block_size = remaining_s;
                next_block->prev_block_data = size;
                return (void *) (uint64_t) header + META_DATA_SIZE;
            }

            header = header + header->block_size;
        }

        // ask for more memory from the kernel
        // * 30 / 10 bc SSE doesn't work or smth and yk... but I'm a genius
        struct_page_t *info_page = (struct_page_t *) KERNEL_VIRT_TO_PHYS(ialloc_frames(ALIGN_UP((uint64_t) (allocator->heapSize * 30 / 10), DEFAULT_PAGE_SIZE), DEFAULT_PAGE_SIZE));
        if (map_frame_into_heap(allocator, (page_map_l4_entry *) read_cr3(), info_page) != 0) {
            return NULL;
        }
        allocator->heapSize += info_page->page_count * DEFAULT_PAGE_SIZE;
    }
}


