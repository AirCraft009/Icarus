//
// Created by Mxsxll on 15.10.2026.
//

#ifndef ICARUS_ISR_H
#define ICARUS_ISR_H
#pragma once
#include <stdint.h>

#include "kernel/util/cpu_regs.h"

typedef enum {
    INT_TMR,
    INT_KBD,
    INT_GEN,
} INTERRUPT_SRC;

struct interrupt_frame {
    // pushed by isr_common
    uint64_t cr4, cr3, cr2;                       // faulting address (valid on #PF)
    cpu_reg_frame gpo_regs;
    // pushed by the stub
    uint64_t vector;
    uint64_t error_code;                // 0 if the CPU didn't push one
    // pushed by the CPU
    cpu_push_frame cpu_regs;
}__attribute__((packed));
_Static_assert(sizeof(struct interrupt_frame) == 200, "asm/C frame mismatch");

#endif //ICARUS_ISR_H
