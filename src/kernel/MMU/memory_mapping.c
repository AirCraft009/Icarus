//
// Created by cocon on 20.09.2026.
//

#include "memory_mapping.h"

#include <stdint.h>

#include "../kernel_info.h"
#include "../kernel_helper.h"

/*
 *  handles paging in and out
*/

extern uint8_t _kernel_end;
extern uint8_t _kernel_end_phys;
static uint64_t BitmapStart;


extern uint64_t pml4[512], pdpt[512], pd[512], pt[512];

int init_mmap(struct multiboot_tag *mmap_tag) {
    cursor cur = {0, 5};

    BitmapStart = (uint64_t)&_kernel_end_phys;
    bitmap *bit_mmap = (bitmap *) BitmapStart; // FIX: value, not address-of
    cons_mprintf( "START: %l", BitmapStart);

    struct multiboot_mmap_entry *mmap;

    for (mmap = ((struct multiboot_tag_mmap *) mmap_tag)->entries;
        (multiboot_uint8_t *) mmap < (multiboot_uint8_t *) mmap_tag + mmap_tag->size;
        mmap = (multiboot_memory_map_t *) ((unsigned long) mmap +
        ((struct multiboot_tag_mmap *) mmap_tag)->entry_size)) {

        if (mmap->addr + mmap->len > bit_mmap->size) {
            bit_mmap->size = mmap->addr + mmap->len;
        }

        uint64_t index, bits;

        if (mmap->type != MULTIBOOT_MEMORY_AVAILABLE) {
            // reserved: round outward so we never under-cover it
            uint64_t start = ALIGN_DOWN(mmap->addr, PAGE_SIZE);
            uint64_t end   = ALIGN_UP(mmap->addr + mmap->len, PAGE_SIZE);
            index = start / PAGE_SIZE;
            bits  = (end - start) / PAGE_SIZE;
            cons_mprintf( "addrG: %l len: %l\n", mmap->addr, mmap->len);
            bitmap_set_values(bit_mmap, index, bits);
        } else {
            // available: round inward so we never over-claim a partial page
            uint64_t start = ALIGN_UP(mmap->addr, PAGE_SIZE);
            uint64_t end   = ALIGN_DOWN(mmap->addr + mmap->len, PAGE_SIZE);
            if (end > start) {
                index = start / PAGE_SIZE;
                bits  = (end - start) / PAGE_SIZE;
                cons_mprintf( "addrF: %l len: %l\n", mmap->addr, mmap->len);
                bitmap_clear_values(bit_mmap, index, bits);
            }
        }
    }
    cons_mprintf( "Final out");
    return 0;
}

int page_in(uint64_t physical_address, uint64_t virtual_address, uint8_t pageT) {
    uint64_t size;
    switch(pageT) {
        case NPage:
            size = PageS;
            break;
        case HPage:
            size = HugePS;
            break;
        case SPage:
            size = SuperPs;
            break;
        default:
            return -1;
    }

    return -1;
}

