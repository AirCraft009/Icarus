//
// Created by Mxsxll on 05.10.2026.
//

#include "kernel/interrupts/APIC.h"
#include <stdint.h>

#include "kernel_helper.h"
#include "kernel/interrupts/PIC.h"


// Code from OsDev Wiki (https://wiki.osdev.org/APIC)

#define IA32_APIC_BASE_MSR 0x1B
#define IA32_APIC_BASE_MSR_BSP 0x100 // Processor is a BSP
#define IA32_APIC_BASE_MSR_ENABLE 0x800
#define PIC1		0x20		/* IO base address for master PIC */
#define PIC2		0xA0		/* IO base address for slave PIC */

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

void enable_apic() {
    /* Section 11.4.1 of 3rd volume of Intel SDM recommends mapping the base address page as strong uncacheable for correct APIC operation. */

    /* Hardware enable the Local APIC if it wasn't enabled */
    cpu_set_apic_base(cpu_get_apic_base());
    disable_PIC();

    /* Set the Spurious Interrupt Vector Register bit 8 to start receiving interrupts */
    //write_reg(0xF0, ReadRegister(0xF0) | 0x100);
}