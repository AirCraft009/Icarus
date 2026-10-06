//
// Created by cocon on 06.10.2026.
//

#include "PCIE.h"

#include "ACPI_helpers.h"
#include "kernel/Memory/DMA/ACPI.h"
#include "kernel/Memory/DMA/MMIO.h"
#include "kernel/util/shellio.h"
#include "kernel_helper.h"
#include "kernel/Memory/memory_mapping.h"


/**
 * discovers the endpoints on the PCI/PCIE bus
 * using either the MCFG table from ACPI or the legacy IO Ports
 */
int discoverPCIE() {
    Iprintf("discovering PCIE: (%x)\n", &MCFG);

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
                                    //TODO: decode BARs and write to conf
                                    //Iprintf("found PCI bar 64bit (%x)[%x] -> bus: (%x) dev: (%x)\n", bar, size, bus, dev_n);
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
                                //TODO: decode BARs and write to conf
                                //Iprintf("found PCI bar 32bit (%x)[%x] -> bus: (%x) dev: (%x)\n", addr, size, bus, dev_n);
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