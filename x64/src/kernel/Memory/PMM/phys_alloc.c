//
// Created by cocon on 17.11.2026.
//

#include "kernel_helper.h"
#include "kernel/Memory/memory_mapping.h"
#include "kernel/Memory/PMM/phys_alloc.h"

#include "kernel/Memory/PMM/mem_map.h"
#include "kernel/util/shellio.h"


extern char _kernel_end[];

/**
 * reserves the give space (set as unavailable) so that it isn't given out
 * @param phys_addr the physical start addr
 * @param size the size of the area
 * @return 0 on success -1 on fail
 */
int reserve_mem(phys_addr_t phys_addr, uint64_t size) {
    mem_map * mmap = (mem_map *) &_kernel_end;
    return set_mem_region(mmap, phys_addr, phys_addr + size);
}


void show_mem(bool verbose) {
    show_mem_map((mem_map *) &_kernel_end, verbose);
}


/**
 * allocate frames.
 * tries to allocate at least 1 frame even on 0 requested
 *
 * @param count amount of pages allocated < 512
 * @param page_type size of each allocated page
 * @return FRAME_ALLOC_FAILED on fail
 */
struct_page_t *ialloc_frames(uint64_t count, PAGE_TYPE page_type) {
    //TODO: make an independent method instead of relying on map_alloc_frame
    //TODO: make structpage a linkedlist so that arbitrary amount of pages can be allocated
    if (count > 510)        // rn this bc the struct_page only has space for 510 entries  (will build a linked list later)
        return (struct_page_t *) FRAME_ALLOC_FAILED;

    uint64_t page_size = PAGE_TYPE_TO_SIZE(page_type);
    mem_map * mmap = (mem_map *) &_kernel_end;
    struct_page_t *info_page = (struct_page_t *) KERNEL_PHYS_TO_VIRT(map_alloc_frame(mmap, page_type));
    info_page->page_count = count;
    info_page->page_addr[0] = (phys_addr_t) KERNEL_VIRT_TO_PHYS(info_page);
    info_page->page_size = page_size;

    for (uint64_t i = 1; i < count; i++) {
        info_page->page_addr[i] = map_alloc_frame(mmap, page_type);
        if (info_page->page_addr[i] == FRAME_ALLOC_FAILED) {
            Iprintf("allocation failed\n");
            return (struct_page_t *) FRAME_ALLOC_FAILED;
        }
    }


    // for (uint64_t i = 0; i < count; i++) {
    //     cons_mprintf("actually allocated: %x\n", info_page->page_addr[i]);
    // }
    return (struct_page_t *) (uint64_t) info_page;
}

/**
 * free a list of physical frames
 * @param info_page the virt addr of a struct page
 */
int idealloc_frames(struct_page_t * info_page) {
    mem_map * mmap = (mem_map *) &_kernel_end;
    for (uint64_t i = 1; i < info_page->page_count; i++) {
        if (map_dealloc_frame(mmap,  ALIGN_DOWN(info_page->page_addr[i], info_page->page_size), info_page->page_size) != 0) {
            return -1;
        }
    }

    map_dealloc_frame(mmap, ALIGN_DOWN(info_page->page_addr[0], DEFAULT_PAGE_SIZE), info_page->page_size);
    return 0;
}

phys_addr_t alloc_kframe(uint64_t size) {
    mem_map * mmap = (mem_map *) &_kernel_end;
    return map_alloc_frame_contiguous(mmap, ALIGN_UP(size, DEFAULT_PAGE_SIZE) / DEFAULT_PAGE_SIZE);
}

int dealloc_kframe(phys_addr_t start, uint64_t size) {
    mem_map * mmap = (mem_map *) &_kernel_end;
    return map_dealloc_frame(mmap, start, size);
}

/**
 * Allocate contiguous physical memory
 * @size is the total size of all 4KiB regions (should be aligned to 4KiB)
 */
phys_addr_t ialloc_kframe(uint64_t pages, PAGE_TYPE page_size) {
    mem_map * mmap = (mem_map *) &_kernel_end;
    return map_alloc_frame_contiguous(mmap, pages);
}

