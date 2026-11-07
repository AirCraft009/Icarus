//
// Created by Mxsxll on 6.11.2026.
//
#include "../../syscall_definition.h"
#include "../include/stdlib.h"
#include <stdint.h>

static inline long icarus_syscall3(uint64_t number, uint64_t first, uint64_t second, uint64_t third)
{
    long result;
    __asm__ volatile ("syscall"
                      : "=a"(result)
                      : "a"(number), "D"(first), "S"(second), "d"(third)
                      : "rcx", "r11", "memory");
    return result;
}

void exit(int status)
{
    icarus_syscall3(SYS_EXIT_PROCESS, status, 0, 0);
    for (;;) { }
}

long write(int descriptor, const void *buffer, unsigned long length)
{
    return icarus_syscall3(SYS_WRITE, descriptor, (long)buffer, (long)length);
}