//
// Created by cocon on 25.09.2026.
//

#include "kernel/Memory/PMM/mem_map.h"

#include <stdbool.h>

#include <stddef.h>

#include "kernel_helper.h"
#include "kernel/Memory/memory_mapping.h"
#include "kernel/util/BitMap.h"
#include "kernel/util/mem_utils.h"
#include "kernel/util/shellio.h"

extern char _kernel_end[];

phys_addr_t map_alloc_frame_contiguous(mem_map * mmap, uint64_t page_count, char * calling);
phys_addr_t map_alloc_frame(mem_map * mmap, PAGE_TYPE page_size, char * calling);

void memmap_register_region(mem_map *mmap, uint64_t start, uint64_t end) {

    uint64_t len = end - start;
    uint64_t real_frames = len / DEFAULT_PAGE_SIZE;
    mmap->mem_size += len;
    mmap->regions[mmap->region_count].start_addr = start;
    mmap->regions[mmap->region_count].len_bytes = len;
    mmap->regions[mmap->region_count].qword_count = ALIGN_UP(real_frames, 64) / 64;      // align up for better parsing
    mmap->regions[mmap->region_count].free_frames = real_frames;

    Iprintf("reg(%x - %x); frames: %x, b: %x, qw: %x\n", start, end, real_frames, len, ALIGN_UP(real_frames, 64) / 64);

    if (mmap->region_count == 0) {
        mmap->regions[mmap->region_count].bitmap_start_addr = 0;
    }else {
        mem_region prev_region = mmap->regions[mmap->region_count - 1];
        // make sure the 64bit padding is taken into account
        mmap->regions[mmap->region_count].bitmap_start_addr = prev_region.bitmap_start_addr + prev_region.qword_count * 64;
    }

    // pad to 64 bits
    mmap->bitmap.size += mmap->regions[mmap->region_count].qword_count * 64;
    mmap->region_count++;
}

/**
 * zeros out the bitmap and sets the padded spaces to 1 (use for fresh systems)
 */
void zero_mem_map(mem_map *mmap) {
    Imemset(mmap->bitmap.data, 0, mmap->bitmap.size / 8);

    for (uint32_t i = 0; i < mmap->region_count; i++) {
        mem_region *region = &mmap->regions[i];
        uint64_t fr_count = region->free_frames;
        set_mem_region(mmap, region->bitmap_start_addr + region->len_bytes,  region->bitmap_start_addr + (region->qword_count * 64) * DEFAULT_PAGE_SIZE);
        region->free_frames = fr_count;     // restore frame count
    }
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
    //Iprintf("setting: (%x - %x)\n", start_addr, len / DEFAULT_PAGE_SIZE);

    for (uint32_t i = 0; i < mmap->region_count; i++) {
        mem_region *region = &mmap->regions[i];
        //cons_mprintf("looping[%i]: start %x - %x; end: %x - %x\n",i, region->start_addr, start_addr,region->start_addr + region->len_bytes, end_addr);
        if (region->start_addr > start_addr || end_addr > region->start_addr + region->len_bytes )
            continue;

        region->free_frames -= len / DEFAULT_PAGE_SIZE;
        uint64_t effective_addr = start_addr - region->start_addr;
        //Iprintf("actually setting bitmap[%i]: %x + %x (%x) = %x\n",i, region->bitmap_start_addr, effective_addr / DEFAULT_PAGE_SIZE, len / DEFAULT_PAGE_SIZE, region->free_frames);
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

        region->free_frames += len / DEFAULT_PAGE_SIZE;
        uint64_t effective_addr = start_addr - region->start_addr;
        bitmap_clear_len(&mmap->bitmap, region->bitmap_start_addr + effective_addr / DEFAULT_PAGE_SIZE , len / DEFAULT_PAGE_SIZE);
        break;
    }

    return 0;
}

void show_mem(bool verbose) {
    show_mem_map((mem_map *) &_kernel_end, verbose);
}

void show_mem_map(mem_map *mmap, bool verbose) {
    for (uint32_t i = 0; i < mmap->region_count; i++) {
        mem_region *region = &mmap->regions[i];
        Iprintf("Memory Region[%i] {addr: %x len: %x \n  (%x Frames) free: (%x Frames)}\n", i, region->start_addr, region->len_bytes, region->len_bytes / DEFAULT_PAGE_SIZE, region->free_frames);
        if (verbose) {
            Iprintf("View: \n");
            show_bit_map_range(&mmap->bitmap, region->bitmap_start_addr, region->len_bytes / DEFAULT_PAGE_SIZE);
        }
    }
}


/**
 * allocate frames.
 * tries to allocate at least 1 frame even on 0 requested
 *
 * @param count amount of pages allocated < 512
 * @param page_type size of each allocated page
 * @return NULL on fail
 */
struct_page_t *ialloc_frames(uint64_t count, PAGE_TYPE page_type, char * calling) {
    //TODO: make an independent method instead of relying on map_alloc_frame
    //TODO: make structpage a linkedlist so that arbitrary amount of pages can be allocated
    if (count > 510)        // rn this bc the struct_page only has space for 510 entries  (will build a linked list later)
        return (struct_page_t *) FRAME_ALLOC_FAILED;

    uint64_t page_size = PAGE_TYPE_TO_SIZE(page_type);
    mem_map * mmap = (mem_map *) &_kernel_end;
    struct_page_t *info_page = (struct_page_t *) KERNEL_PHYS_TO_VIRT(map_alloc_frame(mmap, page_type, calling));
    info_page->page_count = count;
    info_page->page_addr[0] = (phys_addr_t) KERNEL_VIRT_TO_PHYS(info_page);
    info_page->page_size = page_size;

    for (uint64_t i = 1; i < count; i++) {
        info_page->page_addr[i] = map_alloc_frame(mmap, page_type, calling);
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
    return map_alloc_frame_contiguous(mmap, ALIGN_UP(size, DEFAULT_PAGE_SIZE) / DEFAULT_PAGE_SIZE, "alloc_kframe");
}

int dealloc_kframe(phys_addr_t start, uint64_t size) {
    mem_map * mmap = (mem_map *) &_kernel_end;
    return map_dealloc_frame(mmap, start, size);
}

/**
 * Allocate contiguous physical memory
 * @size is the total size of all 4KiB regions (should be aligned to 4KiB)
 */
phys_addr_t ialloc_kframe(uint64_t pages, PAGE_TYPE page_size, char * calling) {
    mem_map * mmap = (mem_map *) &_kernel_end;
    return map_alloc_frame_contiguous(mmap, pages , calling);
}

/**
 * returns bit index of first run of n free frames in w, or -1
 */
static inline int find_run_in_word(uint64_t word, unsigned run_length) {
    uint64_t free_mask = ~word;          // free = 1
    unsigned covered = 1;
    while (covered * 2 <= run_length) {
        free_mask &= free_mask >> covered;
        covered *= 2;
    }
    if (covered < run_length) free_mask &= free_mask >> (run_length - covered);
    return free_mask ? __builtin_ctzll(free_mask) : -1;
}


/**
 * allocate a single page of physical memory
 * @return physical addr or FRAME_ALLOC_FAILED
 */
phys_addr_t map_alloc_frame(mem_map * mmap, PAGE_TYPE page_size, char * calling) {
    uint8_t smallest_unit = PAGE_TYPE_TO_SIZE(page_size) / DEFAULT_PAGE_SIZE;           // how many blocsk need to be contiguous
    return map_alloc_frame_contiguous(mmap,smallest_unit, calling);
}

// 0x18000 / 0x1b3000
/**
 * allocates n 4KIB frames contiguously from a given mem_map
 * @return the physical addr or FRAME_ALLOC_FAILED
 */
phys_addr_t map_alloc_frame_contiguous(mem_map * mmap, uint64_t page_count, char * calling) {

    /*
     *  I know micro-optimizations aren't a priority but this was js such a good. idea..
     *  I've updated the contiguous frame generator to use trailing and leading zeros
     *  as well as an optimized (I tried my best) way to find holes of size n in uint64
     *
     *  That coupled w/ padding the bitmap to be 64 bit aligned
     *  so that I can use 64 bit ints to get better cache utilization (mb/hopefully) should be pretty good yk?
     */

    //Iprintf("allocating %i pages from (%s)\n", page_count, calling);

    for (uint32_t i = 0; i < mmap->region_count; i++) {
        mem_region * region = &mmap->regions[i];
        //Iprintf("WHYY: (%x, %x)\n", region->free_frames, page_count);
        if (region->free_frames < page_count) {
            continue;
        }

        uint32_t carried_run = 0;
        uint32_t run_start = 0;
        for (uint32_t qword_index = 0; qword_index < region->qword_count; ++qword_index) {
            uint64_t qword = *(uint64_t *) &mmap->bitmap.data[region->bitmap_start_addr / 8 + qword_index * 8];
            //Iprintf("wording: ind(%x); (%x)=(%x)\n", i, qword_index, qword);

            if (qword == 0) {                                  // fully free handle separately bc it's ub for trailing 0's
                //("zero'd out ig?");
                if (carried_run == 0)
                    run_start = qword_index * 64;
                carried_run += 64;
                if (carried_run >= page_count) {
                    phys_addr_t start = region->start_addr + run_start * DEFAULT_PAGE_SIZE;
                    //Iprintf("region[%i] frames = %x\n", i, region->free_frames);
                    //Iprintf("allocatedd1: (%x - %x)\n", start, start + page_count * DEFAULT_PAGE_SIZE);
                    set_mem_region(mmap, start, start + page_count * DEFAULT_PAGE_SIZE);
                    return start;
                }
                continue;
            }

            unsigned leading_free = __builtin_ctzll(qword);
            if (carried_run + leading_free >= page_count) {     // free frames at the start
                if (carried_run == 0)
                    run_start = qword_index * 64;
                phys_addr_t start = region->start_addr + run_start * DEFAULT_PAGE_SIZE;
                //Iprintf("region[%i] frames = %x\n", i, region->free_frames);
                //Iprintf("allocatedd2: (%x - %x)\n", start, start + page_count * DEFAULT_PAGE_SIZE);
                set_mem_region(mmap, start, start + page_count * DEFAULT_PAGE_SIZE);
                return start;
            }

            if (page_count <= 64) {                        // hole fully inside this word
                int bit_index = find_run_in_word(qword, page_count);
                //Iprintf("word: %x - %i)\n", qword, bit_index);
                if (bit_index >= 0) {
                    phys_addr_t start = region->start_addr + (qword_index * 64 + bit_index) * DEFAULT_PAGE_SIZE;
                    set_mem_region(mmap, start, start + page_count * DEFAULT_PAGE_SIZE);
                    //Iprintf("region[%i] frames = %x\n", i, region->free_frames);
                    //Iprintf("allocatedd3: (%x - %x)\n", start, start + page_count * DEFAULT_PAGE_SIZE);
                    return start;
                }
            }

            unsigned trailing_free = __builtin_clzll(qword);   // free frames at the top, carried on
            carried_run = trailing_free;
            run_start = qword_index * 64 + 64 - trailing_free;
        }
    }
    return FRAME_ALLOC_FAILED;
}


int map_dealloc_frame(mem_map *mmap, phys_addr_t phys_addr, uint64_t page_size) {
    // default page size = 4096 while the page size param could be a huge or super page
    uint64_t cont_units = page_size / DEFAULT_PAGE_SIZE;

    if (phys_addr < mmap->regions[0].start_addr || phys_addr + page_size > mmap->regions[mmap->region_count - 1].start_addr + mmap->regions[mmap->region_count - 1].len_bytes) {
        return -1;
    }

    for (uint32_t i = 0; i < mmap->region_count; i++) {
        mem_region *region = &mmap->regions[i];
        if (region->start_addr > phys_addr || phys_addr + page_size > region->start_addr + region->len_bytes )
            continue;

        region->free_frames += cont_units;
        uint64_t effective_addr = phys_addr - region->start_addr;
        bitmap_clear_len(&mmap->bitmap, region->bitmap_start_addr + effective_addr / DEFAULT_PAGE_SIZE , cont_units);
        break;
    }
    return 0;
}