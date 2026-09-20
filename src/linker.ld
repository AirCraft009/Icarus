ENTRY(_start)
SECTIONS {
   . = 1M;
  .boot : {KEEP(*(.multiboot2))}
  .text : { *(.text);*(.text.*) }
  .data : ALIGN(4096) { *(.data); *(.data.*) }
  .rodata : { *(.rodata); *(.rodata.*) }
  .bss : ALIGN(4096) {
      __bss_start = .;
      *(COMMON)
      *(.bss .bss.*)
      __bss_end = .;
  }


    /DISCARD/ : { *(.comment) *(.eh_frame) *(.note*) }
}


OUTPUT_FORMAT(elf64-x86-64)
OUTPUT(icarus_kernel.bin)

