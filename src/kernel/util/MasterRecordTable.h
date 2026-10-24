//
// Created by cocon on 19.10.2026.
//

#ifndef ICARUS_MASTERRECORDTABLE_H
#define ICARUS_MASTERRECORDTABLE_H
#pragma once
#include "kernel/Drivers/PS2/PS2_Controller.h"
#include "kernel/interrupts/APIC.h"
#include "kernel/Memory/DMA/ACPI_helpers.h"
#include "kernel/Memory/DMA/MADT.h"

typedef struct MASTER_RECORD_TABLE {
    mcfg MCFG;
    PS2_info_t PS2_info;
    madt_info APIC_INFO;
    ioapic_descriptor *descriptor;
    void *LAPIC_ADDR;
    uint32_t LAPIC_MS_PER_SEC;
    MADT FADT;
}master_record_table_t;

extern master_record_table_t MasterRecordTable;

#endif //ICARUS_MASTERRECORDTABLE_H
