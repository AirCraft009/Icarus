//
// Created by Mxsxll on 05.10.2026.
//

#include "kernel/interrupts/APIC.h"
#include <stdint.h>

#include "kernel_helper.h"
#include "kernel/interrupts/PIC.h"
#include "kernel/Memory/alloc.h"
#include "kernel/Memory/memory_mapping.h"
#include "kernel/Memory/DMA/MADT.h"
#include "kernel/Memory/DMA/MMIO.h"
#include "kernel/util/shellio.h"


// Code from OsDev Wiki (https://wiki.osdev.org/APIC)

#define IA32_APIC_BASE_MSR 0x1B
#define IA32_APIC_BASE_MSR_BSP 0x100 // Processor is a BSP
#define IA32_APIC_BASE_MSR_ENABLE 0x800
#define PIC1		0x20		/* IO base address for master PIC */
#define PIC2		0xA0		/* IO base address for slave PIC */
#define SPURIOUS_INT_VEC 0xF0
#define ADDR_SPACE_IO 1
#define ADDR_SPACE_MMIO 0
#define IO_APIC_MODERN 0x20
#define IO_REG_SEL 0x0
#define IO_WIN 0x10
#define IOAPICVER 0x1
#define REDIR_ENTR_COUNT 16
#define PM_TIMER_HZ 3579545


void *LAPIC_ADDR;
io_apic_descriptor * io_apic_infos;
uint32_t LAPIC_TICKS_PER_MS;

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

void ioapic_write(uintptr_t base, uint8_t reg, uint32_t val) {
    *(volatile uint32_t*)(base + IO_REG_SEL) = reg;   // select register
    *(volatile uint32_t*)(base + IO_WIN) = val;   // write value
}

uint32_t ioapic_read(uintptr_t base, uint8_t reg) {
    *(volatile uint32_t*)(base + IO_REG_SEL) = reg;   // select register
    return *(volatile uint32_t*)(base + IO_WIN);   // write value
}

/**
 *  Route I/O APIC pin 'pin' to core 'apic_id' with vector 'vec'
 **/
void ioapic_route(uintptr_t base, int pin, uint8_t vec, uint8_t apic_id,
                  bool active_low, bool level) {
    uint32_t low = vec | (active_low << 13) | (level << 15);  // unmasked
    ioapic_write(base, 0x11 + 2*pin, (uint32_t)apic_id << 24);  // high half first
    ioapic_write(base, 0x10 + 2*pin, low);                      // low half last
}


/**
 *
 * @param addr a 64bit addr to mmio or a port
 * @param is32 is PM a 24bit or 32bit timer
 * @param isMMIO is it a port access or MMIO access
 */
static uint32_t pm_read(uint64_t addr, bool is32, bool isMMIO) {
    volatile uint32_t v = isMMIO? *(volatile uint32_t *)addr : inportl(addr);
    return is32 ? v : (v & 0xFFFFFF);
}



uint32_t calibrate_lapic_ticks_per_ms(uint64_t pm_addr, bool is32, bool isMMIO) {
    uint32_t mask = is32 ? 0xFFFFFFFF : 0xFFFFFF;
    uint32_t wait = PM_TIMER_HZ / 100;            // minimum window: 10 ms

    lapic_write(LAPIC_TIMER_DIV, 0x3);
    lapic_write(LAPIC_LVT_TIMER, LAPIC_LVT_MASKED);

    uint32_t start = pm_read(pm_addr, is32, isMMIO);
    lapic_write(LAPIC_TIMER_INITCNT, 0xFFFFFFFF);

    uint32_t elapsed;
    do {
        elapsed = (pm_read(pm_addr, is32, isMMIO) - start) & mask;
        //Iprintf("STEP5.%x\n", elapsed);
    } while (elapsed < wait);

    uint32_t cur = lapic_read(LAPIC_TIMER_CURCNT);
    lapic_write(LAPIC_TIMER_INITCNT, 0);

    uint64_t lapic_ticks = 0xFFFFFFFFu - cur;
    // ticks per ms = lapic_ticks / (elapsed / PM_TIMER_HZ * 1000)
    return (uint32_t)((lapic_ticks * PM_TIMER_HZ) / ((uint64_t)elapsed * 1000));
}


/**
 * get the ticks per ms for the lapic timer
 * independent of the system (MMIO or IO) - Space
 */
uint32_t lapic_ticks_per_ms() {

    // check if the X_PMTimerBlock is accessible
    if (!FADT.FullLength) {
        if (FADT.PMTimerBlock == 0)     // check for a null port
            return 0;
        //Iprintf("CALIBRATING: IO-PORT(%x) -> ACPIV1, 32BIT: %i\n", FADT.PMTimerBlock, (FADT.Flags >> 8) & 0x1);
        return calibrate_lapic_ticks_per_ms((uint16_t) FADT.PMTimerBlock, (FADT.Flags >> 8) & 0x1, false);
    }
    if (FADT.X_PMTimerBlock.Address == 0)
        return 0;

    GenericAddressStructure x_timer = FADT.X_PMTimerBlock;
    if (x_timer.AddressSpace == ADDR_SPACE_IO) {
        return calibrate_lapic_ticks_per_ms((uint16_t) x_timer.Address,(FADT.Flags >> 8) & 0x1, false);
    }

    // MMIO mapped space so (ig we first map)
    map_mmio(x_timer.Address, DEFAULT_PAGE_SIZE, WRITEABLE | CACHE_DISABLED);
    return calibrate_lapic_ticks_per_ms(MMIO_PHYS_TO_VIRT(x_timer.Address), (FADT.Flags >> 8) & 0x1, true);
}



/**
 *  initializes the APIC by:
 *      - disabling PIC (remap & mask)
 *      - writing to Spurious int vec
 * currently is a strange mix between x1 & x2
 * I might migrate later???
 **/
int enable_apic() {
    /* Section 11.4.1 of 3rd volume of Intel SDM recommends mapping the base address page as strong uncacheable for correct APIC operation. */

    /* Hardware enable the Local APIC if it wasn't enabled */
    //cpu_set_apic_base(cpu_get_apic_base());
    disable_PIC();

    /* Set the Spurious Interrupt Vector Register bit 8 to start receiving interrupts */
    void * apic = (void *) MMIO_PHYS_TO_VIRT(APIC_INFO->lapic_addr);
    *(uint32_t *) (apic + SPURIOUS_INT_VEC) = (1 << 8) | 0xFF;
    LAPIC_ADDR = apic;

    LAPIC_TICKS_PER_MS = lapic_ticks_per_ms();
    if (!LAPIC_TICKS_PER_MS)
        return -1;
    return 0;
}


uint32_t enable_io_apic(uint64_t io_apic_addr, io_apic_info *info) {

    info->base_address = io_apic_addr;
    info->redirection_count = (ioapic_read(io_apic_addr, IOAPICVER) >> REDIR_ENTR_COUNT & 0xFF) -1;

    return info->redirection_count * sizeof(io_apic_redirection_entry_t);
}

/**
 * enable all io_apics of the system
 */
int enable_io_apics() {
    uint64_t total_size = sizeof(io_apic_infos) * APIC_INFO->ioapic_count;


    for (int i = 0; i < APIC_INFO->ioapic_count; ++i) {
        // read the redirection count and calc the size
        // entry count are bits 16...23 of reg IOAPICVER
        total_size +=
            ((ioapic_read(
                MMIO_PHYS_TO_VIRT(APIC_INFO->ioapics[i].addr),
                IOAPICVER) >> REDIR_ENTR_COUNT & 0xFF) -1)
            * sizeof(io_apic_redirection_entry_t);
    }

    io_apic_infos = (io_apic_descriptor *) Imalloc(sizeof(io_apic_descriptor) + total_size);

    io_apic_info * info = io_apic_infos->apics;
    for (int i = 0; i < APIC_INFO->ioapic_count; ++i) {
        uint32_t info_size = enable_io_apic(MMIO_PHYS_TO_VIRT(APIC_INFO->ioapics[i].addr), info);
        info = (io_apic_info *)((uint8_t *) info + info_size);
    }
    return 0;
}