//
// Created by Mxsxll on 22.09.2026.
//

#include "BitMap.h"

#include <stdbool.h>
#include <stddef.h>

#include "kernel_info.h"
#include "../../shell/shellio.h"

typedef struct MemMap {

    bit_map regions[];
}mem_map;

/**
 * sets bit to a value (0 | 1)
 */
int bitmap_set_value(bit_map *map, uint64_t index, char value) {
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
int bitmap_set(bit_map *map, uint64_t index) {
    uint64_t byte_ind = index / 8;
    uint64_t offset = index % 8;

    if (byte_ind > map->size / 8) {
        return -1;
    }
    map->data[byte_ind] |= 1 << offset;
    return 0;
}
int bitmap_clear(bit_map *map, uint64_t index) {
    uint64_t byte_ind = index / 8;
    uint64_t offset = index % 8;

    if (byte_ind > map->size / 8) {
        return -1;
    }

    map->data[byte_ind] &= ~(1 << offset);
    return 0;
}

byte bitmap_get(bit_map *map, uint64_t index) {
    uint64_t byte_ind = index / 8;
    uint64_t offset = index % 8;

    if (byte_ind > map->size / 8) {
        return -1;
    }

    return (map->data[byte_ind] & (1 << offset)) >> offset;
}

int bitmap_set_len(bit_map *map, uint64_t startIndex, uint64_t len) {
    if (startIndex > map->size || len > map->size - startIndex) {
        return -1;
    }

    uint64_t end = startIndex + len;
    uint64_t i = startIndex;

    // Set bits until byte-aligned.
    while (i < end && (i % 8) != 0) {
        map->data[i / 8] |= (uint8_t)(1u << (i % 8));
        i++;
    }

    // Set complete bytes.
    while (i + 8 <= end) {
        map->data[i / 8] = 0xFF;
        i += 8;
    }

    // Set remaining bits.
    while (i < end) {
        map->data[i / 8] |= (uint8_t)(1u << (i % 8));
        i++;
    }

    return 0;
}

int bitmap_clear_len(bit_map *map, uint64_t startIndex, uint64_t len) {
    if (startIndex > map->size || len > map->size - startIndex) {
        return -1;
    }

    uint64_t end = startIndex + len;
    uint64_t i = startIndex;

    // Set bits until byte-aligned.
    while (i < end && (i % 8) != 0) {
        map->data[i / 8] &= (uint8_t)~(1u << (i % 8));
        i++;
    }

    // Set complete bytes.
    while (i + 8 <= end) {
        map->data[i / 8] = 0x0;
        i += 8;
    }

    // Set remaining bits.
    while (i < end) {
        map->data[i / 8] &= (uint8_t)~(1u << (i % 8));
        i++;
    }

    return 0;
}

void show_mmap(bit_map *map) {
    uint64_t i = 0;
    uint64_t blockS = 0;
    bool currently_set = (map->data[0] & 0x1) == 1;
    while (i < map->size) {
        while (i < map->size
            && currently_set == ((map->data[i / 8] >> (i % 8)) & 1u))
        {
            blockS ++;
            i++;
        }
        cons_mprintf("found block(%i): %x - %x\n",currently_set,  i - blockS, i);
        currently_set = !currently_set;
        blockS = 0;
    }
}

int show_mmap_range(bit_map *map, uint64_t start, uint64_t end) {
    if (start > map->size || end > map->size - start) {
        return -1;
    }
    uint64_t i = start;
    uint64_t blockS = 0;
    bool currently_set = (map->data[start] & 0x1) == 1;
    while (i < end) {
        while (i < end
            && currently_set == ((map->data[i / 8] >> (i % 8)) & 1u))
        {
            blockS ++;
            i++;
        }
        cons_mprintf("found block(%i): %x - %x\n",currently_set,  (i - blockS) * DEFAULT_PAGE_SIZE, i * DEFAULT_PAGE_SIZE);
        currently_set = !currently_set;
        blockS = 0;
    }
    return 0;
}