//
// Created by cocon on 20.09.2026.
//

#ifndef ICARUS_MEMORY_MAPPING_H
#define ICARUS_MEMORY_MAPPING_H

#pragma once

#define PageS 4096
#define HugePS 2097152
#define SuperPs 1073741824

#define NPage 0
#define HPage 1
#define SPage 2

#include "../multiboot2.h"

int init_mmap(	struct multiboot_tag *mmap_tag);

typedef struct PageTable {
    uint64_t pml4[512];
    uint64_t pdpt[512];
    uint64_t pd[512];
    uint64_t pt[512];
}__attribute__((aligned(4096))) page_table_t;

#endif //ICARUS_MEMORY_MAPPING_H
