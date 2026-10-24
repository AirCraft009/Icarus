global _syscall_entry
extern syscall_handler

user_rsp_scratch equ 0x0
kernel_rsp       equ 0x8

_syscall_entry:
    swapgs
    mov [gs:user_rsp_scratch], rsp
    mov rsp, [gs:kernel_rsp]

    push qword [gs:user_rsp_scratch]    ; user RSP
    push rcx                            ; user RIP
    push r11                            ; user RFLAGS
    push qword 0                        ; padding, keeps rsp 16-byte aligned

    ; caller: rax = number, rdi, rsi, rdx, r10, r8 = args 1-5
    ; C:      rdi = number, rsi, rdx, rcx, r8, r9 = args 1-5
    mov r9, r8
    mov r8, r10
    mov rcx, rdx
    mov rdx, rsi
    mov rsi, rdi
    mov rdi, rax

    call syscall_handler                ; result in rax

    add rsp, 8                          ; drop padding
    pop r11                             ; user RFLAGS
    pop rcx                             ; user RIP
    pop rsp                             ; user RSP

    swapgs
    o64 sysret