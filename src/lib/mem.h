//
// Created by cocon on 22.09.2026.
//

#ifndef ICARUS_MEM_H
#define ICARUS_MEM_H
#pragma once
#include <stdint.h>

void  memset(void *addr, int val, uint64_t length);
void memcpy(void *dest, const void *src, uint64_t length);

#endif //ICARUS_MEM_H
