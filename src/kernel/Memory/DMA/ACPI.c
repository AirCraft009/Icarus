//
// Created by cocon on 02.10.2026.
//

#include "kernel/Memory/DMA/ACPI.h"

#include "kernel_helper.h"
#include "../memory_mapping.h"
#include "kernel/Memory/DMA/MMIO.h"
#include "kernel/util/mem_utils.h"
#include "kernel/util/shellio.h"

// GOLD!!! https://www.singlix.com/trdos/archive/OSDev_Wiki/RSDP.pdf

#define SIGNATURE "RSD PTR "

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

struct XSDT {
    struct ACPISDTHeader h;
    uint64_t PointerToOtherSDT[];
}__attribute__((aligned(4)));

struct __attribute__((packed)) RSDT {
    struct ACPISDTHeader h;
    uint32_t PointerToOtherSDT[];
};

uint8_t calculate_checksum(uint8_t *buffer, size_t length) {
    uint8_t sum = 0;
    for (size_t i = 0; i < length; i++) {
        sum += buffer[i];
    }
    return sum; // Must be 0 if valid
}

void *findFACPXSDT(struct XSDT *xsdt)
{
    int entries = (xsdt->h.Length - sizeof(xsdt->h)) / 8;

    for (int i = 0; i < entries; i++) {
        struct ACPISDTHeader *h = (struct ACPISDTHeader *) xsdt->PointerToOtherSDT[i];
        if (!Istrncmp(h->Signature, "FACP", 4)){
            cons_mprintf("FACP: %s", h->OEMID);
            return (void *) h;
        }
    }

    // No FACP found
    return NULL;
}

void *findFACPRSDT(struct RSDT *rsdt)
{
    int entries = (rsdt->h.Length - sizeof(struct ACPISDTHeader)) / 4;

    for (int i = 0; i < entries; i++) {
        struct ACPISDTHeader *h = (struct ACPISDTHeader *) KERNEL_PHYS_TO_VIRT(rsdt->PointerToOtherSDT[i]);
        if (!Istrncmp(h->Signature, "FACP", 4)){
            return (void *) h;
        }
    }

    // No FACP found
    return NULL;
}


int handle_new_acpi(struct XSDP_t * xsdp) {
    if (Istrncmp(xsdp->Signature, SIGNATURE, sizeof(xsdp->Signature)) != 0)
        return -1;

    // checksum for RSDP (first 20 bytes)
    if (calculate_checksum((uint8_t *) xsdp, 20) % 2 != 0) {
        return -1;
    }

    cons_mprintf("OEM: %s\n", xsdp->OEMID);
    int version = xsdp->Revision + 1;
    // ignore xsdp->RsdtAddress it's deprecated in all new versions

    //checksum for XsdP (all bytes)
    if (calculate_checksum((uint8_t *) xsdp, xsdp->Length) % 2 != 0) {
        return -1;
    }

    struct XSDT * root = (struct XSDT *)  KERNEL_VIRT_TO_PHYS(xsdp->XsdtAddress);

    findFACPXSDT(root);
    return 0;
}

int handle_old_acpi(struct RSDP_t * rsdp) {
    if (Istrncmp(rsdp->Signature, SIGNATURE, sizeof(rsdp->Signature)) != 0)
        return -1;

    // checksum for RSDP (first 20 bytes)
    if (calculate_checksum((uint8_t *) rsdp, 20) % 2 != 0) {
        return -1;
    }

    cons_mprintf("OEM: %s\n", rsdp->OEMID);
    int version = rsdp->Revision + 1;

    struct RSDT * root_phys = (struct RSDT *) (rsdp->RsdtAddress);
    if (map_mmio((phys_addr_t) root_phys, DEFAULT_PAGE_SIZE, WRITEABLE | CACHE_DISABLED) == FRAME_ALLOC_FAILED) {
        cons_mprintf("error while mapping mmio\n");
        return -1;
    }

    struct ACPISDTHeader * FACP = findFACPRSDT((struct RSDT *) KERNEL_PHYS_TO_VIRT(root_phys));
    return 0;
}