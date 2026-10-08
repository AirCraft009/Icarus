//
// Created by cocon on 06.10.2026.
//

#include "kernel/Memory/DMA/MADT.h"

#include "kernel_helper.h"
#include "kernel/Memory/DMA/ACPI_helpers.h"
#include "kernel/Memory/alloc.h"
#include "kernel/Memory/memory_mapping.h"
#include "kernel/Memory/DMA/MMIO.h"
#include "kernel/util/mem_utils.h"

madt_info * APIC_INFO;


uint64_t handle_MADT(struct ACPISDTHeader *header) {
    MADT *madt = (MADT *)header;
    uint8_t *base = (uint8_t *)madt;
    uint8_t *end  = base + madt->h.Length;

    uint64_t lapic_addr = *(uint32_t *)(base + 0x24);
    uint32_t madt_flags = *(uint32_t *)(base + 0x28);

    uint32_t n_cpu = 0, n_io = 0, n_iso = 0, n_nmi = 0, n_lnmi = 0;

    // count on first pass so the size is unambiquous (looking at my MCFG)
    for (uint8_t *p = base + 0x2C; p + 2 <= end; ) {
        uint8_t type = p[0], len = p[1];
        if (len < 2 || p + len > end) break;            // malformed instr
        switch (type) {
            case 0: n_cpu++;  break;
            case 1: n_io++;   break;
            case 2: n_iso++;  break;
            case 3: n_nmi++;  break;
            case 4: n_lnmi++; break;
            default: break;
        }
        p += len;
    }

    size_t total = sizeof(madt_info)
                 + n_cpu  * sizeof(cpu_info)
                 + n_io   * sizeof(ioapic_info)
                 + n_iso  * sizeof(iso_info)
                 + n_nmi  * sizeof(nmi_info)
                 + n_lnmi * sizeof(lapic_nmi_info);

    //alocate space for the entire struct
    madt_info *info = (madt_info *)Imalloc(total);
    if (!info)
        return FRAME_ALLOC_FAILED;
    Imemset(info, 0, sizeof(total));

    uint8_t *cur = (uint8_t *)(info + 1);
    info->cpus       = (cpu_info *)cur;       cur += n_cpu  * sizeof(cpu_info);
    info->ioapics    = (ioapic_info *)cur;    cur += n_io   * sizeof(ioapic_info);
    info->isos       = (iso_info *)cur;       cur += n_iso  * sizeof(iso_info);
    info->nmis       = (nmi_info *)cur;       cur += n_nmi  * sizeof(nmi_info);
    info->lapic_nmis = (lapic_nmi_info *)cur;

    info->lapic_addr = lapic_addr;
    info->madt_flags = madt_flags;

    // obv fill the array and keep track of the count
    for (uint8_t *p = base + 0x2C; p + 2 <= end; ) {
        uint8_t type = p[0], len = p[1];
        if (len < 2 || p + len > end) break;

        switch (type) {
            case 0: {
                apic_tag_type0 *r = (apic_tag_type0 *)p;
                cpu_info *c = &info->cpus[info->cpu_count++];
                c->acpi_id = r->APIC_id; c->apic_id = r->APIC_id; c->flags = r->flags;
                break;
            }
            case 1: {
                apic_tag_type1 *r = (apic_tag_type1 *)p;
                ioapic_info *io = &info->ioapics[info->ioapic_count++];
                io->id = r->ACPI_IO_id; io->addr = r->IO_apic_id; io->gsi_base = r->GSIB;
                break;
            }
            case 2: {
                apic_tag_type2 *r = (apic_tag_type2 *)p;
                iso_info *o = &info->isos[info->iso_count++];
                o->bus = r->bus_source; o->source_irq = r->IRQ_source;
                o->gsi = r->GSI; o->flags = r->flags;
                break;
            }
            case 3: {
                apic_tag_type3 *r = (apic_tag_type3 *)p;
                nmi_info *n = &info->nmis[info->nmi_count++];
                n->gsi = r->GSI; n->flags = r->flags;
                break;
            }
            case 4: {
                apic_tag_type4 *r = (apic_tag_type4 *)p;
                lapic_nmi_info *n = &info->lapic_nmis[info->lapic_nmi_count++];
                n->acpi_id = r->ACPI_proc_id; n->lint = r->LINT; n->flags = r->flags;
                break;
            }
            case 5: {                                   // 64-bit LAPIC address replaces the 32-bit one
                apic_tag_type5 *r = (apic_tag_type5 *)p;
                info->lapic_addr = r->LAPIC_addr;
                break;
            }
            default: break;                             // 9 (x2APIC) etc. skipped for now
        }
        p += len;
    }

    map_mmio(info->lapic_addr, DEFAULT_PAGE_SIZE, WRITEABLE | CACHE_DISABLED);
    for (uint32_t i = 0; i < info->ioapic_count; ++i) {
        map_mmio(info->ioapics[i].addr, DEFAULT_PAGE_SIZE, WRITEABLE | CACHE_DISABLED);
    }
    APIC_INFO = info;
    return 0;
}
