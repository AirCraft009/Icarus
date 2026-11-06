//
// Created by Mxsxll on 6.11.2026.
//
#ifndef STRING_H
#define STRING_H
#include <stddef.h>
#include <stdint.h>

void  memset(void *addr, uint8_t val, size_t length);
void memcpy(void *dest, const void *src, size_t length);
int strncmp(const char *cmp1, const char *cmp2, uint64_t length);
int memcmp(const void *str1, const void *str2, uint64_t count);

#endif