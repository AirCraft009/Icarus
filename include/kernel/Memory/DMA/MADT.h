//
// Created by cocon on 06.10.2026.
//

#ifndef ICARUS_MADT_H
#define ICARUS_MADT_H
#pragma once
#include "ACPI_helpers.h"
#include "kernel/Memory/DMA/ACPI.h"

// yes these are duplicate (that's not proper system design you say, why am talking to myself? Have I gone insane und warum in Englisch???)

//flags bit0 = enabled, bit1 = can be enabled later ("online capable")
typedef struct { uint8_t acpi_id, apic_id; uint32_t flags; } cpu_info;
typedef struct { uint8_t id; uint32_t addr; uint32_t gsi_base; } ioapic_info;
typedef struct { uint8_t bus, source_irq; uint32_t gsi; apic_flags flags; } iso_info;
typedef struct { uint32_t gsi; apic_flags flags; } nmi_info;
// acpi_id 0xFF means "applies to all CPUs"
typedef struct { uint8_t acpi_id; uint8_t lint; apic_flags flags; } lapic_nmi_info;

typedef struct madt_info {
    uint64_t lapic_addr;        // physical address of the LAPIC registers  (mapped into MMIO space)
    uint32_t madt_flags;        // bit0: legacy dual 8259 PICs also exist (lwk just mask it no matter cause it can't hurt yk
    uint32_t cpu_count, ioapic_count, iso_count, nmi_count, lapic_nmi_count;

    // These point into the same allocation, right after this struct
    cpu_info       *cpus;
    ioapic_info    *ioapics;
    iso_info       *isos;
    nmi_info       *nmis;
    lapic_nmi_info *lapic_nmis;
} madt_info;

extern madt_info * APIC_INFO;

uint64_t handle_MADT(struct ACPISDTHeader *header);

#endif //ICARUS_MADT_H
