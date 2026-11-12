//
// Created by cocon on 13.10.2026.
//

#ifndef ICARUS_PAGING_H
#define ICARUS_PAGING_H
#pragma once
#include <stdint.h>

#include "page_definitions.h"

void paging_init();
int page_in(page_map_l4_entry *pml4, const void *virt_addr, const void *phys_addr, uint64_t pageSize, uint64_t permissions);
int page_out(page_map_l4_entry *pml4, uint64_t virt_addr);

#endif //ICARUS_PAGING_H
