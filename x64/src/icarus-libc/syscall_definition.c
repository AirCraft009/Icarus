//
// Created by Mxsxll on 6.11.2026.
//
#include "syscall_definition.h"
#include "icarus-libc/stdlib.h"
#include <stdint.h>

#include "kernel/util/shellio.h"

static inline long icarus_syscall5(uint64_t number, uint64_t first, uint64_t second, uint64_t third, uint64_t fourth, uint64_t fifth)
{
    register long rfourth __asm__("r10") = fourth;
    register long rfifth __asm__("r8") = fifth;
    long result;
    __asm__ volatile ("syscall"
                      : "=a"(result)
                      : "a"(number), "D"(first), "S"(second), "d"(third), "r"(rfourth), "r"(rfifth)
                      : "rcx", "r11", "memory");
    return result;
}

void exit(int status)
{
    icarus_syscall5(SYS_EXIT_PROCESS, status, 0, 0, 0, 0);
    for (;;) { }
}

long write(uint64_t descriptor, const void *buffer, uint64_t length, uint64_t offset)
{
    return icarus_syscall5(SYS_WRITE, descriptor, (long)buffer, length, offset, 0);
}

void empty_call () {
    icarus_syscall5(SYS_EMPTY, 0,0,0, 0, 0);
}