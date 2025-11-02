#include "vga.h"
#include "keyboard.h"
#include "commands.h"
#include "fs.h"

void kmain(unsigned long magic, unsigned long addr)
{
    if (magic != 0x2BADB002)
    {
        vga_print("Error: Invalid multiboot magic\n");
        return;
    }
    vga_init();
    vga_clear_screen();

    vga_enable_cursor(13, 15);
    vga_update_cursor(0, 0);

    vga_print("Initializing file system...\n");
    fs_init();

    vga_print("Welcome to HuesOS!\n");

    for (;;)
    {
        static char prompt[128] = "[root@huesiso";
        command_strcpy(prompt, "[root@huesiso");
        char *current_path = fs_get_current_path();

        uint16_t i = 13;
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
            vga_print("Available commands:\n");
            vga_print("help    - Show this help\n");
            vga_print("clear   - Clear screen\n");
            vga_print("ls      - List files\n");
            vga_print("cat     - Show file content\n");
            vga_print("touch   - Create file\n");
            vga_print("mkdir   - Create directory\n");
            vga_print("rm      - Remove file\n");
            vga_print("rmdir   - Remove directory\n");
            vga_print("cd      - Change directory\n");
            vga_print("pwd     - Show current directory\n");
            vga_print("reboot  - Reboot system\n");
            vga_print("shutdown - Shutdown system\n");
            continue;
        }

        if (command_strcmp(input_buffer, "uwu") == 0)
        {
            vga_print("Nyyaa~!\n");
            continue;
        }

        if (command_strcmp(input_buffer, "clear") == 0)
        {
            vga_clear_screen();
            continue;
        }

        if (command_strcmp(input_buffer, "reboot") == 0)
        {
            command_do_reboot();
            continue;
        }

        if (command_strcmp(input_buffer, "shutdown") == 0)
        {
            command_do_shutdown();
            continue;
        }

        if (command_strcmp(input_buffer, "exit") == 0)
        {
            command_do_shutdown();
            continue;
        }

        if (command_strncmp(input_buffer, "ls", 2) == 0)
        {
            const char *path = input_buffer + 2;
            while (*path == ' ')
                path++;
            if (*path == 0)
                path = fs_get_current_path();
            fs_list_directory(path);
            continue;
        }

        if (command_strncmp(input_buffer, "cat", 3) == 0)
        {
            const char *filename = input_buffer + 3;
            while (*filename == ' ')
                filename++;
            if (*filename == 0)
            {
                vga_print("Usage: cat <filename>\n");
            }
            else
            {
                fs_read_file(filename);
            }
            continue;
        }

        if (command_strncmp(input_buffer, "touch", 5) == 0)
        {
            const char *filename = input_buffer + 5;
            while (*filename == ' ')
                filename++;
            if (*filename == 0)
            {
                vga_print("Usage: touch <filename>\n");
            }
            else
            {
                if (fs_create_file(filename, ""))
                {
                    vga_print("File created: ");
                    vga_print(filename);
                    vga_print("\n");
                }
            }
            continue;
        }

        if (command_strncmp(input_buffer, "mkdir", 5) == 0)
        {
            const char *dirname = input_buffer + 5;
            while (*dirname == ' ')
                dirname++;
            if (*dirname == 0)
            {
                vga_print("Usage: mkdir <dirname>\n");
            }
            else
            {
                if (fs_create_directory(dirname))
                {
                    vga_print("Directory created: ");
                    vga_print(dirname);
                    vga_print("\n");
                }
            }
            continue;
        }

        if (command_strncmp(input_buffer, "rm", 2) == 0)
        {
            const char *filename = input_buffer + 2;
            while (*filename == ' ')
                filename++;
            if (*filename == 0)
            {
                vga_print("Usage: rm <filename>\n");
            }
            else
            {
                if (fs_remove_file(filename))
                {
                    vga_print("File removed: ");
                    vga_print(filename);
                    vga_print("\n");
                }
            }
            continue;
        }

        if (command_strncmp(input_buffer, "rmdir", 5) == 0)
        {
            const char *dirname = input_buffer + 5;
            while (*dirname == ' ')
                dirname++;
            if (*dirname == 0)
            {
                vga_print("Usage: rmdir <dirname>\n");
            }
            else
            {
                if (fs_remove_directory(dirname))
                {
                    vga_print("Directory removed: ");
                    vga_print(dirname);
                    vga_print("\n");
                }
            }
            continue;
        }

        if (command_strncmp(input_buffer, "cd", 2) == 0)
        {
            const char *path = input_buffer + 2;
            while (*path == ' ')
                path++;
            if (*path == 0)
                path = "/";

            if (fs_change_directory(path))
            {
                // Путь успешно изменен
            }
            continue;
        }

        if (command_strncmp(input_buffer, "pwd", 3) == 0)
        {
            vga_print(fs_get_current_path());
            vga_print("\n");
            continue;
        }

        vga_print("incorrect command: ");
        vga_print(input_buffer);
        vga_print("\n");
    }
}