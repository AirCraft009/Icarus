//
// Created by cocon on 13.10.2026.
//

#ifndef ICARUS_CPU_REGS_H
#define ICARUS_CPU_REGS_H
#include <stdint.h>

// Define CPU registers structure
typedef struct {
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rbp, rdi, rsi, rdx, rcx, rbx, rax;
} cpu_reg_frame;

#endif //ICARUS_CPU_REGS_H
