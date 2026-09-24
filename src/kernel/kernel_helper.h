//
// Created by cocon on 22.09.2026.
//

#ifndef ICARUS_KERNEL_HELPER_H
#define ICARUS_KERNEL_HELPER_H
#include "kernel_info.h"


#define ALIGN_UP(addr, align)   (((addr) + (align) - 1) & ~((align) - 1))
#define ALIGN_DOWN(addr, align) ((addr) & ~((align) - 1))
#define KERNEL_PHYS_TO_VIRT(p) ((uintptr_t)(p) + KERNEL_VMA)

static inline uint64_t read_cr3(void)
{
    uint64_t value;
    __asm__ volatile ("mov %%cr3, %0" : "=r"(value));
    return value;
}
#endif //ICARUS_KERNEL_HELPER_H
