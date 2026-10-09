//
// Created by cocon on 02.10.2026.
//

#ifndef ICARUS_ACPI_TABLES_H
#define ICARUS_ACPI_TABLES_H
#pragma once
#include <stdint.h>
#include <assert.h>
#include "kernel/Memory/PMM/mem_map.h"
#include "kernel/Memory/DMA/ACPI.h"


typedef struct __attribute__((packed)) {
    uint16_t vendor_id;
    uint16_t device_id;
    uint16_t command;
    uint16_t status;
    uint8_t  revision_id;
    uint8_t  prog_if;
    uint8_t  subclass;
    uint8_t  class_code;
    uint8_t  cache_line;
    uint8_t  latency;
    uint8_t  header_type;   // 0x0E (bit 7 = multifunction)
    uint8_t  bist;
} pci_hdr_common_t;

// Type 0: endpoint
typedef struct __attribute__((packed)) {
    pci_hdr_common_t common;
    uint32_t bar[6];
    uint32_t cardbus_cis;
    uint16_t subsys_vendor;
    uint16_t subsys_id;
    uint32_t rom_base;
    uint8_t  cap_ptr;
    uint8_t  reserved[7];
    uint8_t  int_line;
    uint8_t  int_pin;
    uint8_t  min_grant;
    uint8_t  max_latency;
} pci_hdr0_t;

// Type 1: PCI-to-PCI bridge
typedef struct __attribute__((packed)) {
    pci_hdr_common_t common;
    uint32_t bar[2];
    uint8_t  primary_bus;
    uint8_t  secondary_bus;
    uint8_t  subordinate_bus;
    uint8_t  secondary_latency;
    uint8_t  io_base;
    uint8_t  io_limit;
    uint16_t secondary_status;
    uint16_t mem_base;
    uint16_t mem_limit;
    uint16_t pref_base;
    uint16_t pref_limit;
    uint32_t pref_base_upper;
    uint32_t pref_limit_upper;
    uint16_t io_base_upper;
    uint16_t io_limit_upper;
    uint8_t  cap_ptr;
    uint8_t  reserved[3];
    uint32_t rom_base;
    uint8_t  int_line;
    uint8_t  int_pin;
    uint16_t bridge_control;
} pci_hdr1_t;

static_assert(sizeof(pci_hdr_common_t) == 0x10, "common");
static_assert(sizeof(pci_hdr0_t) == 0x40, "type 0");
static_assert(sizeof(pci_hdr1_t) == 0x40, "type 1");

static_assert(sizeof(pci_hdr0_t) == 0x40, "bad header size");
static_assert(offsetof(pci_hdr0_t, bar) == 0x10, "bad BAR offset");

typedef struct {
    uint64_t base;      // physical address
    uint64_t size;
    bool     is_io;
    bool     prefetchable;
    bool     is_64bit;
} pci_bar_t;

typedef struct pci_dev {
    uint8_t  bus, dev, fn;
    volatile uint32_t *cfg;   // ECAM address of this function
    uint16_t vendor_id, device_id;
    uint8_t  class_code, subclass, prog_if, revision;
    uint8_t  header_type;
    uint8_t  cap_ptr;         // 0 if none
    pci_bar_t bar[6];         // decoded
} pci_dev_t;

static __inline volatile void *ecam_addr(uint64_t base, uint8_t start_bus,
                                       uint8_t bus, uint8_t dev,
                                       uint8_t func, uint16_t off)
{
    return (volatile void *)(base +
        (((uint64_t)(bus - start_bus) << 20) |
         ((uint64_t)dev << 15) |
         ((uint64_t)func << 12) |
         off));
}


typedef struct {
    uint8_t apic_entry_type;
    uint8_t tag_size;
} __attribute__((packed)) apic_tag_common;

typedef struct {
    uint16_t polarity           : 2;
    uint16_t trigger_mode       : 2;
    uint16_t reserved           : 12;
}__attribute__((packed)) apic_flags;

typedef struct ACPI_APIC {
    struct ACPISDTHeader h;
    uint32_t loc_apic_addr;
    uint32_t flags;
    apic_tag_common tags[];
} __attribute__((packed)) MADT;

// apic type 0
typedef struct {
    uint8_t ACPI_proc_id;
    uint8_t APIC_id;
    uint32_t flags;
}__attribute__((packed)) apic_tag_type0;

// apic type 1
typedef struct {
    uint8_t ACPI_IO_id;
    uint8_t reserved;
    uint32_t IO_apic_id;
    uint32_t GSIB;      //global system interrupt base
}__attribute__((packed)) apic_tag_type1;

// apic type 2
typedef struct {
    uint8_t bus_source;
    uint8_t IRQ_source;
    uint32_t GSI;
    apic_flags flags;      //global system interrupt
}__attribute__((packed)) apic_tag_type2;

// apic type 3
typedef struct {
    uint8_t NMI_source;
    uint8_t reserved;
    apic_flags flags;
    uint32_t GSI;      //global system interrupt
}__attribute__((packed)) apic_tag_type3;

// apic type 4
typedef struct {
    uint8_t ACPI_proc_id;       // 0xFF is all processors
    apic_flags flags;
    uint8_t LINT;               //Local Interrupt Pin  (0 or 1)
}__attribute__((packed)) apic_tag_type4;

// apic type 5
typedef struct {
    uint16_t reserved;
    uint64_t LAPIC_addr;
}__attribute__((packed)) apic_tag_type5;

// apic type 9
typedef struct {
    uint16_t Reserved;
    uint32_t x2_LAPIC_id;
    uint32_t flags;
    uint32_t ACPI_id;
}__attribute__((packed)) apic_tag_type9;

extern struct ACPI_MCFG MCFG;
extern struct FADT_TABLE FADT;


// ALL THE BARs https://vlsitrainers.com/pcie-base-address-registers-bars/
#define BAR_TYPE_BIT 1
#define BAR_PREFETCH_BIT 2

#define BAR_TYPE_32BIT 0
#define BAR_TYPE_LEGACY 1
#define BAR_TYPE_64BIT 2

#define BAR_FLAG_MASK 0xFFFFFFF0


static __inline bool bar_is_64bit(volatile uint32_t *bar) {
    return (bar[0] >> BAR_TYPE_BIT) == BAR_TYPE_64BIT;
}

/**
 *
 * @return the physical addr of the BAR value
 */
static __inline uint64_t decode_phys_BAR(volatile uint32_t * bar) {
    uint32_t bar_l = bar[0];

    if (bar_l & 1) {
        // old legacy IO BAR (invalid for now)
        return FRAME_ALLOC_FAILED;
    }

    uint32_t bar_type = (bar_l >> BAR_TYPE_BIT) & 0x3;
    if (bar_type == BAR_TYPE_LEGACY) {
        //not used in this OS bc idk what it does
        return FRAME_ALLOC_FAILED;
    }

    if (bar_type == BAR_TYPE_32BIT) {
        return bar_l & BAR_FLAG_MASK;
    }

    uint32_t bar_h = bar[1];
    return (uint64_t) bar_h << 32 | (bar_l & BAR_FLAG_MASK);
}

static __inline void encode_phys_BAR(volatile uint32_t * bar, uint64_t bar_val) {
    // assume this is a 64 bit value bc why else would you use this method???

    bar[0] = bar_val & 0xFFFFFFFF;
    bar[1] = bar_val >> 32;
}





#endif //ICARUS_ACPI_TABLES_H
