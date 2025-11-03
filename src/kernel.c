#include "vga.h"
#include "commands.h"
#include "conf.h"
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

    vga_print("Welcome to HuesOS (0.0.1)\n");

    for (;;)
    {
        static char prompt[128] = "[root@HuesOS";
        command_strcpy(prompt, "[root@HuesOS");
        char *current_path = "/";

        uint16_t i = 12;
        prompt[i++] = ' ';
        while (*current_path && i < sizeof(prompt) - 3)
        {
            prompt[i++] = *current_path++;
        }
        prompt[i++] = ']';
        prompt[i++] = ' ';
        prompt[i] = 0;

        command_prompt_and_readline(prompt);

        if (input_len == 0 || ctrl_c_pressed)
        {
            continue;
        }

        if (command_strcmp(input_buffer, "help") == 0)
        {
            vga_print("help     - Show this help\n");
            vga_print("clear    - Clear screen\n");
            vga_print("reboot   - Reboot system\n");
            vga_print("shutdown - Shutdown system\n");
            continue;
        }
        else if (command_strcmp(input_buffer, "clear") == 0)
        {
            vga_clear_screen();
            continue;
        }
        else if (command_strcmp(input_buffer, "reboot") == 0)
        {
            command_do_reboot();
        }
        else if (command_strcmp(input_buffer, "shutdown") == 0)
        {
            command_do_shutdown();
        }
        else
        {
            vga_print("Unknown command: ");
            vga_print(input_buffer);
            vga_print("\n");
            continue;
        }
    }
}
