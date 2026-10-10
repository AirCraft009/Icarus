//
// Created by Mxsxll on 05.10.2026.
//

#ifndef ICARUS_APIC_H
#define ICARUS_APIC_H
#pragma once
#include <stdbool.h>
#include <stdint.h>

#define LAPIC_EOI            0xB0
#define LAPIC_TIMER_DIV      0x3E0
#define LAPIC_LVT_TIMER      0x320
#define LAPIC_TIMER_INITCNT  0x380
#define LAPIC_TIMER_CURCNT   0x390
#define APIC_LVT_PERF	     0x340
#define APIC_LVT_LINT0	     0x350
#define APIC_LVT_LINT1	     0x360
#define APIC_LVT_ERR	     0x370
#define LAPIC_LVT_MASKED     0x10000
#define LAPIC_TIMER_PERIODIC 0x20000


/**
 * Redirection entry (GSI -> IRQ) for the io-apic
 * Split into two anonym structs to assure 32bit field access
 */
typedef union IOAPICRedirectionEntry{
    struct __attribute__((packed)) {
        struct __attribute__((packed))
        {
            uint32_t vector       : 8;
            uint32_t delvMode     : 3;
            uint32_t destMode     : 1;
            uint32_t delvStatus   : 1;
            uint32_t pinPolarity  : 1;
            uint32_t remoteIRR    : 1;
            uint32_t triggerMode  : 1;
            uint32_t mask         : 1;
            uint32_t reserved_1   : 15;
        };
        struct __attribute__((packed))
        {
            uint32_t reserved_2   : 24;
            uint32_t destination  : 8;
        };
    };
    struct __attribute__((packed)) {
        uint32_t low_w;
        uint32_t high_w;
    };
}__attribute__((packed)) io_apic_redirection_entry_t;

typedef struct {
    uint64_t base_address;
    uint8_t io_apic_id;
    uint8_t io_apic_version;
    uint8_t redirection_count;      // how many entries can this ioApic hold
    io_apic_redirection_entry_t redirection_table[];
} io_apic_info;

typedef struct {
    uint64_t count;
    io_apic_info apics[];
} io_apic_descriptor;

extern io_apic_descriptor * io_apic_infos;
extern void *LAPIC_ADDR;
extern uint32_t LAPIC_TICKS_PER_MS;

int enable_apic();
int enable_io_apics();
void lapic_write(uint32_t reg_offset, uint32_t value);
uint32_t lapic_read(uint32_t reg_offset);

#endif //ICARUS_APIC_H
