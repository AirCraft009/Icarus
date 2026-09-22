//
// Created by cocon on 22.09.2026.
//

#include "mem.h"

#include <stdint.h>

void  memset(void *addr, int val, uint64_t length) {
    uint8_t *p = (uint8_t *)addr;

    for (uint64_t i = 0; i < length; i++) {
        *p = (uint8_t)val;
        p++;
    }
}

void memcpy(void *dest, const void *src, uint64_t length) {
    uint8_t *dp = (uint8_t *)dest;
    uint8_t *sp = (uint8_t *)src;

    for (uint64_t i = 0; i < length; i++) {
        dp[i] = sp[i];
    }
}

static inline uint64_t align_up(uint64_t addr, uint64_t align) {
    return (addr + align - 1) & ~(align - 1);
}