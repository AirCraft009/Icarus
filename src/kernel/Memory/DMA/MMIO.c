//
// Created by cocon on 02.10.2026.
//

#include "kernel/Memory/DMA/MMIO.h"

#include "kernel_helper.h"
#include "../memory_mapping.h"
#include "kernel/Memory/PMM/mem_map.h"

virt_addr_t map_mmio(phys_addr_t phys_addr, uint64_t size, uint64_t flags) {
    page_map_l4_entry * pml4 = (page_map_l4_entry *) read_cr3();
    phys_addr_t start = ALIGN_DOWN(phys_addr, DEFAULT_PAGE_SIZE);
    phys_addr_t end = ALIGN_UP(phys_addr, DEFAULT_PAGE_SIZE);

    for (int i = 0; i < size; i += DEFAULT_PAGE_SIZE) {
         if (page_in(
            pml4,
            (void *) KERNEL_PHYS_TO_VIRT(start + i),
            (void *) (start + i),
            DEFAULT_PAGE_SIZE,
             flags) != 0)
        {
            return FRAME_ALLOC_FAILED;
        }
    }
    
    return KERNEL_PHYS_TO_VIRT(start);
}
