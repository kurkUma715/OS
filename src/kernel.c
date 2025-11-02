#include "vga.h"
#include "keyboard.h"
#include "commands.h"

void kmain(void) {
    vga_clear_screen();
    vga_enable_cursor(13, 15);
    vga_update_cursor(0, 0);

    vga_print("Welcome to HuesOS!\n");

    for (;;) {
        command_prompt_and_readline("[root@test /] ");
        
        if (input_len == 0) {
            continue;
        }

        if (command_strcmp(input_buffer, "help") == 0) {
            vga_print("Idi naxuy!\n");
            continue;
        }
        if (command_strcmp(input_buffer, "clear") == 0) {
            vga_clear_screen();
            continue;
        }
        if (command_strcmp(input_buffer, "reboot") == 0) {
            vga_print("Rebooting...\n");
            command_do_reboot();
        }
        if (command_strcmp(input_buffer, "shutdown") == 0) {
            vga_print("Shutting down...\n");
            command_do_shutdown();
        }

        vga_print("incorrect command: ");
        vga_print(input_buffer);
        vga_print("\n");
    }
}