## GS Problem

- GS_kernel_base holds the kernel struct for syscalls
- GS_user_base can be any ordinary value that the user dictates.
- SWAPGS swaps gs between these two saved states

> if gs is swapped during a context-switch and then gs is swapped again 
> for whatever reason (2nd process syscalls or sim) then the gs will be missmatched
> This can lead to #Pf because it is #UB to access gs:addr with the user gs

### FIX

1: 
> find a way to store which base is active 0 (kernel) | 1 (user)
> save this in the thread struct
> use this information to swapgs to the needed one.
> Pro: uses swapgs which has to be supported
> Con: hard to keep track of state easy to introduce bugs


2:
> readgs and store it when entering the syscall
> set to kernel gs with rwgsbase during the syscall
> set back to earlier
> do this for every context switch
> easy to preserve state (direct set)