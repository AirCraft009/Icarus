//
// Created by cocon on 02.10.2026.
//

#ifndef ICARUS_ACPI_H
#define ICARUS_ACPI_H
#pragma once
#include "kernel/util/multiboot2.h"
#include <assert.h>


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


typedef struct {
    uint64_t conf_base_addr;        //Base address of enhanced configuration mechanism
    uint16_t segment_group_num;     //PCI Segment Group Number
    uint8_t start_pci; 	            //Start PCI bus number decoded by this host bridge
    uint8_t end_pci; 	            //End PCI bus number decoded by this host bridge
    uint32_t reserved;
} __attribute__((packed)) ECAM;

typedef struct ACPI_MCFG {
    struct ACPISDTHeader h;
    uint64_t reserved;
    ECAM conf_space_addrs[MAX_MCFG_ENTRIES];
}__attribute__((packed)) mcfg;
static_assert(offsetof(mcfg, conf_space_addrs) == 44, "conf_base_addr misaligned");


//OSDEV wiki
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
    // In this OS it'll serve to repr wether ACPI is v1(0) or v2(1)
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


int handle_new_acpi(struct XSDP_t * rsdp);
int handle_old_acpi(struct RSDP_t * rsdp);


#endif //ICARUS_ACPI_H

