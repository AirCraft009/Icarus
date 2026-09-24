//
// Created by Mxsxll on 22.09.2026.
//

#include "memmap.h"

#include <stddef.h>

#include "../kernel_info.h"
#include "../../shell/shellio.h"

/**
 * sets multiple bits to a value (0 | 1)
 */
int memmap_set_value(mem_map *map, uint64_t index, char value) {
    uint64_t byte_ind = index / 8;
    uint64_t offset = index % 8;

    if (byte_ind > map->size / 8) {
        return -1;
    }
    value &= 0x1;
    map->data[byte_ind] |= value << offset;
    return 0;
}

/**
 *
 */
int memmap_set(mem_map *map, uint64_t index) {
    uint64_t byte_ind = index / 8;
    uint64_t offset = index % 8;

    if (byte_ind > map->size / 8) {
        return -1;
    }
    map->data[byte_ind] |= 1 << offset;
    return 0;
}
int memmap_clear(mem_map *map, uint64_t index) {
    uint64_t byte_ind = index / 8;
    uint64_t offset = index % 8;

    if (byte_ind > map->size / 8) {
        return -1;
    }

    map->data[byte_ind] &= ~(1 << offset);
    return 0;
}

byte memmap_get(mem_map *map, uint64_t index) {
    uint64_t byte_ind = index / 8;
    uint64_t offset = index % 8;

    if (byte_ind > map->size / 8) {
        return -1;
    }

    return (map->data[byte_ind] & (1 << offset)) >> offset;
}

int memmap_set_len(mem_map *map, uint64_t startIndex, uint64_t len) {
    if ((startIndex + len) > map->size) {
        return -1;
    }
    uint64_t i;
    for (i = startIndex; i < startIndex + len - 8; i += 8) {
        uint64_t byte_ind = i / 8;

        map->data[byte_ind] = 0xFF;
    }

    for (i = startIndex + len - 8; i < startIndex + len; i ++) {
        uint64_t byte_ind = i / 8;
        uint64_t offset = i % 8;

        map->data[byte_ind] |= (1 << offset);
    }
    return 0;
}

int memmap_clear_len(mem_map *map, uint64_t startIndex, uint64_t len) {
    if ((startIndex + len) > map->size) {
        return -1;
    }

    cons_mprintf("clearing values addr: %x\n", &map->data[0]);
    uint64_t i;
    for (i = startIndex; i < startIndex + len - 8; i += 8) {
        uint64_t byte_ind = i / 8;

        cons_mprintf("pre clearing data[%i]\n", byte_ind);
        map->data[byte_ind] = 0;
    }

    for (i = startIndex + len - 8; i < startIndex + len; i ++) {
        uint64_t byte_ind = i / 8;
        uint64_t offset = i % 8;

        map->data[byte_ind] &= ~(1 << offset);
    }
    return 0;
}

