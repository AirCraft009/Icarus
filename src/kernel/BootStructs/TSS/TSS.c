//
// Created by Mxsxll on 21.09.2026.
//

#include "TSS.h"

#include "../../Memory/PMM/mem_map.h"
#include "../../util/kernel_helper.h"


__attribute__((aligned(0x10)))
static tss_entry_t tss[26];
#define TSS_STACK_PAGES  4            // 16 KiB per stack
#define TSS_IOPB_OFFSET  0x0067       // == sizeof(TSS)-1 == no I/O bitmap

// TSS entry indices (each entry = 4 bytes, TSS = 26 * 4 = 104 bytes)
enum {
    TSS_RSP0 = 1,   // offset 0x04
    TSS_RSP1 = 3,   // offset 0x0C
    TSS_RSP2 = 5,   // offset 0x14
    TSS_IST1 = 9,   // offset 0x24
    TSS_IST2 = 11,
    TSS_IST3 = 13,
    TSS_IST4 = 15,
    TSS_IST5 = 17,
    TSS_IST6 = 19,
    TSS_IST7 = 21,  // offset 0x54
    TSS_IOPB = 25   // offset 0x64: reserved(2), IOPB offset(2)
};

/**
 *Write a 32-bit value into one entry (little-endian byte order)
**/
static inline void tss_set32(tss_entry_t *e, uint32_t v) {
    e->l_w1 = (uint8_t)(v);
    e->l_w2 = (uint8_t)(v >> 8);
    e->h_w1 = (uint8_t)(v >> 16);
    e->h_w2 = (uint8_t)(v >> 24);
}

/**
 * Write a 64-bit value across two consecutive entries
 */
static inline void tss_set64(uint32_t idx, uint64_t v) {
    tss_set32(&tss[idx],     (uint32_t)(v & 0xFFFFFFFF));
    tss_set32(&tss[idx + 1], (uint32_t)(v >> 32));
}

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

void init_tss() {
    // Clear everything (reserved fields included)
    for (int i = 0; i < 26; i++)
        tss_set32(&tss[i], 0);

    // RSP0: kernel stack used on ring3 for ring0 transitions
    uint64_t rsp0 = tss_alloc_stack();
    tss_set64(TSS_RSP0, rsp0);

    // RSP1 / RSP2 unused (left at 0) bc only ring 0 & 3 are used

    // IST1 IST7 are interrupt stacks (then I finally don't have to wrry about the #Pf handler using a corrupted stack tg)
    static const uint32_t ist_idx[7] = {
        TSS_IST1, TSS_IST2, TSS_IST3, TSS_IST4,
        TSS_IST5, TSS_IST6, TSS_IST7
    };
    for (int i = 0; i < 7; i++)
        tss_set64(ist_idx[i], tss_alloc_stack());

    // Entry 25: bytes 0x64-0x65 reserved, 0x66-0x67 = IOPB offset
    tss[TSS_IOPB].l_w1 = 0;
    tss[TSS_IOPB].l_w2 = 0;
    tss[TSS_IOPB].h_w1 = (uint8_t)(TSS_IOPB_OFFSET & 0xFF);   // 0x67
    tss[TSS_IOPB].h_w2 = (uint8_t)(TSS_IOPB_OFFSET >> 8);     // 0x00
}
