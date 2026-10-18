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
#include "kernel/Drivers/PS2/PS2_Controller.h"
#include "kernel/Drivers/PS2/PS2_Keyboard.h"
#include "kernel/util/multiboot2.h"
#include "kernel/util/cpuid_helpers.h"
#include "kernel/Memory/alloc.h"
#include "kernel/Memory/paging.h"
#include "kernel/Memory/DMA/MADT.h"
#include "kernel/Memory/PMM/mem_map.h"
#include "kernel/Processes/thread.h"
#include "kernel/Processes/Scheduling/scheduler.h"

extern uint64_t *gdt_descriptor;


static page_map_l4_entry Kernel_PML4_TABLE[512] __attribute__((aligned(4096))) = {0};
static alloc_t * allocator;

// 20 bytes, position independent, no stack use
static const uint8_t fill_vga[] = {
    0xFC,                          // cld
    0xBF, 0x00, 0x80, 0x0B, 0x00,  // mov edi, 0xB8000
    0xB9, 0xE8, 0x03, 0x00, 0x00,  // mov ecx, 1000        (1000 dwords = 4000 bytes = 80x25x2)
    0xB8, 0xFF, 0xFF, 0xFF, 0xFF,  // mov eax, 0xFFFFFFFF
    0xF3, 0xAB,                    // rep stosd
    0xEB, 0xEC                     // jmp -20 (back to cld, refills forever)
};

static const uint8_t clear_vga[] = {
    0xFC,                          // cld
    0xBF, 0x00, 0x80, 0x0B, 0x00,  // mov edi, 0xB8000
    0xB9, 0xE8, 0x03, 0x00, 0x00,  // mov ecx, 1000        (1000 dwords = 4000 bytes = 80x25x2)
    0xB8, 0x20, 0x07, 0x20, 0x07,  // mov eax, 0x07200720  (two cells: ' ' + attribute 0x07)
    0xF3, 0xAB,                    // rep stosd
    0xEB, 0xEC                     // jmp -20 (back to cld, clears forever)
};


//235372403 = 256809072
void kmain(uint32_t magic, struct multiboot_info *mboot) {
    cursor *cur = shellio_init("ICARUS\n");

    // initialize the Interrupt Descriptor table
    idt_init();

    Imemset(&Kernel_PML4_TABLE[0], 0, sizeof(Kernel_PML4_TABLE));

    //parse the struct given to use from the multiboot2 header
    //init the bitmap for free memory
    paging_init();
    if (handle_multiboot2(magic, mboot, &Kernel_PML4_TABLE[0]) != 0) {
        Iprintf("init_multiboot2 failed, error while parsing mulitboot2 struct\n");
        __asm__ volatile ("cli; hlt"); // Completely hangs the computer
    };

    // call lgdt from high addr again (addr: 0x1088FD)
    gdt_init();


    write_cr3((uint64_t) KERNEL_VIRT_TO_PHYS(&Kernel_PML4_TABLE[0]));

    allocator = alloc_init(DEFAULT_PAGE_SIZE * 510, (void *) KERNEL_HEAP_ADDR);
    handle_ACPI();

    //.Iprintf("---------------MODEL-----------------\n");
    //Iprintf("%s\n", init_cpuid());
    enable_apic();
    enable_io_apics();
    //Iprintf("set up APIC\n");

    if (ps2_controller_init() != 0)
        Iprintf("The PS2 Controller doesn't work\n");

    Iprintf("Ticks per Ms: (%i)", LAPIC_TICKS_PER_MS);
    // TODO: TESTING BC OF THREADS
    page_in(Kernel_PML4_TABLE, (void *)0xB8000, (void *)0xB8000, DEFAULT_PAGE_SIZE, WRITEABLE | 1 << USER_ACCESS_SHIFT);

    tcb_t * thread1 = create_thread(0, false, 1);
    struct_page_t *addr = thread_supply_new_mem(thread1, SPACE_CODE, 2, DEFAULT_PAGE, WRITEABLE );
    //test_walk_table((void *)KERNEL_PHYS_TO_VIRT(thread1->page_table), (void *)KERNEL_PHYS_TO_VIRT(2 * HUGE_PS));
    Imemcpy((void *) addr, fill_vga, sizeof(fill_vga));
    Iprintf("addr: (%x)\n", addr);

    thread1->state = PROCESS_READY;
    scheduler_init(thread1);
    //Iprintf("rip: (%x)\n", thread1->regsiters.psp.rip);
    __asm__ volatile ("sti");
    keyboard_ps2_init();

    lapic_write(LAPIC_TIMER_INITCNT, 0x0);
    lapic_write(LAPIC_LVT_TIMER, 32 | LAPIC_TIMER_PERIODIC);
    lapic_write(LAPIC_TIMER_INITCNT, LAPIC_TICKS_PER_MS * 500);

    //TODO: build a table to hold all important subtables and system_statistics
    while (1)
        ;
}
