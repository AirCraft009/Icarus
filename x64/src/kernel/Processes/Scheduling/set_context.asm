global _switch_threads

_switch_threads:

    ; set_registers
    ;
    ; rdi has the ptr to the register struct
    ; layout:
    ;
    ;   starts: 0 bytes
    ;   uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    ;   : 64 bytes
    ;   uint64_t rbp, rdi, rsi, rdx, rcx, rbx, rax;
    ;
    ;   : 120 bytes
    ;   uint64_t rip
    ;   uint64_t cs
    ;   uint64_t rflags
    ;   uint64_t rsp
    ;   uint64_t ss
    ;   : 160 bytes
    ;

    push qword [rdi + 152]       ; ss
    push qword [rdi + 144]       ; rsp
    push qword [rdi + 136]       ; rflags (IF must be set for user threads)
    push qword [rdi + 128]       ; cs
    push qword [rdi + 120]       ; rip

    ; set registers to the values of the last execution
    mov r15, qword [rdi + 0]
    mov r14, qword [rdi + 8]
    mov r13, qword [rdi + 16]
    mov r12, qword [rdi + 24]
    mov r11, qword [rdi + 32]
    mov r10, qword [rdi + 40]
    mov r9,  qword [rdi + 48]
    mov r8,  qword [rdi + 56]
    mov rbp, qword [rdi + 64]
    mov rsi, qword [rdi + 80]
    mov rdx, qword [rdi + 88]
    mov rcx, qword [rdi + 96]
    mov rbx, qword [rdi + 104]
    mov rax, qword [rdi + 112]
    mov rdi, qword [rdi + 72]           ; restore rdi last, this destroys the ptr to the struct

    iretq





