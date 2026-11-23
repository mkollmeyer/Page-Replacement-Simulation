section .rodata
    msg     db "Hello, World", 10
    msglen  equ $ - msg

section .text
    global _start

_start:
    mov     rbx, 5      ; loop counter
print_loop:
    mov     rax, 1      ; Syscall write()
    mov     rdi, 1      ; File descriptor: stdout
    mov     rsi, msg    ; Address of message
    mov     rdx, msglen ; Length of message
    syscall

    dec     rbx
    jnz     print_loop  ; repeat the loop

    mov     rax, 60     ; Syscall exit()
    xor     rdi, rdi    ; Exit status 0
    syscall
