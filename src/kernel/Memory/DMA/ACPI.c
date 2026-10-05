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
#include "ACPI_helpers.h"



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
        else if (!Istrncmp(h->Signature, "APIC", 4)) {
            //Imemccpy(&MCFG, h, h->Length);
            Iprintf("found APIC: %x\n", h);
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
        //Iprintf("Header: %s\n", h->Signature );
        if (!Istrncmp(h->Signature, "FACP", 4)){
            //Iprintf("found FADT\n");
            Imemccpy(&FACP, h, MIN(h->Length, sizeof(struct FADT)));
            CHECKSUM(&FACP, h->Length);
        }
        else if (!Istrncmp(h->Signature, "MCFG", 4)) {
            Imemccpy(&MCFG, h, h->Length);
            //Iprintf("found MCFG: %x\n", ((struct ACPI_MCFG *) h)->conf_space_addrs[0].segment_group_num);
            CHECKSUM(&MCFG, h->Length);
        }
        else if (!Istrncmp(h->Signature, "APIC", 4)) {
            Iprintf("found APIC(MADT): %x\n", h);
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
    Iprintf("discovering PCIE\n");

    // no MCFG table found, so read w/ the old regs
    if (MCFG.h.Signature[0] == '\0') {
        Iprintf("reading via legacy IO ports\n");
    }


    /*
     * Source for PCIE config space: https://support.microchip.com/s/article/What-is-PCIe-Config-Space
     */
    // brute force w/ ecam firmware / bios should've alr defined all
    for (int i = 0; i < MAX_MCFG_ENTRIES; ++i) {
        if (MCFG.conf_space_addrs[i].conf_base_addr == 0)
            continue;

        ECAM *ecam =  &MCFG.conf_space_addrs[i];
        uint64_t base =  ecam->conf_base_addr;
        uint64_t start_bus = ecam->start_pci;
        Iprintf("entering at MCFG[%i]; (%x) & (%x)\n", i, ecam->start_pci, ecam->end_pci);

        for (int bus = ecam->start_pci; bus < ecam->end_pci + 1; ++bus) {
            for (int dev_n = 0; dev_n < 32; ++dev_n) {
                // get the start addr of the device block
                volatile phys_addr_t dev_phys = (phys_addr_t)  ecam_addr(base, start_bus, bus, dev_n, 0, 0);
                volatile pci_hdr_common_t * dev = (pci_hdr_common_t *) map_mmio(dev_phys, DEFAULT_PAGE_SIZE, CACHE_DISABLED | WRITEABLE);

                uint16_t vendor = dev->vendor_id;
                if (vendor == 0xFFFF)
                    continue;       // no device here

                uint8_t header = dev->header_type;
                uint8_t functions = (header & 0x80) ? 8 : 1;        // bit 7 is multifunction bit

                for (int fn = 0; fn < functions; ++fn) {
                    dev_phys = (phys_addr_t)  ecam_addr(base, start_bus, bus, dev_n, fn, 0);
                    dev = (pci_hdr_common_t *) map_mmio(dev_phys, DEFAULT_PAGE_SIZE, CACHE_DISABLED | WRITEABLE);
                    uint8_t type = header & 0x3;
                    vendor = dev->vendor_id;
                    if (vendor == 0xFFFF)
                        continue;       // no device here

                    if (type == 0) {
                        volatile pci_hdr0_t * endpoint = (volatile pci_hdr0_t *) dev;

                        for (int bar = 0; bar < 6; ++bar) {
                            volatile uint32_t * active_bar = &endpoint->bar[bar];

                            // write all 1's to upper addr of BAR
                            // the register now contains the size after masking non addr bits
                            if (bar_is_64bit(active_bar)) {
                                uint64_t addr = decode_phys_BAR(active_bar);
                                active_bar[0] = 0xFFFFFFFF;
                                active_bar[1] = 0xFFFFFFFF;

                                uint64_t size = ~(decode_phys_BAR(active_bar) & ~0xF) + 1;
                                if (size != 0) {
                                    Iprintf("found PCI bar 64bit (%x)[%x] -> bus: (%x) dev: (%x)\n", bar, size, bus, dev_n);
                                }
                                encode_phys_BAR(active_bar, addr);
                                // increment bar by again bc 64 bit uses two
                                ++bar;
                                goto allocate;
                            }
                            uint32_t addr = active_bar[0];
                            active_bar[0] = 0xFFFFFFFF;
                            uint32_t size = ~(active_bar[0] & ~0xF) + 1;
                            if (size != 0) {
                                Iprintf("found PCI bar 32bit (%x)[%x] -> bus: (%x) dev: (%x)\n", addr, size, bus, dev_n);
                            }
                            active_bar[0] = addr;

                            allocate:
                                map_mmio(addr, size, WRITEABLE | CACHE_DISABLED);
                        }
                    }else {
                        Iprintf("config of Bridge-dev\n");
                    }
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