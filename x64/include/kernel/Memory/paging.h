//
// Created by cocon on 13.10.2026.
//

#ifndef ICARUS_PAGING_H
#define ICARUS_PAGING_H
#pragma once
#include <stdint.h>

#include "page_definitions.h"
#include "PMM/mem_map.h"


#define ENTRIES_PER_TABLE 512
#define PHYS_TO_VIRT(phys)  ((void *)(uintptr_t)(phys))
#define REGION_ALLOC(size)  Imalloc(size)
#define REGION_FREE(ptr)    free(ptr)

/**
 * A contiguous virtual range whose pages all share the same permissions.
 * perms uses the PT_* flag layout (PT_WRITEABLE, PT_USER_ACCESS,
 * PT_WRITE_THROUGH, PT_CACHE_DISABLED, PT_EXECUTION_DISABLED).
 */
typedef struct MappedRegion {
    uint64_t start;
    uint64_t len;
    uint64_t perms;
    struct MappedRegion *next;
} mapped_region_t;

typedef struct {
    mapped_region_t *head;
    mapped_region_t *tail;
} region_list_t;

#define PERMS_INITIAL (PT_WRITEABLE | PT_USER_ACCESS)

void paging_init();
int page_in(page_map_l4_entry *pml4, const void *virt_addr, const void *phys_addr, uint64_t pageSize, uint64_t permissions);
int page_out(page_map_l4_entry *pml4, uint64_t virt_addr);
phys_addr_t test_walk_table(page_map_l4_entry *pml4, const virt_addr_t virt_addr);
bool is_mapped(page_map_l4_entry * pml4, virt_addr_t virt_addr);


#endif //ICARUS_PAGING_H
