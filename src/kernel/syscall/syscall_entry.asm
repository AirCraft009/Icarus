global _syscall_entry

usr_rsp_scratch equ 0
kernel_rsp 0x8

_syscall_entry:
    swapgs
    mov [gs:usr_rsp_scratch], rsp
    mov rsp, [gs:kernel_rsp]

    push rcx                        ; user RIP
    push r11                        ; user RFLAGS
    push qword [gs:user_rsp_scratch]

    ; user: rax = number, rdi, rsi, rdx, r10, r8 = args 1-5
    ; C:    rdi = number, rsi, rdx, rcx, r8, r9 = args 1-5
    mov r9, r8
    mov r8, r10
    mov rcx, rdx
    mov rdx, rsi
    mov rsi, rdi
    mov rdi, rax

    call syscall_handler            ; result in rax

    pop rdi                         ; user RSP
    pop r11
    pop rcx
    mov rsp, rdi
    swapgs
    o64 sysret
