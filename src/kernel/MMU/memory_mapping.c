//
// Created by cocon on 20.09.2026.
//

#include "memory_mapping.h"

#include <stdint.h>

#include "allocater.h"
#include "../kernel_info.h"
#include "../kernel_helper.h"
#include "page_definitions.h"
#include "../../lib/mem_utils.h"

/*
 *  handles paging in and out
*/


extern char _kernel_end[];
extern char _kernel_end_phys[];
extern char _kernel_start[];

//static const page_map_l4_entry KERNEL_PML4 [512];
static uint64_t BitmapStart;
//TODO replace bitmap w/ more efficient data struct that saves last data access etc.
static mem_map * mmap;


/**
 * initiate the kernel physical memory map located after the kernel end
 * reads the multiboot2 mmap info tag and build a bitmap w/ 4KiB blocks
 *
 * @param mmap_tag multiboot2 mmap_tag info
 * @return the size of the mmap (block count)
 */
uint64_t init_mmap(struct multiboot_tag *mmap_tag) {
    cons_mprintf("INIT_MMAP: %x - %x -> %x\n", &_kernel_start, &_kernel_end, &_kernel_end_phys);

    BitmapStart = (uint64_t) &_kernel_end;
    mmap = (mem_map *) BitmapStart;
    mmap->mem_size = 0;
    mmap->size = 0;

    struct multiboot_mmap_entry *mmap_entry;

    for (mmap_entry = ((struct multiboot_tag_mmap *) mmap_tag)->entries;
        (multiboot_uint8_t *) mmap_entry < (multiboot_uint8_t *) mmap_tag + mmap_tag->size;
        mmap_entry = (multiboot_memory_map_t *) ((unsigned long) mmap_entry +
        ((struct multiboot_tag_mmap *) mmap_tag)->entry_size)) {

        // if (mmap_entry->addr < (uint64_t) &_kernel_start) {
        //     // ignore the lower space
        //     mmap->offset = mmap_entry->addr;
        //     continue;
        // }

        if (mmap_entry->addr + mmap_entry->len > mmap->mem_size) {
            mmap->mem_size = mmap_entry->addr + mmap_entry->len;
        }

        uint64_t index, bits;

        if (mmap_entry->type != MULTIBOOT_MEMORY_AVAILABLE) {
            // reserved: round outward so we never under-cover it
            uint64_t start = ALIGN_DOWN(mmap_entry->addr, DEFAULT_PAGE_SIZE);
            uint64_t end = ALIGN_UP(mmap_entry->addr + mmap_entry->len, DEFAULT_PAGE_SIZE);
            index = start / DEFAULT_PAGE_SIZE;
            bits = (end - start) / DEFAULT_PAGE_SIZE;
            cons_mprintf("addrG(%i): %x len: %x\n", mmap_entry->type, mmap_entry->addr, mmap_entry->len);
            mmap->size += bits;
            memmap_set_len(mmap, index, bits);
        } else {
            // available: round inward so we never over-claim a partial page
            uint64_t start = ALIGN_UP(mmap_entry->addr, DEFAULT_PAGE_SIZE);
            uint64_t end = ALIGN_DOWN(mmap_entry->addr + mmap_entry->len, DEFAULT_PAGE_SIZE);
            if (end > start) {
                index = start / DEFAULT_PAGE_SIZE;
                bits = (end - start) / DEFAULT_PAGE_SIZE;
                mmap->size += bits;
                cons_mprintf("addrF: %x len: %x\n", mmap_entry->addr, mmap_entry->len);
                memmap_clear_len(mmap, index, bits);
                show_mmap(mmap);
            }
        }
    }

    cons_mprintf("Size: %x\n", mmap->size / 8);
    //mark the bitmap itself as not available
    //memmap_set_len(mmap, KERNEL_VIRT_TO_PHYS(BitmapStart), mmap->size / DEFAULT_PAGE_SIZE);
    //
    //show_mmap(mmap);
    return mmap->mem_size;
}

/**
 *
 * Page a phys to a virt addr on a pml4 table.
 * walks the table and if tables are missing
 * new ones are allocated as physical 4KiB frames
 *
 * @param pml4 ptr to the 0'th entry of the pml4 table
 * @param virt_addr a virtual address pointing to a page (the offset bits are ignored)
 * @param phys_addr a physical address pointing to a frame (the offset bits are ignored)
 * @param pageSize 4Kib or 2MiB
 * @return 0 for no error -1 for error
 */
int page_in(page_map_l4_entry *pml4, const void *virt_addr, const void *phys_addr, uint64_t pageSize) {
    page_map_l4_entry *pml4_entry = &pml4[VA_PML4_INDEX(virt_addr)];
    if (!pml4_entry->present) {
        // no pdpt entry in the region (make a new one)
        void * pdpt =  alloc_frame(mmap, DEFAULT_PAGE_SIZE);
        if (pdpt == NULL) {
            return -1;
        }
        cons_mprintf("PDPT ptr: %x\n", pdpt);
        Imemset((void *) KERNEL_PHYS_TO_VIRT(pdpt), 0, DEFAULT_PAGE_SIZE);
        pml4->page_ppn = (uint64_t) pdpt;
    }

    pdpt_entry_t *pdpt_table = (pdpt_entry_t *) pml4->page_ppn;
    pdpt_entry_t *pdpt_entry = &pdpt_table[VA_PDPT_INDEX(virt_addr)];

    if (!pdpt_entry->present) {
        pdpt_entry->present = 1;
        // no pd entry in the region (make a new one)
        void * pd =  alloc_frame(mmap, DEFAULT_PAGE_SIZE);
        if (pd == NULL) {
            return -1;
        }
        Imemset((void *) KERNEL_PHYS_TO_VIRT(pd), 0, DEFAULT_PAGE_SIZE);
        pdpt_entry->page_ppn = (uint64_t) pd;
    }

    pd_entry_t *pd_table = (pd_entry_t *) pml4->page_ppn;
    pd_entry_t *pd_entry = &pd_table[VA_PD_INDEX(virt_addr)];

    if (pageSize == HUGE_PS) {
        pd_entry->huge = 1;
        pd_entry->page_ppn = (uint64_t) phys_addr;
        cons_mprintf("PD ptr: %x\n", phys_addr);
        return 0;
    }

    if (!pd_entry->present) {
        pd_entry->present = 1;
        // no pt entry in the region (make a new one)
        void * pt =  alloc_frame(mmap, DEFAULT_PAGE_SIZE);
        if (pt == NULL) {
            return -1;
        }
        Imemset((void *) KERNEL_PHYS_TO_VIRT(pt), 0, DEFAULT_PAGE_SIZE);
        pd_entry->page_ppn = (uint64_t) pt;
    }

    PageTableEntry *pt_table = (PageTableEntry *) pd_entry->page_ppn;
    PageTableEntry *pt_entry = &pt_table[VA_PT_INDEX(virt_addr)];
    pt_entry->present = 1;
    pt_entry->page_ppn = (uint64_t) phys_addr;
    pt_entry->accessed = 0;
    pt_entry->dirty = 0;
    pt_entry->cache_disabled = 1; // TODO: Enable later after more testings

    return 0;
}

int page_out(page_map_l4_entry *pml4, uint64_t virt_addr) {

}
