//
// Created by cocon on 20.09.2026.
//

#include "memory_mapping.h"

#include <stdint.h>

#include "mem_map.h"
#include "../util/kernel_info.h"
#include "../util/kernel_helper.h"
#include "page_definitions.h"
#include "../../lib/mem_utils.h"

/*
 *  handles paging in and out
*/


extern char _kernel_end[];
extern char _kernel_end_phys[];
extern char _kernel_start[];

//static const page_map_l4_entry KERNEL_PML4 [512];
static uint64_t MemmapStart;
//TODO replace bitmap w/ more efficient data struct that saves last data access etc.
static mem_map *mmap;


/**
 * initiate the kernel physical memory map located after the kernel end
 * reads the multiboot2 mmap info tag and build a bitmap w/ 4KiB blocks
 *
 * @param mmap_tag multiboot2 mmap_tag info
 * @return the size of the mmap (block count)
 */
uint64_t init_mmap(struct multiboot_tag *mmap_tag) {
    //phys addr of method 0x107812
    MemmapStart = (uint64_t) &_kernel_end;
    cons_mprintf("INIT_MMAP: %x == %x -> %x\n", MemmapStart, &_kernel_end, &_kernel_end_phys);

    mmap = (mem_map *) MemmapStart;

    struct multiboot_mmap_entry *mmap_entry;

    for (mmap_entry = ((struct multiboot_tag_mmap *) mmap_tag)->entries;
        (multiboot_uint8_t *) mmap_entry < (multiboot_uint8_t *) mmap_tag + mmap_tag->size;
        mmap_entry = (multiboot_memory_map_t *) ((unsigned long) mmap_entry +
        ((struct multiboot_tag_mmap *) mmap_tag)->entry_size)) {


        if (mmap_entry->addr + mmap_entry->len > mmap->mem_size) {
            mmap->mem_size = mmap_entry->addr + mmap_entry->len;
        }

        uint64_t len;

        if (mmap_entry->type != MULTIBOOT_MEMORY_AVAILABLE) {
            // reserved: round outward so we never under-cover it
            // uint64_t start = ALIGN_DOWN(mmap_entry->addr, DEFAULT_PAGE_SIZE);
            // uint64_t end = ALIGN_UP(mmap_entry->addr + mmap_entry->len, DEFAULT_PAGE_SIZE);
            // index = start / DEFAULT_PAGE_SIZE;
            // bits = (end - start) / DEFAULT_PAGE_SIZE;
            // cons_mprintf("addrG(%i): %l len: %l\n", mmap_entry->type, index / 8, bits / 8);

        } else {
            // available: round inward so we never over-claim a partial page
            uint64_t start = ALIGN_UP(mmap_entry->addr, DEFAULT_PAGE_SIZE);
            uint64_t end = ALIGN_DOWN(mmap_entry->addr + mmap_entry->len, DEFAULT_PAGE_SIZE);
            if (end > start) {
                len = end - start;
                //cons_mprintf("addrF: %x - %x = len: %l\n", start, end, len);
                memmap_register_region(mmap, start, end);
            }
        }
    }

    //mark the bitmap itself as not available
    //bitmap_set_len(mmap, KERNEL_VIRT_TO_PHYS(MemmapStart), mmap->size / DEFAULT_PAGE_SIZE);
    show_mem_map(mmap, false);
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
    if (pml4_entry->present == 0) {
        // no pdpt entry in the region (make a new one)
        void * pdpt =  alloc_frame(mmap, DEFAULT_PAGE_SIZE);
        if (pdpt == NULL) {
            return -1;
        }
        Imemset((void *) KERNEL_PHYS_TO_VIRT(pdpt), 0, DEFAULT_PAGE_SIZE);
        pml4_entry->present = 1;
        pml4_entry->page_ppn = (uint64_t) pdpt;
        cons_mprintf("PDPT table: %x\n", pdpt);
    }

    pdpt_entry_t *pdpt_table = (pdpt_entry_t *) pml4->page_ppn;
    pdpt_entry_t *pdpt_entry = &pdpt_table[VA_PDPT_INDEX(virt_addr)];

    if (!pdpt_entry->present) {
        // no pd entry in the region (make a new one)
        void * pd =  alloc_frame(mmap, DEFAULT_PAGE_SIZE);
        if (pd == NULL) {
            return -1;
        }
        Imemset((void *) KERNEL_PHYS_TO_VIRT(pd), 0, DEFAULT_PAGE_SIZE);
        pdpt_entry->present = 1;
        pdpt_entry->page_ppn = (uint64_t) pd;
        cons_mprintf("PD table: %x\n", pd);
    }

    pd_entry_t *pd_table = (pd_entry_t *) pml4->page_ppn;
    pd_entry_t *pd_entry = &pd_table[VA_PD_INDEX(virt_addr)];

    if (pageSize == HUGE_PS) {
        pd_entry->huge = 1;
        pd_entry->page_ppn = (uint64_t) phys_addr;
        pd_entry->writeable = 1;
        cons_mprintf("PD physical: %x\n", phys_addr);
        return 0;
    }

    if (!pd_entry->present) {
        // no pt entry in the region (make a new one)
        void * pt =  alloc_frame(mmap, DEFAULT_PAGE_SIZE);
        if (pt == NULL) {
            return -1;
        }
        pd_entry->present = 1;
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
