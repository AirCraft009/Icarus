bits 32

; multiboot2 header copy-pasted

section .multiboot2
align 8

multiboot2_header_start:
    ; 1. Magic Number (Multiboot2)
    dd 0x15250A3F

    ; 2. Architecture (0 = 32-bit protected mode i386, 1 = 32-bit MIPS, etc.)
    ; Note: GRUB 2 typically expects 0 (i386) even for x86_64 kernels to initialize execution
    dd 0

    ; 3. Header Length
    dd multiboot2_header_end - multiboot2_header_start

    ; 4. Checksum
    dd -(0x15250A3F + 0 + (multiboot2_header_end - multiboot2_header_start))

    align 8
    dw 1                        ; Type: Information request
    dw 0                        ; Flags: None
    dd 12                       ; Size of this tag
    dd 6                        ; Request tag 6 (Framebuffer information)

    align 8
    dw 5                        ; Type: Framebuffer
    dw 0                        ; Flags: None
    dd 20                       ; Size of this tag
    dd 1024                     ; Width
    dd 768                      ; Height
    dd 32                       ; Depth (Bits per pixel)


    align 8
    dw 0                        ; Type: 0 (End)
    dw 0                        ; Flags: None
    dd 8                        ; Size: 8 bytes
multiboot2_header_end:

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