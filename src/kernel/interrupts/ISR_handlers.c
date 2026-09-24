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
}__attribute__((packed));
_Static_assert(sizeof(struct interrupt_frame) == 184, "asm/C frame mismatch");


void page_fault_handler(struct interrupt_frame *frame){
    cons_mprintf("#Page Fault detected: %x\nerror: %x\n", frame->cr2, frame->error_code);
    //TODO: stop user process
    __asm__ volatile ("cli; hlt"); // Completely hangs the computer
}

void general_protection_fault_handler(struct interrupt_frame *frame){
    cons_mprintf( "#General Fault detected\nsegment: %i\nerror: %i\nRAX %x\nCR2 %x\n", frame->ss, frame->error_code, frame->rax, frame->cr2);

    uint8_t * instruction_data = (uint8_t*)frame->rip;
    cons_mprintf("rip: b1=%x,b2=%x,b3=%x\n", instruction_data[0],instruction_data[1],instruction_data[2]);
    cons_mprintf("fulldata=");
    for (int i = 0; i < 10; i++) {
        cons_mprintf("%x ", instruction_data[i]);
    }

    //TODO: stop user process
    __asm__ volatile ("cli; hlt"); // Completely hangs the computer
}

void undefined_instruction_handler(struct interrupt_frame *frame) {
    cursor cur = (cursor) {0,0};
    uint8_t *fault_instr = (uint8_t *)frame->rip;

    cons_mprintf( "undefined instruction: %i", *fault_instr);

    if (frame->cs == 0) {
        // ring 0 (kernel mode #UD) is not recoverable (FOR NOW MUHAHAHAHA)
        __asm__ volatile ("cli; hlt");
    }

    //TODO: stop user process
}

void c_isr(
    struct interrupt_frame *frame
)
{
    cursor cur = {0,0};
    cons_mprintf("Registered w/ general handler: %i\n", frame->vector);
    switch (frame->vector) {
        case 0:
            // Divide error
            break;

        case 8:
            undefined_instruction_handler(frame);
            break;

        case 13:
            general_protection_fault_handler(frame);
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