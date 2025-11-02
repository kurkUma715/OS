section .multiboot2_header
header_start:
    dd 0xe85250d6                ; Magic number
    dd 0                         ; Architecture 0 (i386)
    dd header_end - header_start ; Header length
    dd 0x100000000 - (0xe85250d6 + 0 + (header_end - header_start)) ; Checksum
    
    ; End tag
    dw 0    ; Type
    dw 0    ; Flags  
    dd 8    ; Size
header_end:

section .text
global _start
_start:
    ; Set up stack
    mov esp, stack_top
    
    ; Call kernel main
    extern kmain
    call kmain

.hang:
    hlt
    jmp .hang

section .bss
align 16
stack_bottom:
    resb 16384
stack_top: