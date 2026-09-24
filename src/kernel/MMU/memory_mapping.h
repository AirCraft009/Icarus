//
// Created by cocon on 20.09.2026.
//

#ifndef ICARUS_MEMORY_MAPPING_H
#define ICARUS_MEMORY_MAPPING_H

#pragma once

#define PAGE_S 4096
#define HUGE_PS 2097152
#define SUPER_PS 1073741824
#define VA_INDEX_MASK        0x1FFULL   /* 9 bits per level */

#define VA_PML4_INDEX(va)    ((((uint64_t)(va)) >> 39) & VA_INDEX_MASK)
#define VA_PDPT_INDEX(va)    ((((uint64_t)(va)) >> 30) & VA_INDEX_MASK)
#define VA_PD_INDEX(va)      ((((uint64_t)(va)) >> 21) & VA_INDEX_MASK)
#define VA_PT_INDEX(va)      ((((uint64_t)(va)) >> 12) & VA_INDEX_MASK)

// Offsets within the final page, depending on where the walk ended
#define VA_OFFSET_4K(va)     (((uint64_t)(va)) & 0xFFFULL)       /* bits 11:0  */
#define VA_OFFSET_2M(va)     (((uint64_t)(va)) & 0x1FFFFFULL)    /* bits 20:0  */
#define VA_OFFSET_1G(va)     (((uint64_t)(va)) & 0x3FFFFFFFULL)  /* bits 29:0  */

// canonical check for 48-bit VAs (bits 63:47 must all match)
#define VA_IS_CANONICAL_48(va) \
((((int64_t)(va) << 16) >> 16) == (int64_t)(va))


#define LOWEST_FRAME_SIZE 0x100000;
#include "page_definitions.h"
#include "../multiboot2.h"

uint64_t init_mmap(	struct multiboot_tag *mmap_tag);
int page_in(page_map_l4_entry *pml4, const void *virt_addr, const void *phys_addr, uint64_t pageSize);

#endif //ICARUS_MEMORY_MAPPING_H
