//
// Created by cocon on 22.09.2026.
//

#include <stdint.h>

/**
 *  set a region of memory to a specific value.
 *
 * @param addr ptr to start
 * @param val value to set
 * @param length länge
 */
void  Imemset(void *addr, uint8_t val, uint64_t length) {
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
void Imemccpy(void *dest, const void *src, uint64_t length) {
    uint8_t *dp = (uint8_t *)dest;
    uint8_t *sp = (uint8_t *)src;

    for (uint64_t i = 0; i < length; i++) {
        dp[i] = sp[i];
    }
}

/**
 * compares the values in each byte of cmp1 & cmp2
 * @return 0 on equal diff cmp1[i] - cmp[2]
 */
int Istrncmp(const char *cmp1, const char *cmp2, uint64_t length) {
    uint8_t *dp = (uint8_t *)cmp1;
    uint8_t *sp = (uint8_t *)cmp2;
    for (uint64_t i = 0; i < length; i++) {
        if (dp[i] != sp[i]) {
            return dp[i] - sp[i];
        }
    }
    return 0;
}

int Imemcmp(const void *str1, const void *str2, uint64_t count) {
    const unsigned char *s1 = str1;
    const unsigned char *s2 = str2;
    while (count-- > 0) {
        if (*s1++ != *s2++)
            return s1[-1] < s2[-1] ? -1 : 1;
    }
    return 0;
}
