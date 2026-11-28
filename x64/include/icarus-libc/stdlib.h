//
// Created by Mxsxll on 6.11.2026.
//
#ifndef STDLIB_H
#define STDLIB_H
#include <stdint.h>
#include <stddef.h>

static inline long icarus_syscall5(uint64_t number, uint64_t first, uint64_t second, uint64_t third, uint64_t fourth, uint64_t fifth);
void exit(int status);
long write(uint64_t descriptor, const void *buffer, unsigned long length, uint64_t offset);
void empty_call ();

#endif
