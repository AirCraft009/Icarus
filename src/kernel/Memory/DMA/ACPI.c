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

#define CHECKSUM(buff, p)  \
    if(calculate_checksum((uint8_t *) (buff), (uint64_t) (p)) != 0){ \
        return -1; \
    }

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define SIGNATURE "RSD PTR "
#include "ACPI_tables.h"

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
static struct ACPI_MCFG MCFG = {0};

uint8_t calculate_checksum(uint8_t *buffer, uint64_t length) {
    uint8_t sum = 0;
    for (size_t i = 0; i < length; i++) {
        sum += buffer[i];
    }
    return sum; // Must be 0 if valid
}

int categoriseTablesXSDT(struct XSDT *xsdt)
{
    int entries = (xsdt->h.Length - sizeof(xsdt->h)) / 8;

    for (int i = 0; i < entries; i++) {
        struct ACPISDTHeader *h = (struct ACPISDTHeader *) MMIO_PHYS_TO_VIRT(xsdt->PointerToOtherSDT[i]);
        Iprintf("Header: %s\n", h->Signature );
        if (!Istrncmp(h->Signature, "FACP", 4)){
            Iprintf("found FADT\n");
            Imemccpy(&FACP, h, MIN(h->Length, sizeof(struct FADT)));
            CHECKSUM(&FACP, h->Length);
        }
        else if (!Istrncmp(h->Signature, "MCFG", 4)) {
            Imemccpy(&MCFG, h, h->Length);
            Iprintf("found MCFG: %x\n", ((struct ACPI_MCFG *) h)->conf_space_addrs[0].segment_group_num);
            CHECKSUM(&MCFG, h->Length);
        }
        // TODO: expand for other Tables
    }
    return 0;
}

int categoriseTablesRSDT(struct RSDT *rsdt)
{
    int entries = (rsdt->h.Length - sizeof(struct ACPISDTHeader)) / 4;

    for (int i = 0; i < entries; i++) {
        struct ACPISDTHeader *h = (struct ACPISDTHeader *) MMIO_PHYS_TO_VIRT(rsdt->PointerToOtherSDT[i]);
        Iprintf("Header: %s\n", h->Signature );
        if (!Istrncmp(h->Signature, "FACP", 4)){
            Iprintf("found FADT\n");
            Imemccpy(&FACP, h, MIN(h->Length, sizeof(struct FADT)));
            CHECKSUM(&FACP, h->Length);
        }
        else if (!Istrncmp(h->Signature, "MCFG", 4)) {
            Imemccpy(&MCFG, h, h->Length);
            Iprintf("found MCFG: %x\n", ((struct ACPI_MCFG *) h)->conf_space_addrs[0].segment_group_num);
            CHECKSUM(&MCFG, h->Length);
        }
        // TODO: expand for other Tables
    }
    return 0;
}


/**
 * discovers the endpoints on the PCI/PCIE bus
 * using either the MCFG table from ACPI or the legacy IO Ports
 */
int discoverPCIE() {
    // no MCFG table found, so read w/ the old regs
    if (MCFG.h.Signature[0] == '\0') {

    }


    /*
     * Source for PCIE config space: https://support.microchip.com/s/article/What-is-PCIe-Config-Space
     */
    // brute force w/ ecam firmware / bios should've alr defined all
    for (int i = 0; i < MAX_MCFG_ENTRIES; ++i) {
        if (MCFG.conf_space_addrs[i].conf_base_addr == 0)
            continue;

        ECAM *ecam =  &MCFG.conf_space_addrs[i];
        uint64_t base = ecam->conf_base_addr;
        uint64_t start_bus = ecam->start_pci;

        for (int bus = ecam->start_pci; bus < ecam->end_pci; ++bus) {
            for (int dev_n = 0; dev_n < 32; ++dev_n) {
                // get the start addr of the device block
                volatile pci_hdr_common_t * dev = (pci_hdr_common_t *) ecam_addr(base, start_bus, bus, dev_n, 0, 0);
                uint16_t vendor = dev->vendor_id;
                if (vendor == 0xFFFF)
                    continue;       // no device here

                uint8_t header = dev->header_type;
                uint8_t functions = (header & 0x80) ? 8 : 1;        // bit 7 is multifunction bit

                for (int fn = 0; fn < functions; ++fn) {

                }
            }
        }
    }

    return 0;
}


int handle_new_acpi(struct XSDP_t * xsdp) {
    if (Istrncmp(xsdp->Signature, SIGNATURE, sizeof(xsdp->Signature)) != 0)
        return -1;

    // checksum for RSDP (first 20 bytes)
    CHECKSUM( xsdp, 20);

    Iprintf("OEM: %s\n", xsdp->OEMID);
    int version = xsdp->Revision + 1;
    // ignore xsdp->RsdtAddress it's deprecated in all new versions

    //checksum for XsdP (all bytes)
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

    // checksum for RSDP (first 20 bytes)
    CHECKSUM(rsdp, 20);

    Iprintf("OEM: %s\n", rsdp->OEMID);
    int version = rsdp->Revision + 1;

    struct RSDT * root_phys = (struct RSDT *) (rsdp->RsdtAddress);
    if (map_mmio((phys_addr_t) root_phys, DEFAULT_PAGE_SIZE, WRITEABLE | CACHE_DISABLED) == FRAME_ALLOC_FAILED) {
        Iprintf("error while mapping mmio\n");
        return -1;
    }

    if (categoriseTablesRSDT((struct RSDT *) MMIO_PHYS_TO_VIRT(root_phys)) != 0) {
        Iprintf("categorise tables (failed checksum)\n");
        return -1;
    }

    return 0;
}