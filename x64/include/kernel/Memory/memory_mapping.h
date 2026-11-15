//
// Created by cocon on 20.09.2026.
//

#ifndef ICARUS_MEMORY_MAPPING_H
#define ICARUS_MEMORY_MAPPING_H
#pragma once
#include "page_definitions.h"
#include "kernel/util/multiboot2.h"

//TODO: move helpers to paging.h or page_definitions

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

// Helpers for setting the Page Permissions
#define PT_WRITEABLE_SHIFT 0x1
#define PT_USER_ACCESS_SHIFT 0x3
#define PT_WRITE_THROUGH_SHIFT 0x7
#define PT_CACHE_DISABLED_SHIFT 0xF
#define PT_EXECUTION_DISABLED_SHIFT 0x3F

#define PT_WRITEABLE (0x1 << PT_WRITEABLE_SHIFT)
#define PT_USER_ACCESS (0x1 << PT_USER_ACCESS_SHIFT)
#define PT_WRITE_THROUGH (0x1 << PT_WRITE_THROUGH_SHIFT)
#define PT_CACHE_DISABLED (0x1 << PT_CACHE_DISABLED_SHIFT)
#define PT_EXECUTION_DISABLED (0x1L << PT_EXECUTION_DISABLED_SHIFT)



#define PERM_BIT(p, shift)  (((p) >> (shift)) & 0x1)

// Intermediate entries: a 1 propagates up the tree, a 0 never clears anything.
// NX / PWT / PCD are deliberately NOT touched here.
#define APPLY_UPPER_PERMS(e, p)                                   \
    do {                                                          \
        if (PERM_BIT(p,PT_USER_ACCESS_SHIFT)) (e)->user_access = 1; \
        if (PERM_BIT(p, PT_WRITEABLE_SHIFT))   (e)->writeable   = 1; \
    } while (0)

// Leaf entry apply what the caller asked for.
#define APPLY_LEAF_PERMS(e, p)                                                    \
    do {                                                                          \
        (e)->user_access         = PERM_BIT(p,PT_USER_ACCESS_SHIFT);                \
        (e)->writeable           = PERM_BIT(p, PT_WRITEABLE_SHIFT);                  \
        (e)->write_through       = PERM_BIT(p, PT_WRITE_THROUGH_SHIFT);              \
        (e)->cache_disabled      = PERM_BIT(p, PT_CACHE_DISABLED_SHIFT);             \
        (e)->execution_disabled  = PERM_BIT(p, PT_EXECUTION_DISABLED_SHIFT);         \
    } while (0)

int handle_mb2_mmap(struct multiboot_tag *mmap_tag, page_map_l4_entry *pml4);


#endif //ICARUS_MEMORY_MAPPING_H
