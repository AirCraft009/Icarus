global _set_registers

_set_registers:
    ; edi has the ptr to the tcb struct
    ; layout:
    ;
    ;typedef struct THREAD_CONTROL_BLOCK {
    ;   PID_t pid;                   // Process ID
    ;   PID_t parent_pid;            // Parent Process ID
    ;   TID_t tid;                   // thread ID
    ;
    ;   ---- offset 24 ------->
    ;   uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    ;   uint64_t rbp, rdi, rsi, rdx, rcx, rbx, rax;
    ;   uint64_t rip, cs, rflags, rsp, ss;
    ;
    ;    additional info ...
    ;}

    mov r15, qword [rdi + 24]
    mov r14, qword [rdi + 32]
    mov r13, qword [rdi + 40]
    mov r12, qword [rdi + 48]
    mov r11, qword [rdi + 56]
    mov r10, qword [rdi + 64]
    mov r9,  qword [rdi + 72]
    mov r8,  qword [rdi + 80]
    mov rbp, qword [rdi + 88]
    mov rdi, qword [rdi + 96]
    mov rsi, qword [rdi + 104]
    mov rdx, qword [rdi + 112]
    mov rcx, qword [rdi + 120]
    mov rbx, qword [rdi + 128]
    mov rax, qword [rdi + 136]
    ; ignore rip

