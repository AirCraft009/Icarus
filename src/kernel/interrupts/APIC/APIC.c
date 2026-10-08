//
// Created by Mxsxll on 05.10.2026.
//

#include "kernel/interrupts/APIC.h"
#include <stdint.h>

#include "kernel_helper.h"
#include "kernel/interrupts/PIC.h"
#include "kernel/Memory/DMA/MADT.h"


// Code from OsDev Wiki (https://wiki.osdev.org/APIC)

#define IA32_APIC_BASE_MSR 0x1B
#define IA32_APIC_BASE_MSR_BSP 0x100 // Processor is a BSP
#define IA32_APIC_BASE_MSR_ENABLE 0x800
#define PIC1		0x20		/* IO base address for master PIC */
#define PIC2		0xA0		/* IO base address for slave PIC */
#define SPURIOUS_INT_VEC 0xF0


void *LAPIC_ADDR;

/* Set the physical address for local APIC registers */
void cpu_set_apic_base(uintptr_t apic) {
    uint32_t eax = (apic & 0xfffff0000) | IA32_APIC_BASE_MSR_ENABLE;
    uint32_t edx = (apic >> 32) & 0x0f;

    cpuSetMSR(IA32_APIC_BASE_MSR, eax, edx);
}

/**
 * Get the physical address of the APIC registers page
 * make sure you map it to virtual memory ;)
 */
uintptr_t cpu_get_apic_base() {
    uint32_t eax, edx;
    cpuGetMSR(IA32_APIC_BASE_MSR, &eax, &edx);

    return (eax & 0xfffff000) | ((edx & 0x0f) << 32);
}

/**
 * Reads a 32-bit value from a register in the MMIO local APIC
 * @param reg_offset The target register offset
 * @return the 32-bit int to read
 */
uint32_t lapic_read(uint32_t reg_offset) {
    volatile uint32_t* reg = (volatile uint32_t*)(LAPIC_ADDR + reg_offset);

    return *reg;
}

/**
 *  Writes a 32-bit value to a register in the MMIO Local APIC.
 *
 * @param reg_offset The target register offset
 * @param value The 32-bit integer data to write
 */
void lapic_write(uint32_t reg_offset, uint32_t value) {
    volatile uint32_t* reg = (volatile uint32_t*)(LAPIC_ADDR + reg_offset);

    *reg = value;
}


/**
 *  initializes the APIC by:
 *      - disabling PIC (remap & mask)
 *      - writing to Spurious int vec
 * currently is a strange mix between x1 & x2
 * I might migrate later???
 */
void enable_apic() {
    /* Section 11.4.1 of 3rd volume of Intel SDM recommends mapping the base address page as strong uncacheable for correct APIC operation. */

    /* Hardware enable the Local APIC if it wasn't enabled */
    //cpu_set_apic_base(cpu_get_apic_base());
    disable_PIC();

    /* Set the Spurious Interrupt Vector Register bit 8 to start receiving interrupts */
    void * apic = (void *) MMIO_PHYS_TO_VIRT(APIC_INFO->lapic_addr);
    *(uint64_t *) (apic + SPURIOUS_INT_VEC) = (1 << 8) | 0xFF;
    LAPIC_ADDR = apic;
}