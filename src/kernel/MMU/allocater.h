//
// Created by cocon on 23.09.2026.
//

#ifndef ICARUS_ALLOCATER_H
#define ICARUS_ALLOCATER_H
#include <stdint.h>

#include "../util/BitMap.h"

void * alloc_frame(bit_map *mmap, uint64_t page_size);

#endif //ICARUS_ALLOCATER_H
