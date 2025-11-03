#include "vga.h"
#include <stdint.h>

static void delay(uint32_t ticks)
{
    for (volatile uint32_t i = 0; i < ticks * 100000; i++)
        __asm__ volatile("nop");
}

void kmain(void)
{
    vga_init();
    vga_clear_screen();
    vga_enable_cursor(13, 15);
    vga_update_cursor(0, 0);

    vga_print("Welcome to HuesOS (kernel 0.1)\n");
    vga_print("[root@HuesOS /] ");

    for (;;)
        __asm__ volatile("hlt");
}
