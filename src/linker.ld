ENTRY(_start)
KERNEL_VMA = 0xFFFFFFFF80000000;

/*
 Set the code running in protected mode to virtual addr 1M
 */

SECTIONS {
  . = 1M;
  .boot : {
      KEEP(*(.multiboot2))
      *(.boot.text)
      *(.boot.data)
      *(.boot.bss)
      *(.boot.rodata)
  }


/*
 Set the code running in long mode( the long jump + C kernel)
 to the virtual addr 0xFFFFFFFF80000000
 !!! THEY'RE STILL LOCATED AT .1M + IN MEMORY !!!!!
 !!! IT'S ONLY VIRTUALLY (Virtual addr space) LOCATED SOMEWHERE ELSE !!!
 !!! THEN TRANSLATED TO THE PHYSICAL LOW ADDR !!!
 */
    . = ALIGN(4096);
    . += KERNEL_VMA;

    .text : AT(ADDR(.text) - KERNEL_VMA) { *(.text .text.*) }
    . = ALIGN(4096);
    .rodata : AT(ADDR(.rodata) - KERNEL_VMA) { *(.rodata .rodata.*) }
    . = ALIGN(4096);
    .data : AT(ADDR(.data) - KERNEL_VMA) { *(.data .data.*) }
    . = ALIGN(4096);
    .bss : AT(ADDR(.bss) - KERNEL_VMA) {
      __bss_start = .;
      *(COMMON)
      *(.bss .bss.*)
      __bss_end = .;
    }

    . = ALIGN(4096);
    _kernel_end = .;

    /DISCARD/ : { *(.comment) *(.eh_frame) *(.note*) }
}



OUTPUT_FORMAT(elf64-x86-64)
OUTPUT(icarus_kernel.bin)