## Syscall User-ABI

### Instructions

Use the syscall instruction instead of interrupts bc of extra speed.\
Design a wrapper to restore state and pass args

### Stack PTR

syscall doesn't save the RSP so this will have to be done before syscall and restore right after.\
this obv can't use the stack.\
use MSR instead. swapgs, sysretq
