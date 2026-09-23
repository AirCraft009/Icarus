// gdt.c
#include "gdt.h"
#include "../../lib/mem_utils.h"

static gdt_entry_t    gdt[GDT_ENTRIES];
static gdt_entry_hi_t gdt_hi;
static gdt_ptr_t      gdt_descriptor;
static tss_t          tss;

static void gdt_set_entry(int i, uint32_t base, uint32_t limit,
                           uint8_t access, uint8_t flags) {
    gdt[i].limit_low    = limit & 0xFFFF;
    gdt[i].base_low      = base & 0xFFFF;
    gdt[i].base_mid      = (base >> 16) & 0xFF;
    gdt[i].access        = access;
    gdt[i].granularity   = (flags & 0xF0) | ((limit >> 16) & 0x0F);
    gdt[i].base_high     = (base >> 24) & 0xFF;
}

static inline void gdt_flush(uint64_t gdt_ptr_addr) {
    __asm__ volatile (
        "lgdt (%0)                 \n"
        "mov $0x10, %%ax           \n" // kernel data selector
        "mov %%ax, %%ds            \n"
        "mov %%ax, %%es            \n"
        "mov %%ax, %%fs            \n"
        "mov %%ax, %%gs            \n"
        "mov %%ax, %%ss            \n"
        "pushq $0x08               \n" // kernel code selector
        "lea 1f(%%rip), %%rax      \n"
        "pushq %%rax               \n"
        "lretq                     \n" // far return reloads CS
        "1:                        \n"
        :
        : "r" (gdt_ptr_addr)
        : "rax", "memory"
    );
}

static inline void tss_flush(void) {
    __asm__ volatile (
        "mov $0x28, %%ax   \n" // TSS selector
        "ltr %%ax          \n"
        :
        :
        : "rax"
    );
}

void gdt_init(void) {
    gdt_set_entry(0, 0, 0, 0, 0);                    // null
    gdt_set_entry(1, 0, 0xFFFFF, 0x9A, 0xA0);         // kernel code
    gdt_set_entry(2, 0, 0xFFFFF, 0x92, 0xC0);         // kernel data
    gdt_set_entry(3, 0, 0xFFFFF, 0xF2, 0xC0);         // user data
    gdt_set_entry(4, 0, 0xFFFFF, 0xFA, 0xA0);         // user code

    Imemset(&tss, 0, sizeof(tss));
    tss.iomap_base = sizeof(tss);

    uint64_t tss_base  = (uint64_t)&tss;
    uint32_t tss_limit = sizeof(tss) - 1;
    gdt_set_entry(5, (uint32_t)(tss_base & 0xFFFFFFFF), tss_limit, 0x89, 0x00);
    gdt_hi.base_upper32 = (uint32_t)(tss_base >> 32);
    gdt_hi.reserved = 0;
    Imemcpy(&gdt[6], &gdt_hi, sizeof(gdt_hi));

    gdt_descriptor.limit = sizeof(gdt) - 1;
    gdt_descriptor.base  = (uint64_t)&gdt;

    gdt_flush((uint64_t)&gdt_descriptor);
    tss_flush();
}