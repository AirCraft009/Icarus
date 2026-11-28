//
// Created by cocon on 23.10.2026.
//

#include "syscall.h"

#include "kernel_helper.h"
#include "../../../include/syscall_definition.h"
#include "kernel/Memory/alloc.h"
#include "kernel/Memory/memory_mapping.h"
#include "kernel/Memory/paging.h"
#include "kernel/Processes/Scheduling/scheduler.h"
#include "kernel/syscall/core_information_table.h"
#include "kernel/util/shellio.h"

extern void syscall_entry(void);
static lcore_ktable_t * ktable;

/**
 * initialises syscalls and the nec MSR regs for one core
 */
int syscall_init() {
    // temporary syscall initstack
    // shared between all process so first syscall can load
    void * sys_init_stack = Imalloc(DEFAULT_PAGE_SIZE);
    //read and patch instead of directly writing EFER.SCE as it can lead to an invalid 
    uint64_t efer = rmsr(MSR_EFER);
    efer |= 1ULL;                  // EFER.SCE
    wrmsr(MSR_EFER, efer);

    // [47:32] AND 0xFFFC is the cs selector for kernel space
    // [47:32] + 0x8 is the cs selector for kernel space
    // ([63:48] + 0x8) or 3 is ss user space
    // ([63:48] + 0x10) or 3 is cs user space
    wrmsr(MSR_STAR, (0x10ULL << 48) | (0x08ULL << 32));     // cs selector = 0x8; ss = 0x10;
    wrmsr(MSR_LSTAR, (uint64_t) syscall_entry);                                  // entry point for the syscall
    wrmsr(MSR_FMASK, 0x2);                                  // disable IF flags so that syscall isn't interrupted
    wrmsr(MSR_GSBASE, 0x0);                                 // GSBASE for the usr can be 0 on init per core
    wrmsr(MSR_FSBASE, 0x0);                                 // TODO: IMPLEMENT PER THREAD LEAVE EMPTY FOR NOW

    //KERNEL GSBASE
    // has scratch space & holds kernel rsp and usr rsp on syscall
    ktable = Imalloc(sizeof(lcore_ktable_t));
    ktable->selfPointer = (uint64_t)ktable;
    ktable->kernelStackTop = ((uint64_t) sys_init_stack) & ~0xFULL;
    wrmsr(MSR_KernelGSBase, ktable->selfPointer);
    return 0;
}

/**
 * writes data at 'buf' with length 'len' to a data_consumer 'output_id'
 */
long syscall_write(uint64_t data_id, uint8_t *buf, uint64_t len, uint64_t offset) {
    // for now js write to console and ignore the output_id

    write_str_len(buf, 0, len);
    return 0;
}
long syscall_exit_thread() {
    return 0;
}

long syscall_exit_process() {
    scheduler_kill_current_thread();
    return 0;
}

int syscall_check_init() {
    page_map_l4_entry * pml4 = (page_map_l4_entry *) KERNEL_PHYS_TO_VIRT(read_cr3());
    if
    (
        !is_mapped
        (
            pml4,
            SYSCALL_STACK_BOTTOM       // check if the syscall stack unique to each process is set up.
        )
    ){
        struct_page_t * pt = ialloc_frames(SYSCALL_STACK_PAGE_COUNT, SYSCALL_STACK_PAGE_SIZE);
        if (pt == (void *) FRAME_ALLOC_FAILED)
            return -1;

        for (int i = 0; i < pt->page_count; i++) {
            page_in
            (
                pml4,
                (void *) SYSCALL_STACK_BOTTOM + i * PAGE_TYPE_TO_SIZE(SYSCALL_STACK_PAGE_SIZE),
                 (void *) pt->page_addr[i],
                PAGE_TYPE_TO_SIZE(SYSCALL_STACK_PAGE_SIZE),
                PT_WRITEABLE
            );
        }
        ktable->kernelStackTop = ((uint64_t) SYSCALL_STACK) & ~0xFULL;
    }
    return 0;
}

long syscall_handler(uint64_t number, uint64_t first, uint64_t second,uint64_t third, uint64_t fourth, uint64_t fifth) {

    if (syscall_check_init() != 0)
        return 0x100;

    __asm__ volatile ("sti");
    switch (number) {
        case SYS_WRITE:
             return syscall_write(first, (uint8_t *) second, third, fourth);
        case SYS_EXIT_THREAD:
            return syscall_exit_thread();
        case SYS_EXIT_PROCESS:
            return syscall_exit_process();
        default:
            return 0x900;
    }
}

