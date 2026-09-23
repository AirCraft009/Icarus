//
// Created by cocon on 20.09.2026.
//

#include "memory_mapping.h"

#include <stdint.h>

#include "../kernel_info.h"
#include "../kernel_helper.h"
#include "page_definitions.h"

/*
 *  handles paging in and out
*/



extern uint64_t pml4[512], pdpt[512], pd[512], pt[512];
extern uint8_t _kernel_end;
extern uint8_t _kernel_end_phys;

static uint64_t BitmapStart;
static bitmap * mmap;
static page_table_t * page_table;


int init_mmap(struct multiboot_tag *mmap_tag) {

    BitmapStart = (uint64_t)&_kernel_end_phys;
    mmap = (bitmap *) BitmapStart;
    cons_mprintf( "Bitmap_start: %l\n", BitmapStart);

    struct multiboot_mmap_entry *mmap_entry;

    for (mmap_entry = ((struct multiboot_tag_mmap *) mmap_tag)->entries;
        (multiboot_uint8_t *) mmap_entry < (multiboot_uint8_t *) mmap_tag + mmap_tag->size;
        mmap_entry = (multiboot_memory_map_t *) ((unsigned long) mmap_entry +
        ((struct multiboot_tag_mmap *) mmap_tag)->entry_size)) {

        if (mmap_entry->addr + mmap_entry->len > mmap->size) {
            mmap->size = mmap_entry->addr + mmap_entry->len;
        }

        uint64_t index, bits;

        if (mmap_entry->type != MULTIBOOT_MEMORY_AVAILABLE) {
            // reserved: round outward so we never under-cover it
            uint64_t start = ALIGN_DOWN(mmap_entry->addr, PAGE_SIZE);
            uint64_t end   = ALIGN_UP(mmap_entry->addr + mmap_entry->len, PAGE_SIZE);
            index = start / PAGE_SIZE;
            bits  = (end - start) / PAGE_SIZE;
            //cons_mprintf( "addrG: %l len: %l\n", mmap->addr, mmap->len);
            bitmap_set_values(mmap, index, bits);
        } else {
            // available: round inward so we never over-claim a partial page
            uint64_t start = ALIGN_UP(mmap_entry->addr, PAGE_SIZE);
            uint64_t end   = ALIGN_DOWN(mmap_entry->addr + mmap_entry->len, PAGE_SIZE);
            if (end > start) {
                index = start / PAGE_SIZE;
                bits  = (end - start) / PAGE_SIZE;
                //cons_mprintf( "addrF: %l len: %l\n", mmap->addr, mmap->len);
                bitmap_clear_values(mmap, index, bits);
            }
        }
    }
    return mmap->size;
}

int page_in(page_frame *pf, uint8_t pageT) {
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

