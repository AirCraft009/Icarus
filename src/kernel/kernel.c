#include <stddef.h>
#include <stdint.h>

#include "kernel_helper.h"
#include "page_definitions.h"
#include "kernel/GDT/gdt.h"
#include "kernel/interrupts/APIC.h"
#include "kernel/interrupts/IDT.h"
#include "kernel/util/mem_utils.h"
#include "kernel/util/shellio.h"
#include "../../include/kernel/Memory/memory_mapping.h"
#include "kernel/util/multiboot2.h"
#include "kernel/util/cpuid_helpers.h"
#include "kernel/Memory/alloc.h"
#include "kernel/Memory/DMA/MADT.h"
#include "kernel/Memory/PMM/mem_map.h"

extern uint64_t *gdt_descriptor;


static page_map_l4_entry Kernel_PML4_TABLE[512] __attribute__((aligned(4096)));
static alloc_t * allocator;


void kmain(uint32_t magic, struct multiboot_info *mboot) {
    cursor *cur = init_shellio("ICARUS\n");

    // initialize the Interrupt Descriptor table
    idt_init();

    Imemset(&Kernel_PML4_TABLE[0], 0, sizeof(Kernel_PML4_TABLE));

    // call lgdt from high addr again (addr: 0x1088FD)
    gdt_init();
    //parse the struct given to use from the multiboot2 header
    //init the bitmap for free memory
    if (handle_multiboot2(magic, mboot, &Kernel_PML4_TABLE[0]) != 0) {
        Iprintf("init_multiboot2 failed, error while parsing mulitboot2 struct\n");
        __asm__ volatile ("cli; hlt"); // Completely hangs the computer
    };
    test_walk_table(&Kernel_PML4_TABLE[0], (phys_addr_t *) KERNEL_PHYS_TO_VIRT(0x0100000));
    Iprintf("RAAAAHH\n");
    write_cr3((uint64_t) KERNEL_VIRT_TO_PHYS(&Kernel_PML4_TABLE[0]));

    Iprintf("KERNEL SETUP CONCLUDED: %x\n", &Kernel_PML4_TABLE[0]);
    allocator = init_alloc(DEFAULT_PAGE_SIZE * 510, (void *) KERNEL_HEAP_ADDR);

    handle_ACPI();

    Iprintf("---------------MODEL-----------------\n");
    Iprintf("%s\n", init_cpuid());
    enable_apic();
    Iprintf("set up APIC\n");
    //__asm__ volatile ("sti");
    // cons_mprintf("KERNEL ENDING");
    lapic_write(0x380, 0xFFFFFFFF);
    Iprintf("Timer-start: (%x)\n", lapic_read(0x380));
    for (int i = 0; i < APIC_INFO->iso_count; ++i) {
        Iprintf("wiring: (%x - > %x)\n", APIC_INFO->isos[i].gsi, APIC_INFO->isos[i].source_irq);
    }

    while (1)
        ;
}
//235372403 = 256809072