//
// Created by cocon on 22.09.2026.
//

#ifndef ICARUS_KERNEL_HELPER_H
#define ICARUS_KERNEL_HELPER_H
#pragma once


#include <stdint.h>

#define KERNEL_VMA 0xFFFFFFFF80000000UL
#define KERNEL_HEAP_ADDR 0xFFFFFF0000000000UL
#define KERNEL_MMIO 0xFFFFFE0000000000UL

#define INVALID_PHYS_ADDR UINT64_MAX
// page size for each bitmap entry
#define DEFAULT_PAGE_SIZE 4096

#define ALIGN_UP(addr, align)   (((addr) + (align) - 1) & ~((align) - 1))
#define ALIGN_DOWN(addr, align) ((addr) & ~((align) - 1))
#define KERNEL_PHYS_TO_VIRT(p) ((uintptr_t)(p) + KERNEL_VMA)
#define KERNEL_VIRT_TO_PHYS(p) ((uintptr_t)(p) - KERNEL_VMA)

#define MMIO_PHYS_TO_VIRT(p) ((uintptr_t)(p) + KERNEL_MMIO)
#define MMIO_VIRT_TO_PHYS(p) ((uintptr_t)(p) - KERNEL_MMIO)

#define PCI_INDEX_PORT 0xCF8
#define PCI_DATA_PORT 0xCFC

static __inline uint64_t read_cr3(void){
    uint64_t value;
    __asm__ volatile ("mov %%cr3, %0" : "=r"(value));
    return value;
}

static __inline void write_cr3(uint64_t value) {
    __asm__ volatile ("mov %0, %%cr3" : : "r"(value) : "memory");
}


static __inline void pci_set_index(uint32_t index){
    __asm__ volatile ("outl %0, %1"
                      :
                      : "a"(index), "Nd"(PCI_INDEX_PORT));
}

static __inline void pci_write_val(uint32_t value){
    __asm__ volatile ("outl %0, %1"
                      :
                      : "a"(value), "Nd"(PCI_DATA_PORT));
}

static __inline uint32_t pci_read_val(){
    uint32_t value;
    __asm__ volatile ("inl %1, %0"
                      :
                      : "a"(value), "Nd"(PCI_DATA_PORT));
    return value;
}

static __inline void invlpg(void *addr) {
    __asm__ volatile ("invlpg (%0)" : : "r"(addr) : "memory");
}


static __inline void cpuSetMSR(uint32_t msr, uint32_t lo, uint32_t hi) {
    __asm__ __volatile__(
        "wrmsr"
        :
        : "c"(msr), "a"(lo), "d"(hi)
    );
}

static __inline void cpuGetMSR(uint32_t msr, uint32_t *lo, uint32_t *hi) {
    __asm__ __volatile__(
        "rdmsr"
        : "=a"(*lo), "=d"(*hi)
        : "c"(msr)
    );
}
#endif //ICARUS_KERNEL_HELPER_H
