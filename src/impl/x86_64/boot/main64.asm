global long_mode_start
extern kmain

section .text
bits 64

long_mode_start:
    mov ax, 0
    mov ss, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov rsp, stack_top

    call kmain

.hang:
    hlt
    jmp .hang

section .bss
align 16
stack_bottom64:
    resb 4096 * 4
stack_top: