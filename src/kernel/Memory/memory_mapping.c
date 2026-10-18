//
// Created by cocon on 20.09.2026.
//

#include "kernel/Memory/memory_mapping.h"

#include <stdint.h>

#include "kernel_helper.h"
#include "kernel/Memory/paging.h"
#include "kernel/util/mem_utils.h"
#include "kernel/util/shellio.h"

/*
 *  handles paging in and out
*/


extern char _kernel_end[];
extern char _kernel_end_phys[];
extern char _kernel_start_phys[];

//static const page_map_l4_entry KERNEL_PML4 [512];
static uint64_t MemmapStart;
static mem_map *mmap;

int page_in_regions(mem_map *mmap, page_map_l4_entry * pml4);


/**
 * initiate the kernel physical memory map located after the kernel end
 * reads the multiboot2 mmap info tag and build a bitmap w/ 4KiB blocks
 *
 * @param mmap_tag multiboot2 mmap_tag info
 * @return the size of the mmap (block count)
 */
int handle_mb2_mmap(struct multiboot_tag *mmap_tag, page_map_l4_entry *pml4) {
    //phys addr of method 0x107812
    MemmapStart = (uint64_t) &_kernel_end;
    Iprintf("INIT_MMAP: %x == %x -> %x\n", _kernel_start_phys, &_kernel_end, &_kernel_end_phys);

    mmap = (mem_map *) MemmapStart;
    mmap->region_count = 0;
    mmap->mem_size = 0;
    mmap->bitmap.size = 0;

    struct multiboot_mmap_entry *mmap_entry;

    for (mmap_entry = ((struct multiboot_tag_mmap *) mmap_tag)->entries;
        (multiboot_uint8_t *) mmap_entry < (multiboot_uint8_t *) mmap_tag + mmap_tag->size;
        mmap_entry = (multiboot_memory_map_t *) ((unsigned long) mmap_entry +
        ((struct multiboot_tag_mmap *) mmap_tag)->entry_size)) {


        if (mmap_entry->addr + mmap_entry->len > mmap->mem_size) {
            mmap->mem_size = mmap_entry->addr + mmap_entry->len;
        }

        uint64_t len;

        if (mmap_entry->type == MULTIBOOT_MEMORY_AVAILABLE) {
            // available: round inward so we never over-claim a partial page
            uint64_t start = ALIGN_UP(mmap_entry->addr, DEFAULT_PAGE_SIZE);
            uint64_t end = ALIGN_DOWN(mmap_entry->addr + mmap_entry->len, DEFAULT_PAGE_SIZE);
            if (end > start) {
                Iprintf("addrF: %x - %x = len: %i\n", start, end, end - start);
                memmap_register_region(mmap, start, end);
            }
        }
    }

    zero_mem_map(mmap);


    //mark the kernel and bitmap itself as not available
    int err = set_mem_region(mmap,
        ALIGN_DOWN((uint64_t) &_kernel_start_phys, DEFAULT_PAGE_SIZE),
        KERNEL_VIRT_TO_PHYS(ALIGN_UP((uint64_t) mmap->bitmap.data + mmap->bitmap.size, DEFAULT_PAGE_SIZE ))
    );

    //show_mem(true);
    if (err != 0) {
        Iprintf("ERROR: set_mem_region returned %d\n", err);
        return -1;
    }

    if ( page_in_regions(mmap, pml4) != 0) {
        return -1;
    };
    //show_mem_map(mmap, false);
    return 0;
}

int page_in_regions(mem_map *mmap, page_map_l4_entry * pml4) {
    uint64_t start;
    uint64_t end;
    for (uint64_t i = 0; i < mmap->region_count; i++) {
        start = mmap->regions[i].start_addr;
        end = start + mmap->regions[i].len_bytes;

        uint64_t addr = ALIGN_UP(start, HUGE_PS);
        while (addr < end) {
            uint64_t remaining = end - addr;
            uint64_t sz;

            if ((addr % SUPER_PS) == 0 && remaining >= SUPER_PS)
                sz = SUPER_PS;
            else if (remaining >= HUGE_PS)
                sz = HUGE_PS;
            else
                sz = DEFAULT_PAGE_SIZE;

            if (
                page_in(
                    pml4,
                    (void *)KERNEL_PHYS_TO_VIRT(addr),
                    (void *)addr,
                    HUGE_PS,
                    WRITEABLE) != 0)
            {
                return -1;
            }
            addr += HUGE_PS;

        }
    }
    return 0;
}