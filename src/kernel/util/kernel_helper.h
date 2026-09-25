//
// Created by cocon on 22.09.2026.
//

#ifndef ICARUS_KERNEL_HELPER_H
#define ICARUS_KERNEL_HELPER_H
#include "kernel_info.h"
#include "stdint.h"


#define ALIGN_UP(addr, align)   (((addr) + (align) - 1) & ~((align) - 1))
#define ALIGN_DOWN(addr, align) ((addr) & ~((align) - 1))
#define KERNEL_PHYS_TO_VIRT(p) ((uintptr_t)(p) + KERNEL_VMA)
#define KERNEL_VIRT_TO_PHYS(p) ((uintptr_t)(p) - KERNEL_VMA)

static inline uint64_t read_cr3(void)
{
    uint64_t value;
    __asm__ volatile ("mov %%cr3, %0" : "=r"(value));
    return value;
}

static inline void write_cr3(uint64_t value) {
    __asm__ volatile ("mov %0, %%cr3" : : "r"(value) : "memory");
}

static inline void invlpg(void *addr) {
    __asm__ volatile ("invlpg (%0)" : : "r"(addr) : "memory");
}
#endif //ICARUS_KERNEL_HELPER_H
