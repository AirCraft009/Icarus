//
// Created by cocon on 23.10.2026.
//

#include "syscall.h"

#include "kernel_helper.h"
#include "syscall_definition.h"
#include "kernel/Memory/alloc.h"
#include "kernel/syscall/core_information_table.h"
#include "kernel/util/shellio.h"

/**
 * initialises syscalls and the nec MSR regs for one core
 */
int syscall_init() {
    cpuSetMSR(MSR_EFER, 0x1, 0x0 );     // set bit 0 (syscall enable) I hope you get why
    // [47:32] AND 0xFFFC is the cs selector for kernel space
    // [47:32] + 0x8 is the cs selector for kernel space
    // ([63:48] + 0x8) or 3 is ss user space
    // ([63:48] + 0x10) or 3 is cs user space
    wrmsr(MSR_STAR, (0x10ULL << 48) | (0x08ULL << 32));     // cs selector = 0x8; ss = 0x10;
    wrmsr(MSR_LSTAR, 0x0);                                  // entry point for the syscall
    wrmsr(MSR_FMASK, 0x2);                                  // disable IF flags so that syscall isn't interrupted
    wrmsr(MSR_GSBASE, 0x0);                                 // GSBASE for the usr can be 0 on init per core
    wrmsr(MSR_FSBASE, 0x0);                                 // TODO: IMPLEMENT PER THREAD LEAVE EMPTY FOR NOW

    //KERNEL GSBASE
    // has scratch space & holds kernel rsp and usr rsp on syscall
    lcore_ktable_t * ktable = Imalloc(sizeof(lcore_ktable_t));
    ktable->selfPointer = (uint64_t)ktable;
    wrmsr(MSR_KernelGSBase, ktable->selfPointer);
    return 0;
}

/**
 * writes data at 'buf' with length 'len' to a data_consumer 'output_id'
 */
long syscall_write(uint64_t output_id, uint8_t *buf, uint64_t len, uint64_t offset) {
    // for now js write to console and ignore the output_id
    write_str_len(buf, offset, len);
    return 0;
}
long syscall_exit_thread() {
    Iprintf("exit");
    return 0;
}

long syscall_exit_process() {
    Iprintf("exit process");
    return 0;
}

long syscall_handler(uint64_t number, uint64_t first, uint64_t second,uint64_t third, uint64_t fourth, uint64_t fifth) {
    switch (number) {
        case SYS_WRITE:
             return syscall_write(first, (uint8_t *) second, third, fourth);
        case SYS_EXIT_THREAD:
            return syscall_exit_thread();
        case SYS_EXIT_PROCESS:
            return syscall_exit_process();
        default:
            return -1;
    }
}

