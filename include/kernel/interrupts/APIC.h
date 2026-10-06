//
// Created by Mxsxll on 05.10.2026.
//

#ifndef ICARUS_APIC_H
#define ICARUS_APIC_H
#pragma once
#include <stdbool.h>
#include <stdint.h>

typedef struct {
    uint64_t APIC_addr;
    uint16_t core_count;
    uint16_t IO_APIC_count;
    bool global_nmi_enabled;
}apic_info_header;


void enable_apic();

#endif //ICARUS_APIC_H
