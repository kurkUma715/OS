#include "vga.h"
#include "commands.h"
#include "conf.h"
#include "ata.h"
#include <stdint.h>
#include "fat32.h"

static char name[32] = "root";
static char host[32] = "HuesOS";

extern fat32_t fs;
extern fat32_dir_t current_dir;
extern char current_path[256];

static void build_prompt(char *prompt, uint16_t size, const char *username, const char *hostname, const char *current_path)
{
    uint16_t i = 0;

    prompt[i++] = '[';

    const char *n = username;
    while (*n && i < size - 1)
        prompt[i++] = *n++;

    prompt[i++] = '@';

    const char *s = hostname;
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
    vga_print("Kernel started\n");

    vga_clear_screen();
    vga_enable_cursor();

    ata_init();
    vga_print("Disks scanned.\n");

    fat32_mount(0, &fs);
    current_dir.cluster = fs.root_cluster;
    command_strcpy(current_dir.path, "/");

    vga_clear_screen();

    vga_print("Welcome to HuesOS (0.0.1)\n");

    for (;;)
    {
        static char prompt[128];
        extern char current_path[256];

        build_prompt(prompt, sizeof(prompt), name, host, current_dir.path);

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