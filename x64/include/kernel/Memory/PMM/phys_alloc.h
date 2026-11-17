//
// Created by cocon on 17.11.2026.
//

#ifndef ICARUS_PHYS_ALLOC_H
#define ICARUS_PHYS_ALLOC_H
#pragma once
#include "mem_map.h"
#include "kernel/Memory/PMM/phys_alloc.h"

int idealloc_frames(struct_page_t *info_page);
phys_addr_t ialloc_kframe(uint64_t pages, PAGE_TYPE page_size);
struct_page_t *ialloc_frames(uint64_t count, PAGE_TYPE page_size);
void show_mem(bool verbose);
int reserve_mem(phys_addr_t phys_addr, uint64_t size);

#endif //ICARUS_PHYS_ALLOC_H
