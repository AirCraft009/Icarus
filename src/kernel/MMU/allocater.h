//
// Created by cocon on 23.09.2026.
//

#ifndef ICARUS_ALLOCATER_H
#define ICARUS_ALLOCATER_H
#include <stdint.h>

#include "memmap.h"

void * alloc_frame(mem_map *mmap, uint64_t page_size);

#endif //ICARUS_ALLOCATER_H
