//
// Created by cocon on 22.09.2026.
//

#include "mem_utils.h"

#include <stdint.h>

/**
 *  set a region of memory to a specific value.
 *
 * @param addr ptr to start
 * @param val value to set
 * @param length länge
 */
void  Imemset(void *addr, int val, uint64_t length) {
    uint8_t *p = (uint8_t *)addr;

    for (uint64_t i = 0; i < length; i++) {
        *p = (uint8_t)val;
        p++;
    }
}

/**
 *
 *  copies a given amount of bytes from src to dest
 *
 * @param dest start of destination
 * @param src start of source
 * @param length length of mem to copy
 */
void Imemcpy(void *dest, const void *src, uint64_t length) {
    uint8_t *dp = (uint8_t *)dest;
    uint8_t *sp = (uint8_t *)src;

    for (uint64_t i = 0; i < length; i++) {
        dp[i] = sp[i];
    }
}

static inline uint64_t align_up(uint64_t addr, uint64_t align) {
    return (addr + align - 1) & ~(align - 1);
}