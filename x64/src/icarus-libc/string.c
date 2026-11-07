//
// Created by Mxsxll on 6.11.2026.
//

#include <stdint.h>
#include "../include/icarus-libc/string.h"

/**
 *  set a region of memory to a specific value.
 *
 * @param addr ptr to start
 * @param val value to set
 * @param length in bytes
 */
void  memset(void *addr, uint8_t val, uint64_t length) {
    uint64_t *word_p = (uint64_t *)addr;

    uint64_t word_count = length / sizeof(uint64_t);
    uint64_t word_val = 0x0101010101010101ULL * val;            // make the value into a 64 bit int
    for (uint64_t i = 0; i < word_count; i++) {
        *word_p = word_val;
        word_p++;
    }
    uint8_t * p = (uint8_t *)word_p;

    for (uint64_t i = 0; i < length - word_count * sizeof(uint64_t); i++) {
        p[i] = val;
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
void memcpy(void *dest, const void *src, uint64_t length) {
    uint8_t *dp = (uint8_t *)dest;
    uint8_t *sp = (uint8_t *)src;

    uint64_t word_count = length / sizeof(uint64_t);
    for (uint64_t i = 0; i < word_count; i++) {
        *(uint64_t *)dp = *(uint64_t *)sp;
        dp += sizeof(uint64_t);
        sp += sizeof(uint64_t);
    }

    for (uint64_t i = 0; i < length - word_count * sizeof(uint64_t); i++) {
        *(char *)dp = *(char *)sp;
        dp++;
        sp++;
    }
}

/**
 * compares the values in each byte of cmp1 & cmp2
 * @return 0 on equal diff cmp1[i] - cmp[2]
 */
int strncmp(const char *cmp1, const char *cmp2, uint64_t length) {
    //TODO: first try to set w/ u64
    uint8_t *dp = (uint8_t *)cmp1;
    uint8_t *sp = (uint8_t *)cmp2;
    for (uint64_t i = 0; i < length && dp[i] != '\0'; i++) {
        if (dp[i] != sp[i])
            return dp[i] - sp[i];
    }
    return 0;
}

int memcmp(const void *str1, const void *str2, uint64_t count) {
    const unsigned char *s1 = str1;
    const unsigned char *s2 = str2;
    while (count-- > 0) {
        if (*s1++ != *s2++)
            return s1[-1] < s2[-1] ? -1 : 1;
    }
    return 0;
}

size_t strlen(const char *string){
    size_t length = 0;
    while (string[length] != '\0')
        length++;
    return length;
}


