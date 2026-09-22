// gdt.h
#include <stdint.h>

#define GDT_ENTRIES 7

typedef struct __attribute__((packed)) {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_mid;
    uint8_t  access;
    uint8_t  granularity; // flags (high nibble) | limit_high (low nibble)
    uint8_t  base_high;
} gdt_entry_t;

typedef struct __attribute__((packed)) {
    uint32_t base_upper32; // bits 63:32 of base, for the 16-byte TSS descriptor
    uint32_t reserved;
} gdt_entry_hi_t;

typedef struct __attribute__((packed)) {
    uint16_t limit;
    uint64_t base;
} gdt_ptr_t;

// 64-bit TSS (structurally same as 32-bit TSS's IO/stack fields but widened;
// io-bitmap/legacy fields mostly unused, IST stacks are what you actually want)
typedef struct __attribute__((packed)) {
    uint32_t reserved0;
    uint64_t rsp0, rsp1, rsp2;
    uint64_t reserved1;
    uint64_t ist1, ist2, ist3, ist4, ist5, ist6, ist7;
    uint64_t reserved2;
    uint16_t reserved3;
    uint16_t iomap_base;
} tss_t;

void gdt_init(void);