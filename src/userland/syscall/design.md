## Syscall User-ABI

### Instructions

Use the syscall instruction instead of interrupts bc of extra speed.\
Design a wrapper to restore state and pass args

### Stack PTR

syscall doesn't save the RSP so this will have to be done before syscall and restore right after.\
this obv can't use the stack.\
use MSR instead. swapgs, sysretq


## MSR: SYSCALL

Syscalls uses 4 MSRs
- IA32_EFER (0xC0000080): bit 0, SCE (syscall enable), must be set or syscall raises #UD.
- IA32_STAR (0xC0000081): segment selectors. Bits 47:32 are the kernel CS base (SS = that + 8). 
  - Bits 63:48 are the user base: on sysret, CS = base + 16 and SS = base + 8. This forces a particular GDT layout.
  - tho I might use iretq idk yet
- IA32_LSTAR (0xC0000082): the 64-bit entry point (RIP) that syscall jumps to.
- IA32_FMASK (0xC0000084): RFLAGS bits to clear on entry. Typically you clear IF so interrupts are off until you’re on a safe stack.
