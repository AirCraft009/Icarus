MB2_MAGIC  equ 0xE85250D6
MB2_ARCH   equ 0                     ; i386 (32-bit protected mode entry)

section .multiboot2 align=8
multiboot2_header_start:
    dd MB2_MAGIC
    dd MB2_ARCH
    dd multiboot2_header_end - multiboot2_header_start
    dd 0x100000000 - (MB2_MAGIC + MB2_ARCH + (multiboot2_header_end - multiboot2_header_start))

    ; Information request: memory map (6) and framebuffer info (8), optional
    align 8
    dw 1                ; type
    dw 1                ; flags: optional
    dd 16               ; size = 8 header + 2*4 requests
    dd 6
    dd 8

    ; Framebuffer request (omit this whole tag if you want BIOS text mode)
;    align 8
;    dw 5
;    dw 1                ; optional
;    dd 20
;    dd 1024
;    dd 768
;    dd 32

    ; End tag
    align 8
    dw 0
    dw 0
    dd 8
multiboot2_header_end:


extern kmain

CR0_PG     equ 0x80000000
CR4_PAE    equ 0x20
EFER       equ 0xC0000080
EFER_LME   equ 0x100

section .boot.text
bits 32
global _start

_start:
    cli
    mov edi, eax        ; magic -> RDI (1st arg); GRUB gives the magic in eax but we use eax to set cr3 cr4 etc
    mov esi, ebx        ; info  -> RSI (2nd arg); GRUB gives us the info we requested in the multiboot2 header but ebx is used

    ; Load the GDT
    lgdt [gdt_descriptor]

    ; CR3 = physical address of PML4
    mov eax, pml4
    mov cr3, eax

    ; Enable PAE
    mov eax, cr4
    or eax, CR4_PAE
    mov cr4, eax

    ; Enable Long Mode (EFER.LME)
    mov ecx, EFER
    rdmsr
    or eax, EFER_LME
    wrmsr

    ; Enable paging
    mov eax, cr0
    or eax, CR0_PG
    mov cr0, eax
    ; FINALLY IN LONG MODE (32-bit submode - compatability mode)

    ; Jump into 64-bit code segment
    jmp 0x08:long_mode

.text
bits 64

long_mode:
    ; Load data segment
    ; clean up any old / stale values
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov ss, ax

    mov rsp, stack_top
    xor ebp, ebp
    cld                 ; required by C


    mov edi, edi        ; clears edi in 64-bit mode bc of zero-extend (not sure lwk)
    mov esi, esi        ; same for RSI

    call kmain

hang:
    hlt
    jmp hang


section .boot.data

global pml4

align 4096
pml4:
    dq pdpt + 0x003             ; [0]   identity map
    times 510 dq 0
    dq pdpt + 0x003             ; [511] higher half

align 4096
pdpt:
    dq pd + 0x003               ; [0]
    times 509 dq 0
    dq pd + 0x003               ; [510]
    dq 0                        ; [511]

align 4096
pd:
    times 16 dq 0x00000083
    times 496 dq 0

align 4096
pt:
    times 512 dq 0


section .boot.rodata

align 8
gdt:
    dq 0x0000000000000000      ; null descriptor
    dq 0x00AF9A000000FFFF      ; 64-bit code
    dq 0x00CF92000000FFFF      ; data

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt - 1
    dd gdt

.data

.rodata

.boot.bss

section .bss
align 16
stack_bottom:
    resb 16384
stack_top:

