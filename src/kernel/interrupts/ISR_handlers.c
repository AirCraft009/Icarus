//
// Created by cocon on 20.09.2026.
//
#include <stdint.h>

#include "kernel/Drivers/PS2/PS2_Controller.h"
#include "kernel/Drivers/PS2/PS2_Keyboard.h"
#include "kernel/interrupts/APIC.h"
#include "kernel/util/shellio.h"
#include "kernel/interrupts/ISR.h"
#include "kernel/Processes/Scheduling/scheduler.h"
#include "kernel/util/cpu_regs.h"




void Stack_segment_fault(struct interrupt_frame *frame) {
    Iprintf("Stack segfault");
    Iprintf("RF: %x\n", frame->cpu_regs.rflags);
    Iprintf("RIP: %x\n", frame->cpu_regs.rip);
    Iprintf("CS: %x\n", frame->cpu_regs.cs);
    Iprintf("RSP: %x\n", frame->cpu_regs.rsp);
    Iprintf("SS: (%x)\n", frame->cpu_regs.ss);
}

void page_fault_handler(struct interrupt_frame *frame){
    Iprintf("firstB (%x)\n", *(uint8_t *)PROCESS_CODE_VIRT_ADDR);
    Iprintf("#Page Fault detected: (%x)\nerror: %x\n", frame->cr2, frame->error_code);
    //TODO: stop user process
    __asm__ volatile ("cli; hlt"); // Completely hangs the computer
}

void general_protection_fault_handler(struct interrupt_frame *frame){
    Iprintf( "#General Fault detected\nsegment: (%i)\nerror: (%i)\nRAX (%x)\nCR2 (%x)\nCR3: (%x)\nCR4: (%x)", frame->cpu_regs.ss, frame->error_code, frame->gpo_regs.rax, frame->cr2, frame->cr3, frame->cr4);

    uint8_t * instruction_data = (uint8_t*)frame->cpu_regs.rip;
    Iprintf("rip: b1=%x,b2=%x,b3=%x\n", instruction_data[0],instruction_data[1],instruction_data[2]);
    Iprintf("fulldata=");
    for (int i = 0; i < 10; i++) {
        Iprintf("%x ", instruction_data[i]);
    }

    //TODO: stop user process
    __asm__ volatile ("cli; hlt"); // Completely hangs the computer
}

void undefined_instruction_handler(struct interrupt_frame *frame) {
    cursor cur = (cursor) {0,0};
    uint8_t *fault_instr = (uint8_t *)frame->cpu_regs.rip;

    Iprintf( "undefined instruction: %i", *fault_instr);

    if (frame->cpu_regs.cs == 0) {
        // ring 0 (kernel mode #UD) is not recoverable (FOR NOW MUHAHAHAHA)
        __asm__ volatile ("cli; hlt");
    }

    //TODO: stop user process
}


void timer_interrupt_handler(struct interrupt_frame *frame) {
    Iprintf("scheduling\n");
    schedule(frame, INT_TMR);
}

void keyboard_interrupt_handler(struct interrupt_frame *frame) {
    char c = kbd_translate(ps2_read_timeout());
    if (c != 0)
        Iprintf("%c",c);
    lapic_write(LAPIC_EOI, 0);
}

void c_isr(
    struct interrupt_frame *frame
)
{
    //Iprintf("Registered w/ fault handler: %x\n", frame->vector);
    switch (frame->vector) {
        case 0:
            // Divide error
            break;

        case 8:
            undefined_instruction_handler(frame);
            break;


        case 12:
            Stack_segment_fault(frame);
            break;

        case 13:
            general_protection_fault_handler(frame);
            break;

        case 14:
            page_fault_handler(frame);
            break;

        case 32:
            // Timer IRQ
            timer_interrupt_handler(frame);
            break;

        case 33:
            keyboard_interrupt_handler(frame);
            break;

        default:
            // Unhandled interrupt
            break;
    }
    return;
}