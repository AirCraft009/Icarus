bits 32

CR0_PG     equ 0x80000000
CR4_PAE    equ 0x20
EFER       equ 0xC0000080
EFER_LME   equ 0x100

section .text
global _start

_start:
    cli

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

    call icmain

hang:
    hlt
    jmp hang

section .bss
align 16
stack_bottom:
    resb 16384
stack_top:

section .data

align 4096
pml4:
    dq pdpt + 0x003
    times 511 dq 0

align 4096
pdpt:
    dq pd + 0x003
    times 511 dq 0

align 4096
pd:
    ; Present + writable + 2 MiB page
    dq 0x00000083
    times 511 dq 0


section .rodata

align 8
gdt:
    dq 0x0000000000000000      ; null descriptor
    dq 0x00AF9A000000FFFF      ; 64-bit code
    dq 0x00CF92000000FFFF      ; data

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt - 1
    dd gdt