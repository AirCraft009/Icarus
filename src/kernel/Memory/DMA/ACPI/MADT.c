//
// Created by cocon on 06.10.2026.
//

#include "MADT.h"

#include "ACPI_helpers.h"
#include "kernel/util/shellio.h"


int handle_MADT(struct ACPISDTHeader *header) {
    MADT * madt = (MADT *) header;


    uint8_t *base = (uint8_t *)madt;
    uint8_t *end  = base + madt->h.Length;
    uint8_t *p    = base + 0x2C;

    while (((uint8_t *) p) + 2 <= end) {            // need at least type + length bytes
        uint8_t type = ((apic_tag_common *) p)->apic_entry_type;
        uint8_t len  = ((apic_tag_common *) p)->tag_size;

        if (len < 2 || p + len > end) // malformed entry, stop
            break;
        Iprintf("tag: %x\n", type);
        switch (type) {
            case 0:
                apic_tag_type0 * tag_t0 = (apic_tag_type0 *) p;
                Iprintf("PROCESSOR ID: %i\n", tag_t0->ACPI_proc_id);
                break;
            case 1:
                apic_tag_type1 * tag_t1 = (apic_tag_type1 *) p;
                Iprintf("IO ID: %i\n", tag_t1->IO_apic_id);
                break;
            case 2:
                apic_tag_type2 * tag_t2 = (apic_tag_type2 *) p;
                Iprintf("bus: %i\n", tag_t2->bus_source);
                break;
            case 3:
                apic_tag_type3 * tag_t3 = (apic_tag_type3 *) p;
                Iprintf("bus: %i\n", tag_t2->bus_source);
                break;
            case 4:
                apic_tag_type4 * tag_t4 = (apic_tag_type4 *) p;
                Iprintf("bus: %i\n", tag_t2->bus_source);
                break;
            case 5:
                apic_tag_type5 * tag_t5 = (apic_tag_type5 *) p;
                Iprintf("bus: %i\n", tag_t2->bus_source);
                break;
            case 9:
                // skip for now only relevant on machines w/ many cores
                break;
            default: break;
        }

        p += len;
    }
    return 0;
}
