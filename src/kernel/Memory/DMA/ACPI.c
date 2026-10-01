//
// Created by cocon on 02.10.2026.
//

#include "kernel/Memory/DMA/ACPI.h"

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
    uint64_t *PointerToOtherSDT;
}__attribute__((aligned(4)));


uint8_t calculate_checksum(uint8_t *buffer, size_t length) {
    uint8_t sum = 0;
    for (size_t i = 0; i < length; i++) {
        sum += buffer[i];
    }
    return sum; // Must be 0 if valid
}

void *findFACP(void *RootSDT)
{
    struct XSDT *xsdt = (struct XSDT *) RootSDT;
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


int handle_new_acpi(struct XSDP_t * xsdp) {
    if (Istrncmp(xsdp->Signature, SIGNATURE, sizeof(xsdp->Signature)) != 0)
        return -1;

    // checksum for RSDP (first 20 bytes)
    if (calculate_checksum((uint8_t *) xsdp, 20) % 2 != 0) {
        return -1;
    }

    cons_mprintf("OEM: %s", xsdp->OEMID);
    int version = xsdp->Revision + 1;
    // ignore xsdp->RsdtAddress it's deprecated in all new versions

    //checksum for XsdP (all bytes)
    if (calculate_checksum((uint8_t *) xsdp, xsdp->Length) % 2 != 0) {
        return -1;
    }

    struct XSDT * root = (struct XSDT *) xsdp;

    findFACP(xsdp);
    return 0;
}

int handle_old_acpi(struct RSDP_t * rsdp) {
    if (Istrncmp(rsdp->Signature, SIGNATURE, sizeof(rsdp->Signature)) != 0)
        return -1;

    // checksum for RSDP (first 20 bytes)
    if (calculate_checksum((uint8_t *) rsdp, 20) % 2 != 0) {
        return -1;
    }

    cons_mprintf("OEM: %s", rsdp->OEMID);
    int version = rsdp->Revision + 1;
    // ignore xsdp->RsdtAddress it's deprecated in all new versions


    return 0;
}
