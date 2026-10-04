//
// Created by cocon on 02.10.2026.
//

#ifndef ICARUS_ACPI_TABLES_H
#define ICARUS_ACPI_TABLES_H
#pragma once
#include <stdint.h>

#define MAX_MCFG_ENTRIES 16

// all structs from osdev wiki
struct ACPISDTHeader {
    char Signature[4];
    uint32_t Length;
    uint8_t Revision;
    uint8_t Checksum;
    char OEMID[6];
    char OEMTableID[8];
    uint32_t OEMRevision;
    uint32_t CreatorID;
    uint32_t CreatorRevision;
};

typedef struct
{
    uint8_t AddressSpace;
    uint8_t BitWidth;
    uint8_t BitOffset;
    uint8_t AccessSize;
    uint64_t Address;
} GenericAddressStructure;

struct FADT
{
    struct   ACPISDTHeader h;
    uint32_t FirmwareCtrl;
    uint32_t Dsdt;

    // field used in ACPI 1.0; no longer in use, for compatibility only
    uint8_t  Reserved;

    uint8_t  PreferredPowerManagementProfile;
    uint16_t SCI_Interrupt;
    uint32_t SMI_CommandPort;
    uint8_t  AcpiEnable;
    uint8_t  AcpiDisable;
    uint8_t  S4BIOS_REQ;
    uint8_t  PSTATE_Control;
    uint32_t PM1aEventBlock;
    uint32_t PM1bEventBlock;
    uint32_t PM1aControlBlock;
    uint32_t PM1bControlBlock;
    uint32_t PM2ControlBlock;
    uint32_t PMTimerBlock;
    uint32_t GPE0Block;
    uint32_t GPE1Block;
    uint8_t  PM1EventLength;
    uint8_t  PM1ControlLength;
    uint8_t  PM2ControlLength;
    uint8_t  PMTimerLength;
    uint8_t  GPE0Length;
    uint8_t  GPE1Length;
    uint8_t  GPE1Base;
    uint8_t  CStateControl;
    uint16_t WorstC2Latency;
    uint16_t WorstC3Latency;
    uint16_t FlushSize;
    uint16_t FlushStride;
    uint8_t  DutyOffset;
    uint8_t  DutyWidth;
    uint8_t  DayAlarm;
    uint8_t  MonthAlarm;
    uint8_t  Century;

    // reserved in ACPI 1.0; used since ACPI 2.0+
    uint16_t BootArchitectureFlags;

    uint8_t  Reserved2;
    uint32_t Flags;

    // 12 byte structure; see below for details
    GenericAddressStructure ResetReg;

    uint8_t  ResetValue;
    uint8_t  Reserved3[3];

    // 64bit pointers - Available on ACPI 2.0+
    uint64_t                X_FirmwareControl;
    uint64_t                X_Dsdt;

    GenericAddressStructure X_PM1aEventBlock;
    GenericAddressStructure X_PM1bEventBlock;
    GenericAddressStructure X_PM1aControlBlock;
    GenericAddressStructure X_PM1bControlBlock;
    GenericAddressStructure X_PM2ControlBlock;
    GenericAddressStructure X_PMTimerBlock;
    GenericAddressStructure X_GPE0Block;
    GenericAddressStructure X_GPE1Block;
};

typedef struct {
    uint64_t conf_base_addr;        //Base address of enhanced configuration mechanism
    uint16_t segment_group_num;     //PCI Segment Group Number
    uint8_t start_pci; 	            //Start PCI bus number decoded by this host bridge
    uint8_t end_pci; 	            //End PCI bus number decoded by this host bridge
    uint32_t reserved;
} ECAM;

typedef struct ACPI_MCFG {
    struct ACPISDTHeader h;
    ECAM conf_space_addrs[MAX_MCFG_ENTRIES];
}mcfg;

#include <stdint.h>
#include <assert.h>

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

_Static_assert(sizeof(pci_hdr0_t) == 0x40, "bad header size");
_Static_assert(offsetof(pci_hdr0_t, bar) == 0x10, "bad BAR offset");

static inline volatile void *ecam_addr(uint64_t base, uint8_t start_bus,
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
#endif //ICARUS_ACPI_TABLES_H
