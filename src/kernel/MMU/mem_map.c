//
// Created by cocon on 25.09.2026.
//

#include "mem_map.h"

#include <stdbool.h>

#include "../util/BitMap.h"

#include <stddef.h>

#include "../../lib/mem_utils.h"
#include "../../shell/shellio.h"
#include "../util/kernel_info.h"


void memmap_register_region(mem_map *mmap, uint64_t start, uint64_t end) {
    mmap->mem_size += end - start;
    mmap->regions[mmap->region_count].start_addr = start;
    mmap->regions[mmap->region_count].len_bytes = end - start;
    mmap->regions[mmap->region_count].free_frames = (end - start) / DEFAULT_PAGE_SIZE;

    if (mmap->region_count == 0) {
        mmap->regions[mmap->region_count].bitmap_start_addr = 0;
    }else {
        mem_region prev_region = mmap->regions[mmap->region_count - 1];
        mmap->regions[mmap->region_count].bitmap_start_addr = prev_region.bitmap_start_addr + prev_region.len_bytes / DEFAULT_PAGE_SIZE;
    }

    mmap->bitmap.size += (end - start) / DEFAULT_PAGE_SIZE;
    mmap->region_count++;
}

/**
 * zeros out the entire bitmap (use for fresh systems)
 */
void zero_mem_map(mem_map *mmap) {
    Imemset(mmap->bitmap.data, 0, mmap->bitmap.size);
}

/**
 *
 * sets a region of memory to 1(filled)
 * @param start_addr a 4096 aligned physical addr
 * @param end_addr a 4096 aligned physical addr
 * @return
 */
int set_mem_region(mem_map *mmap, uint64_t start_addr, uint64_t end_addr) {
    if (start_addr < mmap->regions[0].start_addr || end_addr > mmap->regions[mmap->region_count - 1].start_addr + mmap->regions[mmap->region_count - 1].len_bytes) {
        return -1;
    }

    uint64_t len = end_addr - start_addr;

    for (uint32_t i = 0; i < mmap->region_count; i++) {
        mem_region *region = &mmap->regions[i];
        //cons_mprintf("looping[%i]: start %x - %x; end: %x - %x\n",i, region->start_addr, start_addr,region->start_addr + region->len_bytes, end_addr);
        if (region->start_addr > start_addr || end_addr > region->start_addr + region->len_bytes )
            continue;

        region->free_frames -= len / DEFAULT_PAGE_SIZE;
        uint64_t effective_addr = start_addr - region->start_addr;
        //cons_mprintf("actually setting bitmap[%i]: %x + %x (%x) = %x\n",i, region->bitmap_start_addr, effective_addr / DEFAULT_PAGE_SIZE, len / DEFAULT_PAGE_SIZE, region->free_frames);
        bitmap_set_len(&mmap->bitmap, region->bitmap_start_addr + effective_addr / DEFAULT_PAGE_SIZE , len / DEFAULT_PAGE_SIZE);
        break;
    }

    return 0;
}

int zero_mem_region(mem_map *mmap, uint64_t start_addr, uint64_t end_addr) {
    if (start_addr < mmap->regions[0].start_addr || end_addr > mmap->regions[mmap->region_count - 1].start_addr + mmap->regions[mmap->region_count - 1].len_bytes) {
        return -1;
    }

    uint64_t len = end_addr - start_addr;

    for (uint32_t i = 0; i < mmap->region_count; i++) {
        mem_region *region = &mmap->regions[i];
        if (region->start_addr > start_addr || start_addr + len > region->start_addr + region->len_bytes )
            continue;

        region->free_frames -= len / DEFAULT_PAGE_SIZE;
        uint64_t effective_addr = start_addr - region->start_addr;
        bitmap_clear_len(&mmap->bitmap, region->bitmap_start_addr + effective_addr / DEFAULT_PAGE_SIZE , len / DEFAULT_PAGE_SIZE);
        break;
    }

    return 0;
}

void show_mem_map(mem_map *mmap, bool verbose) {
    for (uint32_t i = 0; i < mmap->region_count; i++) {
        mem_region *region = &mmap->regions[i];
        cons_mprintf("Memory Region[%i] {addr: %x len: %x \n  (%x Frames) free: (%x Frames)}\n", i, region->start_addr, region->len_bytes, region->len_bytes / DEFAULT_PAGE_SIZE, region->free_frames);
        if (verbose) {
            cons_mprintf("View: \n");
            show_bit_map_range(&mmap->bitmap, region->bitmap_start_addr, region->len_bytes / DEFAULT_PAGE_SIZE);
        }
    }
}

/**
 * Allocate a physical frame from a given memory map and page size
 *
 */
phys_addr_t alloc_frame(mem_map *mmap, uint64_t page_size) {

    // default page size = 4096 while the page size param could be a huge or super page
    uint64_t cont_units = page_size / DEFAULT_PAGE_SIZE;
    //cons_mprintf("Allocating memory for frame [%i]\n", cont_units);
    uint64_t count = 0;
   for (uint32_t i = 0; i < mmap->region_count; i++) {
       mem_region * region = &mmap->regions[i];
       if (region->free_frames < cont_units) {
           continue;
       }

       for (uint64_t j = region->bitmap_start_addr; j < region->len_bytes / DEFAULT_PAGE_SIZE; j++) {
           uint64_t byte_ind = j / 8;
           uint64_t bit_ind = j % 8;
           if (((mmap->bitmap.data[byte_ind] >> bit_ind) & 0x1) == 0) {
               // false = 0 = free;
               count++;
           }else {
               //cons_mprintf("full frame[%i] - full byte[%i]=%i\n", j, byte_ind, region.bitmap.data[byte_ind]);
               count = 0;
                continue;
           }

           if (count == cont_units) {
               //success
                uint32_t start = j - (count - 1);
               //cons_mprintf("starting at: %i\n", start);
               //manually setting bits bc I'm sure it'll be faster (100% cope)
               for (uint64_t k = start; k < start + count; k++) {
                   byte_ind = k / 8;
                   bit_ind = k % 8;
                   mmap->bitmap.data[byte_ind] |= (0x1 << bit_ind);
               }
               region->free_frames -= cont_units;
               return (region->start_addr + start * DEFAULT_PAGE_SIZE);
           }
       }
   }
    return FRAME_ALLOC_FAILED;
}