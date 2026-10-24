//
// Created by cocon on 23.10.2026.
//

#include "syscall.h"

#include "kernel_helper.h"
#include "kernel/util/cpuid_helpers.h"
#include "kernel/util/shellio.h"

int syscall_init() {
    if (!(CPUID_TABLE.features >> CPUID_FEAT_EDX_MSR & 0x1)) {
        Iprintf("MSR not supported: ICARUS cannot run\n");
        return -1;
    }

    cpuSetMSR(MSR_EFER, 0x1, 0x0 );     // set bit 0 (syscall enable) I hope you get why
    // [47:32] AND 0xFFFC is the cs selector for kernel space
    // [47:32] + 0x8 is the cs selector for kernel space
    // ([63:48] + 0x8) or 3 is ss user space
    // ([63:48] + 0x10) or 3 is cs user space
    wrmsr(MSR_STAR, (0x10ULL << 48) | (0x08ULL << 32));     // cs selector = 0x8; ss = 0x10;
    wrmsr(MSR_LSTAR, 0x0);                                 // entry point for the syscall
    wrmsr(MSR_FMASK, 0x2);                                  // disable IF flags so that syscall isn't interrupted
    return 0;
}

