//
// Created by cocon on 20.09.2026.
//
// src from: https://wiki.osdev.org/Interrupts_Tutorial

#include <stdint.h>
#include "IDT.h"
extern void *isr_table[256];

// same segment that we jumped to in boot.asm
static const uint16_t GDT_OFFSET_KERNEL_CODE = 0x8;


typedef struct {
    uint16_t    isr_low;      // The lower 16 bits of the ISR's address
    uint16_t    kernel_cs;    // The GDT segment selector that the CPU will load into CS before calling the ISR
    uint8_t	    ist;          // The IST in the TSS that the CPU will load into RSP; set to zero for now
    uint8_t     attributes;   // Type and attributes; see the IDT page
    uint16_t    isr_mid;      // The higher 16 bits of the lower 32 bits of the ISR's address
    uint32_t    isr_high;     // The higher 32 bits of the ISR's address
    uint32_t    reserved;     // Set to zero
} __attribute__((packed)) idt_entry_t;

__attribute__((aligned(0x10)))
static idt_entry_t idt[256]; // Create an array of IDT entries; aligned for performance


typedef struct {
    uint16_t	limit;
    uint64_t	base;
} __attribute__((packed)) idtr_t;

void idt_set_descriptor(uint8_t vector, void* isr, uint8_t flags, uint8_t ist) {
    idt_entry_t* descriptor = &idt[vector];

    descriptor->isr_low        = (uint64_t)isr & 0xFFFF;
    descriptor->kernel_cs      = GDT_OFFSET_KERNEL_CODE;
    descriptor->ist            = ist;
    descriptor->attributes     = flags;
    descriptor->isr_mid        = ((uint64_t)isr >> 16) & 0xFFFF;
    descriptor->isr_high       = ((uint64_t)isr >> 32) & 0xFFFFFFFF;
    descriptor->reserved       = 0;
}

static idtr_t idtr;


/**
 * Initializes the idt (Interrupt Descriptor Table)
 *  - build the table (fill the 256 entries & set default params)
 *  - set special settings (like IST (Interrupt Stack Table) index)
 *  - actually load the idt
 */
void idt_init(void) {
    idtr.base  = (uintptr_t)&idt[0];
    idtr.limit = sizeof(idt) - 1;

    for (int v = 0; v < 256; v++)
        idt_set_descriptor(v, isr_table[v], 0x8E, 0);   // present, DPL0, interrupt gate

    //Double fault (needs ist entry bc normal stack is not available anymore)
    idt[8].ist = 1;

    __asm__ volatile ("lidt %0" : : "m"(idtr));
    // sti only once the PIC/APIC is set up and c_isr handles the timer
}

