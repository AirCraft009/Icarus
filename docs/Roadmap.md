## Roadmap

### Boot
- Grub magic
  - gets to protected mode
  - paging is disabled
  - loads boot.s
- boot.s switches to long mode
  - Set the PAE enable bit in CR4
  - Load CR3 with the physical address of the PML4 (Level 4 Page Map)
  - Enable long mode by setting the LME flag (bit 8) in MSR 0xC0000080 (aka EFER)
  - Enable paging
  - enter the main kernel

## KERNEL TIME
  - set up IDT
  - #PF handler
  - #GP handler
  - #UD handler
  - #DF handler
  - Physical memory allocator
  - Dynamic page-table mapper
  - Kernel heap
  - Create a global config table over which all others are reachable.
    - name: MRT (Master Record Table)


## I/O
  - Timer interrupt
  - Keyboard interrupt
  - Basic scheduler
  - Ring 3
  - Syscalls
    - Cross-compiler: https://wiki.osdev.org/GCC_Cross-Compiler#Why_cross-compilers_are_necessary
  - First user program
  - read from a storage device
    

## Threads & Multithreading
  - expand threading to multiple CPU cores
  - make process and thread creation api better
    - find a thread via it's tid
    - find a process through it's pid
    - process should contain a list of:
      - all threads 
      - mappings 
  - work on signal handling
  - create a unified mapping layer w/ a clean format
    - all programs can get mappings
    - mappings are passed to each thread
  - work in interrupts for IO-APIC devs 

