//
// Created by Mxsxll on 05.10.2026.
//

#ifndef ICARUS_APIC_H
#define ICARUS_APIC_H
#pragma once
#include <stdbool.h>
#include <stdint.h>

#include "kernel/Memory/PMM/mem_map.h"

typedef struct {
    uint64_t APIC_addr;
    uint16_t core_count;
    uint16_t IO_APIC_count;
    bool global_nmi_enabled;
}apic_info_header;


extern void *LAPIC_ADDR;

void enable_apic();
void lapic_write(uint32_t reg_offset, uint32_t value);
uint32_t lapic_read(uint32_t reg_offset);

#endif //ICARUS_APIC_H
