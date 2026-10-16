global _switch_threads

_switch_threads:

    ; set_registers
    ;
    ; rdi has the ptr to the tcb struct
    ; layout:
    ;
    ;typedef struct THREAD_CONTROL_BLOCK {
    ;   PID_t pid;                   // Process ID
    ;   PID_t parent_pid;            // Parent Process ID
    ;   TID_t tid;                   // thread ID
    ;
    ;   ---- offset 24 ------->
    ;   uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    ;   ---- offset 88 ------->
    ;   uint64_t rbp, rdi, rsi, rdx, rcx, rbx, rax;
    ;   ---- offset 144 ------>
    ;   uint64_t rip, cs, rflags, rsp, ss;
    ;
    ;    additional info ...
    ;}

    ; build the iretq frame straight from the TCB (push order: SS first, RIP last)
    mov word [abs 0xB8002], 0x2F41      ; white on green 'A', second cell

    push qword [rdi + 176]       ; ss
    push qword [rdi + 168]       ; rsp
    push qword [rdi + 160]       ; rflags (IF must be set for user threads)
    push qword [rdi + 152]       ; cs
    push qword [rdi + 144]       ; rip

    ; set registers to the value of last execution
    mov r15, qword [rdi + 24]
    mov r14, qword [rdi + 32]
    mov r13, qword [rdi + 40]
    mov r12, qword [rdi + 48]
    mov r11, qword [rdi + 56]
    mov r10, qword [rdi + 64]
    mov r9,  qword [rdi + 72]
    mov r8,  qword [rdi + 80]
    mov rbp, qword [rdi + 88]
    mov rsi, qword [rdi + 104]
    mov rdx, qword [rdi + 112]
    mov rcx, qword [rdi + 120]
    mov rbx, qword [rdi + 128]
    mov rax, qword [rdi + 136]
    mov rdi, qword [rdi + 96]           ; restore rdi last bc this destroys the ptr to tcb

    ; debug: write a white-on-red '!' to the top-left VGA cell
    mov word [abs 0xB8000], 0x4F21

    ; now switch
    iretq





