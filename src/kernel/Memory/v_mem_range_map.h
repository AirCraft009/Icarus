//
// Created by cocon on 24.09.2026.
//

#ifndef ICARUS_V_MEM_RANGE_MAP_H
#define ICARUS_V_MEM_RANGE_MAP_H
#include <stdint.h>

// length in standard pages
typedef struct VMemRange {
    uint64_t start;
    uint64_t len;
}v_mem_range;

typedef struct RangeMap {
    uint64_t size;
    v_mem_range * free_ranges;
}range_map;

#endif //ICARUS_V_MEM_RANGE_MAP_H
