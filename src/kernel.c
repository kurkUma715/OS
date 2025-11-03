#include "vga.h"
#include "commands.h"
#include "conf.h"
#include <stdint.h>

static char name[32] = "root";

static void build_prompt(char *prompt, uint16_t size, const char *username, const char *current_path)
{
    uint16_t i = 0;

    prompt[i++] = '[';

    const char *n = username;
    while (*n && i < size - 1)
        prompt[i++] = *n++;

    prompt[i++] = '@';

    const char *sysname = "OS";
    const char *s = sysname;
    while (*s && i < size - 1)
        prompt[i++] = *s++;

    prompt[i++] = ' ';

    const char *path = current_path;
    while (*path && i < size - 1)
        prompt[i++] = *path++;

    prompt[i++] = ']';
    prompt[i++] = ' ';
    prompt[i] = 0;
}

void kmain(void)
{
    vga_init();
    vga_clear_screen();
    vga_enable_cursor();

    vga_print("Welcome to HuesOS (0.0.1)\n");

    for (;;)
    {
        static char prompt[128];
        char *current_path = "/";

        build_prompt(prompt, sizeof(prompt), name, current_path);

        command_prompt_and_readline(prompt);

        if (input_len == 0)
        {
            if (ctrl_c_pressed)
            {
                ctrl_c_pressed = false;
                continue;
            }

            if (ctrl_d_pressed)
            {
                ctrl_d_pressed = false;
                break;
            }

            continue;
        }

        if (command_dispatch(name))
            break;
    }
}