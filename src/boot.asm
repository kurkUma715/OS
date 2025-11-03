section .multiboot
align 4
multiboot_header:
    dd 0x1BADB002
    dd 0x00000003
    dd -(0x1BADB002 + 0x00000003)

section .text
global _start
extern kmain

_start:
    cmp eax, 0x2BADB002
    jne no_multiboot
    
    mov esp, stack_top
    
    push ebx
    push eax
    call kmain
    
hang:
    cli
    hlt
    jmp hang

no_multiboot:
    mov dword [0xB8000], 0x4F424F4F
    cli
    hlt

section .bss
align 16
stack_bottom:
    resb 16384
stack_top: