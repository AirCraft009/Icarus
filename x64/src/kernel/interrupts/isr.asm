[BITS 64]

section .text

extern c_isr

; ISR stubs
;
; Collapse ISR into a common handler for a cleaner api
;   - I don't wanna have 256 separate handlers
;
;   error-code exceptions:
;       CPU pushed:  error_code
;       stub pushes: vector
;
;   non-error-code exceptions:
;       stub pushes: error_code = 0 (just so that the stack is the same, which is needed for the C function)
;       stub pushes: vector
;
; Then common_isr always sees:
;
;       [rsp + 0]  = vector
;       [rsp + 8]  = error code
;       [rsp + 16] = RIP
;       [rsp + 24] = CS
;       [rsp + 32] = RFLAGS
;       ...

%macro ISR_NOERR 1
global isr_%1
isr_%1:
    push qword 0              ; keep stack the same w/ dummy value
    push qword %1             ; interrupt vector
    jmp common_isr
%endmacro


%macro ISR_ERR 1
global isr_%1
isr_%1:
    push qword %1             ; interrupt vector
    jmp common_isr
%endmacro


; CPU exceptions 0-31

ISR_NOERR 0
ISR_NOERR 1
ISR_NOERR 2
ISR_NOERR 3
ISR_NOERR 4
ISR_NOERR 5
ISR_NOERR 6
ISR_NOERR 7

ISR_ERR   8
ISR_NOERR 9
ISR_ERR   10
ISR_ERR   11
ISR_ERR   12
ISR_ERR   13
ISR_ERR   14

ISR_NOERR 15
ISR_NOERR 16
ISR_ERR   17
ISR_NOERR 18
ISR_NOERR 19
ISR_NOERR 20
ISR_NOERR 21
ISR_NOERR 22
ISR_NOERR 23
ISR_NOERR 24
ISR_NOERR 25
ISR_NOERR 26
ISR_NOERR 27
ISR_NOERR 28
ISR_NOERR 29
ISR_ERR   30
ISR_NOERR 31


; Hardware/software vectors 32-255
;
; These don't get a CPU-pushed error code, so use NOERR.

%assign n 32
%rep 224
    ISR_NOERR n
%assign n n + 1
%endrep


; Common ISR entry
global common_isr

common_isr:

    ; Save all general-purpose registers.
    ; we restore them w/ pops in the reverse order later

    push rax
    push rcx
    push rdx
    push rbx
    push rbp
    push rsi
    push rdi
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    ; now push control regs bc it contains the faulted addr in case of #PF
    mov rax, cr2
    push rax
    mov rax, cr3
    push rax
    mov rax, cr4
    push rax




    lea rdi, [rsp]      ; arg 3 = CPU interrupt frame


    ; C function has the correct args on the stack(or ig the correct struct since it only takes a struct)
    call c_isr


    ; Restore registers
    pop rax     ; pop the stale cr2 reg
    pop rax     ; pop the stale cr3 reg
    pop rax     ; pop the stale cr4 reg

    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rdi
    pop rsi
    pop rbp
    pop rbx
    pop rdx
    pop rcx
    pop rax


    ; Remove:
    ;   vector
    ;   error code

    add rsp, 16

    iretq


; IDT stub table

section .rodata

align 8
global isr_table

isr_table:

%assign n 0
%rep 256
    dq isr_%+n
%assign n n + 1
%endrep