//
// Created by cocon on 20.09.2026.
//
#include <stdint.h>
#include "../../shell/shellio.h"

struct interrupt_frame {
    // pushed by isr_common
    uint64_t cr2;                       // faulting address (valid on #PF)
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rbp, rdi, rsi, rdx, rcx, rbx, rax;
    // pushed by the stub
    uint64_t vector;
    uint64_t error_code;                // 0 if the CPU didn't push one
    // pushed by the CPU
    uint64_t rip, cs, rflags, rsp, ss;
};
_Static_assert(sizeof(struct interrupt_frame) == 184, "asm/C frame mismatch");


void page_fault_handler(struct interrupt_frame *frame){
    cursor cur = (cursor) {0,0};

    char *error = "#Page Fault detected %i";
    __asm__ volatile ("cli; hlt"); // Completely hangs the computer
}

void c_isr(
    struct interrupt_frame *frame
)
{
    switch (frame->vector) {
        case 0:
            // Divide error
            break;

        case 14:
            page_fault_handler(frame);
            break;

        case 32:
            // Timer IRQ
            break;

        default:
            // Unhandled interrupt
            break;
    }
}