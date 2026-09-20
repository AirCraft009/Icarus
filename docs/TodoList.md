## Todo-List

### Boot
- Grub magic
  - gets to protected mode
  - paging is disabled
  - initRAMfs
  - loads boot.s
- boot.s switches to long mode
  - Set the PAE enable bit in CR4
  - Load CR3 with the physical address of the PML4 (Level 4 Page Map)
  - Enable long mode by setting the LME flag (bit 8) in MSR 0xC0000080 (aka EFER)
  - Enable paging
  - enter the main kernel
- KERNEL TIME
  - set up IDT
  - #PF handler
  - #GP handler
  - #UD handler
  - #DF handler
  - Physical memory allocator
  - Dynamic page-table mapper
  - Kernel heap
  - Timer interrupt
  - Keyboard interrupt
  - Basic scheduler
  - Ring 3
  - Syscalls
  - First user program