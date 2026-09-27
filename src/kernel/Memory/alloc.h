//
// Created by cocon on 27.09.2026.
//

#ifndef ICARUS_ALLOC_H
#define ICARUS_ALLOC_H
#pragma once
#define MAX_ALLOCATION_SIZE
#include <stdint.h>

typedef struct AllocMetaData {
    uint64_t alloc_length;

}__attribute__((packed)) alloc_meta_data_t;

#endif //ICARUS_ALLOC_H
