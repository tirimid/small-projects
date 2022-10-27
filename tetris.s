    .global _start

    .data

hello_msg:  .asciz "hello world\n"
bye_msg:    .asciz "bye bye\n"

    .text

_start:
    mov $1, %rax
    mov $1, %rdi
    mov $hello_msg, %rsi
    mov $12, %rdx
    syscall

    mov $1, %rax
    mov $1, %rdi
    mov $bye_msg, %rsi
    mov $8, %rdx
    syscall
    
    mov $60, %rax
    xor %rdi, %rdi
    syscall
