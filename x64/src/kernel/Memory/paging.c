//
// Created by cocon on 13.10.2026.
//

#include "kernel/Memory/paging.h"

#include "kernel_helper.h"
#include "kernel/Memory/memory_mapping.h"
#include <string.h>

#include "kernel/Memory/alloc.h"
#include "kernel/Memory/PMM/phys_alloc.h"
#include "kernel/util/shellio.h"

extern char _kernel_end[];
static mem_map *mmap;

/**
 * initializes state for paging to work correctly
 * call after handle_mb2_mmap
 */
void paging_init() {
    mmap = (mem_map *) &_kernel_end;
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
 * @param pageSize 4Kib, 2MiB, 1GiB
 * @param permissions all permission bits
 * @return 0 for no error -1 for error
 */
int page_in(page_map_l4_entry *pml4, const void *virt_addr, const void *phys_addr,
            uint64_t pageSize, uint64_t permissions) {

    page_map_l4_entry *pml4_entry = &pml4[VA_PML4_INDEX(virt_addr)];
    if (pml4_entry->present == 0) {
        phys_addr_t pdpt = ialloc_kframe(1, DEFAULT_PAGE_SIZE);
        if (pdpt == FRAME_ALLOC_FAILED) {
            Iprintf("ERROR PML4!\n");
            return -1;
        }
        memset((void *) KERNEL_PHYS_TO_VIRT(pdpt), 0, DEFAULT_PAGE_SIZE);
        pml4_entry->page_ppn = pdpt >> 12;
        pml4_entry->present = 1;
    }
    APPLY_UPPER_PERMS(pml4_entry, permissions);

    pdpt_entry_t *pdpt_table = (pdpt_entry_t *) KERNEL_PHYS_TO_VIRT(pml4_entry->page_ppn << 12);
    pdpt_entry_t *pdpt_entry = &pdpt_table[VA_PDPT_INDEX(virt_addr)];

    if (pageSize == SUPER_PS) {                 /* 1 GiB leaf */
        APPLY_LEAF_PERMS(pdpt_entry, permissions);
        pdpt_entry->huge     = 1;
        pdpt_entry->page_ppn = ((uint64_t) phys_addr) >> 12;
        pdpt_entry->present  = 1;
        invlpg(virt_addr);
        return 0;
    }

    if (pdpt_entry->present == 0) {
        phys_addr_t pd = ialloc_kframe(1, DEFAULT_PAGE_SIZE);
        if (pd == FRAME_ALLOC_FAILED) {
            Iprintf("ERROR PDPT!\n");
            return -1;
        }
        memset((void *) KERNEL_PHYS_TO_VIRT(pd), 0, DEFAULT_PAGE_SIZE);
        pdpt_entry->page_ppn = pd >> 12;
        pdpt_entry->present  = 1;
    }
    APPLY_UPPER_PERMS(pdpt_entry, permissions);

    pd_entry_t *pd_table = (pd_entry_t *) KERNEL_PHYS_TO_VIRT(pdpt_entry->page_ppn << 12);
    pd_entry_t *pd_entry = &pd_table[VA_PD_INDEX(virt_addr)];

    if (pageSize == HUGE_PS) {                  /* 2 MiB leaf */
        APPLY_LEAF_PERMS(pd_entry, permissions);
        pd_entry->huge     = 1;
        pd_entry->page_ppn = ((uint64_t) phys_addr) >> 12;
        pd_entry->present  = 1;
        invlpg(virt_addr);
        return 0;
    }

    if (pd_entry->present == 0) {
        phys_addr_t pt = ialloc_kframe(1, DEFAULT_PAGE_SIZE);
        if (pt == FRAME_ALLOC_FAILED) {
            Iprintf("ERROR PT!\n");
            return -1;
        }
        memset((void *) KERNEL_PHYS_TO_VIRT(pt), 0, DEFAULT_PAGE_SIZE);
        pd_entry->page_ppn = pt >> 12;
        pd_entry->present  = 1;
    }
    APPLY_UPPER_PERMS(pd_entry, permissions);

    PageTableEntry *pt_table = (PageTableEntry *) KERNEL_PHYS_TO_VIRT(pd_entry->page_ppn << 12);
    PageTableEntry *pt_entry = &pt_table[VA_PT_INDEX(virt_addr)];

    APPLY_LEAF_PERMS(pt_entry, permissions);
    pt_entry->accessed = 0;
    pt_entry->dirty    = 0;
    pt_entry->page_ppn = ((uint64_t) phys_addr) >> 12;
    pt_entry->present  = 1;
    invlpg(virt_addr);

    return 0;
}

phys_addr_t walk_table(page_map_l4_entry * pml4, virt_addr_t virt_addr) {
    page_map_l4_entry *pml4_entry = &pml4[VA_PML4_INDEX(virt_addr)];
    if (pml4_entry->present == 0) {
        return INVALID_PHYS_ADDR;
    }

    pdpt_entry_t *pdpt_entry = &((pdpt_entry_t *) KERNEL_PHYS_TO_VIRT(pml4_entry->page_ppn << 12))[VA_PDPT_INDEX(virt_addr)];
    if (pdpt_entry->present == 0)
        return INVALID_PHYS_ADDR;

    if (pdpt_entry->huge == 1)
        return pdpt_entry->page_ppn << 12;


    pd_entry_t *pd_entry = &((pd_entry_t *) KERNEL_PHYS_TO_VIRT(pdpt_entry->page_ppn << 12))[VA_PD_INDEX(virt_addr)];
    if (pd_entry->present == 0)
        return INVALID_PHYS_ADDR;

    if (pd_entry->huge == 1)
        return pd_entry->page_ppn << 12;


    PageTableEntry *pt_entry = &((PageTableEntry *) KERNEL_PHYS_TO_VIRT(pd_entry->page_ppn << 12))[VA_PT_INDEX(virt_addr)];
    if (pt_entry->present == 0)
        return INVALID_PHYS_ADDR;

    return pt_entry->page_ppn << 12;
}

bool is_mapped(page_map_l4_entry * pml4, virt_addr_t virt_addr) {
    return  walk_table(pml4, virt_addr) != INVALID_PHYS_ADDR;
}


phys_addr_t test_walk_table(page_map_l4_entry *pml4, const virt_addr_t virt_addr) {
    Iprintf("Walking addr (%x) on (%x)\n", virt_addr, pml4);
    page_map_l4_entry *pml4_entry = &pml4[VA_PML4_INDEX(virt_addr)];
    if (pml4_entry->present == 0) {
        Iprintf("PML4 table: PDPT NOT PRESENT AT(%i)\n",  VA_PML4_INDEX(virt_addr));
        return INVALID_PHYS_ADDR;
    }

    pdpt_entry_t *pdpt_entry = &((pdpt_entry_t *) KERNEL_PHYS_TO_VIRT(pml4_entry->page_ppn << 12))[VA_PDPT_INDEX(virt_addr)];
    if (pdpt_entry->present == 0) {
        Iprintf("PDPT table: PD NOT PRESENT AT(%i)\n",  VA_PDPT_INDEX(virt_addr));
        return INVALID_PHYS_ADDR;
    }

    pd_entry_t *pd_entry = &((pd_entry_t *) KERNEL_PHYS_TO_VIRT(pdpt_entry->page_ppn << 12))[VA_PD_INDEX(virt_addr)];
    if (pd_entry->huge == 1 && pd_entry->present == 1) {
        Iprintf("Ending walk at PD (%x)\n", pd_entry->page_ppn << 12);
        return pd_entry->page_ppn << 12;
    }
    Iprintf("PD: (%x) = (%x)\n", virt_addr, *pd_entry);
    if (pd_entry->present == 0) {
        Iprintf("PD table: PT NOT PRESENT AT (%i)\n",  VA_PD_INDEX(virt_addr));
        return INVALID_PHYS_ADDR;
    }

    PageTableEntry *pt_entry = &((PageTableEntry *) KERNEL_PHYS_TO_VIRT(pd_entry->page_ppn << 12))[VA_PT_INDEX(virt_addr)];
    if (pt_entry->present == 0) {
        Iprintf("PT: NO ACTUAL FRAME AT (%i)\n",  VA_PT_INDEX(virt_addr));
        return INVALID_PHYS_ADDR;
    }
    Iprintf("4KIB entry: %x\n", pt_entry->page_ppn << 12);
    return pt_entry->page_ppn << 12;
}

int page_out(page_map_l4_entry *pml4, uint64_t virt_addr) {
    page_map_l4_entry *pml4_entry = &pml4[VA_PML4_INDEX(virt_addr)];
    if (pml4_entry->present == 0) {
        return -1;
    }

    pdpt_entry_t *pdpt_entry = &((pdpt_entry_t *) KERNEL_PHYS_TO_VIRT(pml4_entry->page_ppn << 12))[VA_PDPT_INDEX(virt_addr)];

    if (pdpt_entry->huge == 1 && pdpt_entry->present == 0) {
        *(uint64_t *) pml4_entry = 0;
        *(uint64_t *) pdpt_entry = 0;
        invlpg((void *) virt_addr);
        return 0;
    }

    if (pdpt_entry->present == 0) {
        return -2;
    }

    pd_entry_t *pd_entry = &((pd_entry_t *) KERNEL_PHYS_TO_VIRT(pdpt_entry->page_ppn << 12))[VA_PD_INDEX(virt_addr)];
    if (pd_entry->huge == 1 && pd_entry->present == 0) {
        *(uint64_t *) pml4_entry = 0;
        *(uint64_t *) pdpt_entry = 0;
        *(uint64_t *) pd_entry = 0;
        invlpg((void *) virt_addr);
        return 0;
    }

    if (pd_entry->present == 0) {
        return -3;
    }

    PageTableEntry *pt_entry = &((PageTableEntry *) KERNEL_PHYS_TO_VIRT(pd_entry->page_ppn << 12))[VA_PT_INDEX(virt_addr)];
    *(uint64_t *) pml4_entry = 0;
    *(uint64_t *) pdpt_entry = 0;
    *(uint64_t *) pd_entry = 0;
    *(uint64_t *) pt_entry = 0;
    invlpg((void *) virt_addr);
    return 0;
}


/* Effective permissions along a walk:
 * writeable and user_access are only set if EVERY level allows them (AND),
 * execution_disabled is set if ANY level disables execution (OR). */
static uint64_t perms_combine(uint64_t inherited,
                              uint64_t writeable,
                              uint64_t user_access,
                              uint64_t execution_disabled)
{
    uint64_t result = inherited;
    if (!writeable)        result &= ~(uint64_t)PT_WRITEABLE;
    if (!user_access)      result &= ~(uint64_t)PT_USER_ACCESS;
    if (execution_disabled) result |= (uint64_t)PT_EXECUTION_DISABLED;
    return result;
}

/* Cache attributes only come from the leaf entry. */
static uint64_t perms_apply_cache(uint64_t perms, uint64_t write_through, uint64_t cache_disabled){
    if (write_through) perms |= (uint64_t)PT_WRITE_THROUGH;
    if (cache_disabled) perms |= (uint64_t)PT_CACHE_DISABLED;
    return perms;
}

/* Appends a mapped range, merging it into the last region if it is adjacent
 * in virtual memory and has identical permissions. Returns 0 on success. */
static int region_list_add(region_list_t *list, uint64_t start, uint64_t len, uint64_t perms){
    mapped_region_t *tail = list->tail;

    if (tail != NULL && tail->start + tail->len == start && tail->perms == perms) {
        tail->len += len;
        return 0;
    }

    mapped_region_t *node = (mapped_region_t *)REGION_ALLOC(sizeof(mapped_region_t));
    if (node == NULL) {
        return -1;
    }
    node->start = start;
    node->len   = len;
    node->perms = perms;
    node->next  = NULL;

    if (tail != NULL) {
        tail->next = node;
    } else {
        list->head = node;
    }
    list->tail = node;
    return 0;
}

void page_regions_free(mapped_region_t *head){
    while (head != NULL) {
        mapped_region_t *next = head->next;
        REGION_FREE(head);
        head = next;
    }
}

static int walk_page_table(const PageTableEntry *page_table, uint64_t base,
                           uint64_t inherited, region_list_t *list)
{
    for (uint64_t index = 0; index < ENTRIES_PER_TABLE; index++) {
        const PageTableEntry *entry = &page_table[index];
        if (!entry->present) continue;

        uint64_t perms = perms_combine(inherited, entry->writeable, entry->user_access,
                                       entry->execution_disabled);
        perms = perms_apply_cache(perms, entry->write_through, entry->cache_disabled);

        if (region_list_add(list, base | (index << 12), PAGE_S, perms) != 0) return -1;
    }
    return 0;
}

static int walk_page_directory(const pd_entry_t *page_directory, uint64_t base,
                               uint64_t inherited, region_list_t *list)
{
    for (uint64_t index = 0; index < ENTRIES_PER_TABLE; index++) {
        const pd_entry_t *entry = &page_directory[index];
        if (!entry->present) continue;

        uint64_t perms = perms_combine(inherited, entry->writeable, entry->user_access,
                                       entry->execution_disabled);
        uint64_t entry_base = base | (index << 21);

        if (entry->huge) {
            perms = perms_apply_cache(perms, entry->write_through, entry->cache_disabled);
            if (region_list_add(list, entry_base, HUGE_PS, perms) != 0) return -1;
        } else {
            const PageTableEntry *page_table =
                (const PageTableEntry *)PHYS_TO_VIRT((uint64_t)entry->page_ppn << 12);
            if (walk_page_table(page_table, entry_base, perms, list) != 0) return -1;
        }
    }
    return 0;
}

static int walk_pdpt(const pdpt_entry_t *pdpt, uint64_t base,
                     uint64_t inherited, region_list_t *list)
{
    for (uint64_t index = 0; index < ENTRIES_PER_TABLE; index++) {
        const pdpt_entry_t *entry = &pdpt[index];
        if (!entry->present) continue;

        uint64_t perms = perms_combine(inherited, entry->writeable, entry->user_access,
                                       entry->execution_disabled);
        uint64_t entry_base = base | (index << 30);

        if (entry->huge) {
            perms = perms_apply_cache(perms, entry->write_through, entry->cache_disabled);
            if (region_list_add(list, entry_base, SUPER_PS, perms) != 0) return -1;
        } else {
            const pd_entry_t *page_directory =
                (const pd_entry_t *)PHYS_TO_VIRT((uint64_t)entry->page_ppn << 12);
            if (walk_page_directory(page_directory, entry_base, perms, list) != 0) return -1;
        }
    }
    return 0;
}

/**
 * Walks the whole page table and returns a linked list of mapped regions in
 * ascending virtual address order (lower half first, then the upper half).
 * Returns NULL if nothing is mapped or if allocation failed.
 * Release the result with page_regions_free().
 */
mapped_region_t *page_table_get_regions(const page_map_l4_entry *table){
    region_list_t list = { NULL, NULL };

    for (uint64_t index = 0; index < ENTRIES_PER_TABLE; index++) {
        const page_map_l4_entry *entry = &table[index];
        if (!entry->present) continue;

        uint64_t base = index << 39;
        if (index >= 256) {
            base |= 0xFFFF000000000000ULL;   /* sign-extend into canonical form */
        }

        uint64_t perms = perms_combine(PERMS_INITIAL, entry->writeable, entry->user_access,
                                       entry->execution_disabled);

        const pdpt_entry_t *pdpt =
            (const pdpt_entry_t *)PHYS_TO_VIRT((uint64_t)entry->page_ppn << 12);

        if (walk_pdpt(pdpt, base, perms, &list) != 0) {
            page_regions_free(list.head);
            return NULL;
        }
    }
    return list.head;
}




