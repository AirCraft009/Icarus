//
// Created by cocon on 22.09.2026.
//

#ifndef ICARUS_MEM_H
#define ICARUS_MEM_H
#pragma once
#include <stdint.h>

void  Imemset(void *addr, uint8_t val, uint64_t length);
void Imemcpy(void *dest, const void *src, uint64_t length);
int Istrncmp(const void *cmp1, const void *cmp2, uint64_t length);
int Imemcmp(const void *str1, const void *str2, uint64_t count);

#endif //ICARUS_MEM_H
