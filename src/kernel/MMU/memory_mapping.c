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
extern char _kernel_start_phys[];

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
    cons_mprintf("INIT_MMAP: %x == %x -> %x\n", _kernel_start_phys, &_kernel_end, &_kernel_end_phys);

    mmap = (mem_map *) MemmapStart;
    mmap->region_count = 0;
    mmap->mem_size = 0;

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
                cons_mprintf("addrF: %x - %x = len: %l\n", start, end, len);
                memmap_register_region(mmap, start, end);
            }
        }
    }

    //cons_mprintf("zeroing mmap\n");
    zero_mem_map(mmap);

   // cons_mprintf("setting kernel and bitmap space\n");
    //mark the kernel and bitmap itself as not available
    int err = set_mem_region(mmap,
        ALIGN_DOWN((uint64_t) &_kernel_start_phys, DEFAULT_PAGE_SIZE),
        KERNEL_VIRT_TO_PHYS(ALIGN_UP((uint64_t) mmap->bitmap.data + mmap->bitmap.size, DEFAULT_PAGE_SIZE ))
    );

    if (err != 0) {
        cons_mprintf("ERROR: set_mem_region returned %d\n", err);
    }
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
        phys_addr_t pdpt = alloc_frame(mmap, DEFAULT_PAGE_SIZE);
        if (pdpt == FRAME_ALLOC_FAILED) {
            cons_mprintf("ERROR PML4!\n");
            return -1;
        }
        Imemset((void *) KERNEL_PHYS_TO_VIRT(pdpt), 0, DEFAULT_PAGE_SIZE);
        pml4_entry->present = 1;
        pml4_entry->page_ppn = pdpt >> 12;
        cons_mprintf("PDPT table: %x\n", pdpt);
    }

    pdpt_entry_t *pdpt_table = (pdpt_entry_t *) KERNEL_PHYS_TO_VIRT(pml4_entry->page_ppn << 12);
    pdpt_entry_t *pdpt_entry = &pdpt_table[VA_PDPT_INDEX(virt_addr)];

    if (pdpt_entry->present == 0) {
        phys_addr_t pd = alloc_frame(mmap, DEFAULT_PAGE_SIZE);
        if (pd == FRAME_ALLOC_FAILED) {
            cons_mprintf("ERROR PDPT !\n");
            return -1;
        }
        Imemset((void *) KERNEL_PHYS_TO_VIRT(pd), 0, DEFAULT_PAGE_SIZE);
        pdpt_entry->present = 1;
        pdpt_entry->page_ppn = pd >> 12;
        cons_mprintf("PD table: %x\n", pd);
    }

    pd_entry_t *pd_table = (pd_entry_t *) KERNEL_PHYS_TO_VIRT(pdpt_entry->page_ppn << 12);
    pd_entry_t *pd_entry = &pd_table[VA_PD_INDEX(virt_addr)];

    if (pageSize == HUGE_PS) {
        pd_entry->huge = 1;
        pd_entry->present = 1;
        pd_entry->page_ppn = ((uint64_t) phys_addr) >> 12;
        pd_entry->writeable = 1;
        //cons_mprintf("PD physical: %x maps to %x\n", phys_addr, virt_addr);
        return 0;
    }

    if (pd_entry->present == 0) {
        phys_addr_t pt = alloc_frame(mmap, DEFAULT_PAGE_SIZE);
        if (pt == FRAME_ALLOC_FAILED) {
            cons_mprintf("ERROR PD!\n");
            return -1;
        }
        pd_entry->present = 1;
        Imemset((void *) KERNEL_PHYS_TO_VIRT(pt), 0, DEFAULT_PAGE_SIZE);
        pd_entry->page_ppn = pt >> 12;
    }

    PageTableEntry *pt_table = (PageTableEntry *) KERNEL_PHYS_TO_VIRT(pd_entry->page_ppn << 12);
    PageTableEntry *pt_entry = &pt_table[VA_PT_INDEX(virt_addr)];
    //cons_mprintf("4KIB entry: %x", phys_addr);
    pt_entry->present = 1;
    pt_entry->writeable = 1;
    pt_entry->page_ppn = ((uint64_t) phys_addr) >> 12;
    pt_entry->accessed = 0;
    pt_entry->dirty = 0;
    pt_entry->cache_disabled = 1;

    return 0;
}

phys_addr_t test_walk_table(page_map_l4_entry *pml4, const phys_addr_t *virt_addr) {
    cons_mprintf("Walking addr (%x)\n", virt_addr);
    page_map_l4_entry *pml4_entry = &pml4[VA_PML4_INDEX(virt_addr)];
    if (pml4_entry->present == 0) {
        cons_mprintf("PML4 table: PDPT NOT PRESENT AT(%i)\n",  VA_PML4_INDEX(virt_addr));
        return INVALID_PHYS_ADDR;
    }

    pdpt_entry_t *pdpt_entry = &((pdpt_entry_t *) KERNEL_PHYS_TO_VIRT(pml4_entry->page_ppn << 12))[VA_PDPT_INDEX(virt_addr)];
    if (pdpt_entry->present == 0) {
        cons_mprintf("PDPT table: PD NOT PRESENT AT(%i)\n",  VA_PDPT_INDEX(virt_addr));
        return INVALID_PHYS_ADDR;
    }

    pd_entry_t *pd_entry = &((pd_entry_t *) KERNEL_PHYS_TO_VIRT(pdpt_entry->page_ppn << 12))[VA_PD_INDEX(virt_addr)];
    if (pd_entry->huge == 1) {
        cons_mprintf("Ending walk at PD (%x)\n", pd_entry->page_ppn << 12);
        return pd_entry->page_ppn << 12;
    }
    if (pd_entry->present == 0) {
        cons_mprintf("PD table: PT NOT PRESENT AT (%i)\n",  VA_PD_INDEX(virt_addr));
        return INVALID_PHYS_ADDR;
    }

    PageTableEntry *pt_entry = &((PageTableEntry *) KERNEL_PHYS_TO_VIRT(pd_entry->page_ppn << 12))[VA_PT_INDEX(virt_addr)];
    cons_mprintf("4KIB entry: %x\n", pt_entry->page_ppn << 12);
    return pt_entry->page_ppn << 12;
}

int page_out(page_map_l4_entry *pml4, uint64_t virt_addr) {

}