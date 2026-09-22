//
// Created by Mxsxll on 22.09.2026.
//

#include "bitmap.h"

int bitmap_set_value(bitmap *map, uint64_t index, char value) {
    uint64_t byte_ind = index / 8;
    uint64_t offset = index % 8;

    if (byte_ind > map->size / 8) {
        return -1;
    }
    value &= 0x1;
    map->data[byte_ind] |= value << offset;
    return 0;
}
int bitmap_set(bitmap *map, uint64_t index) {
    uint64_t byte_ind = index / 8;
    uint64_t offset = index % 8;

    if (byte_ind > map->size / 8) {
        return -1;
    }
    map->data[byte_ind] |= 1 << offset;
    return 0;
}
int bitmap_clear(bitmap *map, uint64_t index) {
    uint64_t byte_ind = index / 8;
    uint64_t offset = index % 8;

    if (byte_ind > map->size / 8) {
        return -1;
    }

    map->data[byte_ind] &= ~(1 << offset);
    return 0;
}

byte bitmap_get(bitmap *map, uint64_t index) {
    uint64_t byte_ind = index / 8;
    uint64_t offset = index % 8;

    if (byte_ind > map->size / 8) {
        return -1;
    }

    return (map->data[byte_ind] & (1 << offset)) >> offset;
}

int bitmap_set_values(bitmap *map, uint64_t startIndex, uint64_t len) {
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

int bitmap_clear_values(bitmap *map, uint64_t startIndex, uint64_t len) {
    if ((startIndex + len) > map->size) {
        return -1;
    }
    uint64_t i;
    for (i = startIndex; i < startIndex + len - 8; i += 8) {
        uint64_t byte_ind = i / 8;

        map->data[byte_ind] = 0;
    }

    for (i = startIndex + len - 8; i < startIndex + len; i ++) {
        uint64_t byte_ind = i / 8;
        uint64_t offset = i % 8;

        map->data[byte_ind] &= ~(1 << offset);
    }
    return 0;
}