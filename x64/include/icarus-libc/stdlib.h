//
// Created by Mxsxll on 6.11.2026.
//
#ifndef STDLIB_H
#define STDLIB_H
#include <stdint.h>
#include <stddef.h>

static inline long icarus_syscall3(uint64_t number, uint64_t first, uint64_t second, uint64_t third);
void exit(int status);
long write(int descriptor, const void *buffer, unsigned long length);

#endif
