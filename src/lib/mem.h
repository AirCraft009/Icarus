//
// Created by cocon on 22.09.2026.
//

#ifndef ICARUS_MEM_H
#define ICARUS_MEM_H
#pragma once
#include <stdint.h>

void  Imemset(void *addr, int val, uint64_t length);
void Imemcpy(void *dest, const void *src, uint64_t length);
static inline uint64_t align_up(uint64_t addr, uint64_t align);

#endif //ICARUS_MEM_H
