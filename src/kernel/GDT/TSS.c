//
// Created by Mxsxll on 21.09.2026.
//

#include "../GDT/Tss.h"

#include "../../lib/mem_utils.h"
#include "../Memory/PMM/mem_map.h"
#include "../util/kernel_helper.h"

#define TSS_STACK_PAGES  4            // 16 KiB per stack
#define TSS_IOPB_OFFSET  0x0067       // == sizeof(TSS)-1 == no I/O bitmap

/**
 * Allocate a stack and return its top (stacks grow down)
**/
static uint64_t tss_alloc_stack(void) {
    phys_addr_t phys = ialloc_kframe(TSS_STACK_PAGES, DEFAULT_PAGE_SIZE);
    if (phys == FRAME_ALLOC_FAILED) {
        return FRAME_ALLOC_FAILED;
    }

    uint64_t base = (uint64_t)KERNEL_PHYS_TO_VIRT(phys);
    uint64_t top  = base + TSS_STACK_PAGES * DEFAULT_PAGE_SIZE;
    return top & ~0xFULL;   // 16-byte aligned
}

void init_tss(tss_t * tss) {
    // Clear everything (reserved fields included)
    Imemset(tss, 0, sizeof(*tss));

    // RSP0: kernel stack used on ring3 for ring0 transitions
    uint64_t rsp0 = tss_alloc_stack();
    tss->rsp0 = rsp0;

    // RSP1 / RSP2 unused (left at 0) bc only ring 0 & 3 are used

    // IST1 IST7 are interrupt stacks (then I finally don't have to wrry about the #Pf handler using a corrupted stack tg)
    tss->ist1 = tss_alloc_stack();
    tss->ist2 = tss_alloc_stack();
    tss->ist3 = tss_alloc_stack();
    tss->ist4 = tss_alloc_stack();
    tss->ist5 = tss_alloc_stack();
    tss->ist6 = tss_alloc_stack();
    tss->ist7 = tss_alloc_stack();

    // Entry 25: bytes 0x64-0x65 reserved, 0x66-0x67 = IOPB offset
    tss->iomap_base = sizeof(tss);
}
