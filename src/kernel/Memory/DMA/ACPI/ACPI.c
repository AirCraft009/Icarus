//
// Created by cocon on 02.10.2026.
//

#include "kernel/Memory/DMA/ACPI.h"

#include "kernel_helper.h"
#include "kernel/Memory/memory_mapping.h"
#include "kernel/Memory/DMA/MMIO.h"
#include "kernel/util/mem_utils.h"
#include "kernel/util/shellio.h"
#include "kernel/Memory/DMA/ACPI_helpers.h"
#include "kernel/Memory/DMA/MADT.h"
#include "kernel/Memory/DMA/PCIE.h"

// GOLD!!! https://www.singlix.com/trdos/archive/OSDev_Wiki/RSDP.pdf

#define CHECKSUM(buff, p)  \
    if(calculate_checksum((uint8_t *) (buff), (uint64_t) (p)) != 0){ \
        return -1; \
    }
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define SIGNATURE "RSD PTR "


struct XSDT {
    struct ACPISDTHeader h;
    uint64_t PointerToOtherSDT[];
}__attribute__((aligned(4)));

struct __attribute__((packed)) RSDT {
    struct ACPISDTHeader h;
    uint32_t PointerToOtherSDT[];
};

// zero init so that ACPI v1 doesn't read uninit slop
static struct FADT FACP = {0};
struct ACPI_MCFG MCFG = {0};

uint8_t calculate_checksum(uint8_t *buffer, uint64_t length) {
    uint8_t sum = 0;
    for (size_t i = 0; i < length; i++) {
        sum += buffer[i];
    }
    return sum; // Must be 0 if valid
}


int handle_table(struct ACPISDTHeader *h) {
    if (!Istrncmp(h->Signature, "FACP", 4)){
        Imemccpy(&FACP, h, MIN(h->Length, sizeof(struct FADT)));
        CHECKSUM(&FACP, h->Length);
    }
    else if (!Istrncmp(h->Signature, "MCFG", 4)) {
        Imemccpy(&MCFG, h, h->Length);
        Iprintf("found MCFG: (%x)\n", &MCFG);
        CHECKSUM(&MCFG, h->Length);
    }
    else if (!Istrncmp(h->Signature, "APIC", 4)) {
        Iprintf("found APIC(MADT): %x\n", h);
        handle_MADT(h);
        Iprintf("PLEASE WORK: (%i)\n", APIC_INFO->lapic_addr);
        CHECKSUM(&MCFG, h->Length);
    }
    return 0;
}

int categoriseTablesXSDT(struct XSDT *xsdt){
    int entries = (xsdt->h.Length - sizeof(xsdt->h)) / 8;

    for (int i = 0; i < entries; i++) {
        struct ACPISDTHeader *h = (struct ACPISDTHeader *) MMIO_PHYS_TO_VIRT(xsdt->PointerToOtherSDT[i]);
        if (handle_table(h) != 0)
            return -1;
    }
    return 0;
}


int categoriseTablesRSDT(struct RSDT *rsdt){
    int entries = (rsdt->h.Length - sizeof(struct ACPISDTHeader)) / 4;

    for (int i = 0; i < entries; i++) {
        struct ACPISDTHeader *h = (struct ACPISDTHeader *) MMIO_PHYS_TO_VIRT(rsdt->PointerToOtherSDT[i]);
        if (handle_table(h) != 0) {
            return -1;
        }
    }
    return 0;
}





int handle_new_acpi(struct XSDP_t * xsdp) {
    if (Istrncmp(xsdp->Signature, SIGNATURE, sizeof(xsdp->Signature)) != 0)
        return -1;

    CHECKSUM( xsdp, 20);
    Iprintf("OEM: %s\n", xsdp->OEMID);
    int version = xsdp->Revision + 1;
    // ignore xsdp->RsdtAddress it's deprecated in all new versions

    CHECKSUM (xsdp, xsdp->Length)
    void * root_phys = (void *)  xsdp->XsdtAddress;
    if (map_mmio((phys_addr_t) root_phys, DEFAULT_PAGE_SIZE, WRITEABLE | CACHE_DISABLED) == FRAME_ALLOC_FAILED) {
        Iprintf("error while mapping mmio\n");
        return -1;
    }

    if (categoriseTablesXSDT((struct XSDT *) MMIO_PHYS_TO_VIRT(root_phys)) != 0) {
        Iprintf("categorise tables (failed checksum)\n");
        return -1;
    }
    return 0;
}

int handle_old_acpi(struct RSDP_t * rsdp) {
    if (Istrncmp(rsdp->Signature, SIGNATURE, sizeof(rsdp->Signature)) != 0)
        return -1;

    CHECKSUM(rsdp, 20);
    Iprintf("OEM: %s\n", rsdp->OEMID);
    int version = rsdp->Revision + 1;

    struct RSDT * root_phys = (struct RSDT *) (rsdp->RsdtAddress);
    virt_addr_t ret = map_mmio((phys_addr_t) root_phys, DEFAULT_PAGE_SIZE, WRITEABLE | CACHE_DISABLED);
    if (ret == FRAME_ALLOC_FAILED) {
        Iprintf("error while mapping mmio\n");
        return -1;
    }

    if (categoriseTablesRSDT((struct RSDT *) MMIO_PHYS_TO_VIRT(root_phys)) != 0) {
        Iprintf("categorise tables (failed checksum)\n");
        return -1;
    }
    discoverPCIE();
    return 0;
}