section .multiboot
align 4
multiboot_header:
    dd 0x1BADB002               ; Magic number
    dd 0x00000003               ; Flags
    dd -(0x1BADB002 + 0x00000003) ; Checksum

section .text
global _start
extern kmain

_start:
    ; Проверка multiboot
    cmp eax, 0x2BADB002
    jne no_multiboot
    
    ; Настройка стека
    mov esp, stack_top
    
    ; Вызов kernel main
    push ebx                    ; Multiboot info structure
    push eax                    ; Multiboot magic number
    call kmain
    
    ;Если kmain вернется
    cli
hang:
    hlt
    jmp hang

no_multiboot:
    mov dword [0xB8000], 0x4F424F4F  ; Вывести 'BOO' на экран (красный)
    cli
    hlt

section .bss
align 16
stack_bottom:
    resb 16384
stack_top: