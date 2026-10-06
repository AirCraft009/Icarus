//
// Created by Mxsxll on 06.10.2026.
//

#include "kernel/interrupts/PIC.h"

#include "kernel_helper.h"

/**
 * remaps the PIC interrupts to avoid collisions w/ the cpu exceptions
 * Then we need to mask all lines bc it's not compatible w/ the APIC
 * https://wiki.osdev.org/8259_PIC#Disabling
 */
__inline void disable_PIC() {
    outportb(0x20, 0x11);  outportb(0xA0, 0x11);   // ICW1: begin initialization
    outportb(0x21, 0x20);  outportb(0xA1, 0x28);   // ICW2: vector offsets (32 and 40)
    outportb(0x21, 0x04);  outportb(0xA1, 0x02);   // ICW3: slave is on master's IRQ2
    outportb(0x21, 0x01);  outportb(0xA1, 0x01);   // ICW4: 8086 mode
    outportb(0x21, 0xFF);  outportb(0xA1, 0xFF);   // mask all lines (this also disables it)

}
