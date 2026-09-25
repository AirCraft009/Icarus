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
    mmap->regions[mmap->region_count].end_addr = end;
    mmap->regions[mmap->region_count].free_frames = (end - start) / DEFAULT_PAGE_SIZE;
    mmap->regions[mmap->region_count].bitmap.size = (end - start) / DEFAULT_PAGE_SIZE;

    // now zero init the region as all regions are empty on registering.
    void * map_start = &mmap->regions[mmap->region_count].bitmap.data;
    Imemset(map_start, 0, (end - start) / DEFAULT_PAGE_SIZE);
    mmap->region_count++;
}

void show_mem_map(mem_map *mmap, bool verbose) {
    for (uint32_t i = 0; i < mmap->region_count; i++) {
        mem_region region = mmap->regions[i];
        cons_mprintf("Memory Region[%i] {range: %x - %x \n  (%x Frames) free: (%x Frames)}\n", i, region.start_addr, region.end_addr, region.bitmap.size, region.free_frames);
        if (verbose) {
            cons_mprintf("View: \n");
            show_bit_map(&region.bitmap);
        }
    }
}

/**
 * Allocate a physical frame from a given memory map and page size
 *
 */
void * alloc_frame(mem_map *mmap, uint64_t page_size) {

    // default page size = 4096 while the page size param could be a huge or super page
    uint64_t cont_units = page_size / DEFAULT_PAGE_SIZE;
    uint64_t count = 0;
   for (uint32_t i = 0; i < mmap->region_count; i++) {
       mem_region region = mmap->regions[i];
       if (region.free_frames < cont_units)
           continue;

       for (uint32_t j = 0; j < region.bitmap.size; j++) {
           uint64_t byte_ind = j / 8;
           uint64_t bit_ind = j % 8;
           if (region.bitmap.data[byte_ind] >> bit_ind & 0x1) {
               // false = 0 = free;
               count++;
           }else {
               count = 0;
                continue;
           }

           if (count == cont_units) {
               //success

               //manually setting bit bc I'm sure it'll be faster (100% cope)
               region.bitmap.data[byte_ind] |= (0x1 << bit_ind);
               region.free_frames -= cont_units;
               return (void *)(region.start_addr + j - (count - 1) * DEFAULT_PAGE_SIZE);
           }
       }
   }
    return NULL;
}