ENTRY(_start)
SECTIONS
{
  . = 1M;  /* GRUB загружает ядро по 1Мб по умолчанию */

  .text : {
    *(.multiboot)
    *(.text*)
  }

  .rodata : { *(.rodata*) }
  .data   : { *(.data*) }
  .bss    : { *(.bss*) *(COMMON) }
}
