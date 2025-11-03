#include "commands.h"
#include "vga.h"
#include "keyboard.h"
#include "ports.h"
#include "ata.h"
#include "fat32.h"
#include <string.h>

char input_buffer[INPUT_BUFFER_SIZE] = {0};
uint16_t input_len = 0;
bool ctrl_c_pressed = false;
bool ctrl_d_pressed = false;

extern uint8_t terminal_row;
extern uint8_t terminal_col;

fat32_t fs;
fat32_dir_t current_dir;
char current_path[256] = "/";

char *command_strrchr(const char *s, int c)
{
    const char *last = NULL;
    while (*s)
    {
        if (*s == (char)c)
            last = s;
        s++;
    }
    return (char *)last;
}

int command_strcmp(const char *a, const char *b)
{
    for (;; a++, b++)
    {
        if (*a != *b)
            return (int)(uint8_t)*a - (int)(uint8_t)*b;
        if (!*a)
            return 0;
    }
}

void command_strcpy(char *dest, const char *src)
{
    while (*src)
    {
        *dest++ = *src++;
    }
    *dest = 0;
}

int command_strncmp(const char *a, const char *b, uint16_t n)
{
    for (uint16_t i = 0; i < n; i++)
    {
        if (a[i] != b[i])
            return (int)(uint8_t)a[i] - (int)(uint8_t)b[i];
        if (!a[i])
            return 0;
    }
    return 0;
}

void command_strcat(char *dest, const char *src)
{
    char *end = dest;
    while (*end)
        end++;
    while (*src)
        *end++ = *src++;
    *end = 0;
}

uint16_t command_strlen(const char *s)
{
    uint16_t len = 0;
    while (s[len] != '\0')
        len++;
    return len;
}

void command_do_reboot(void)
{
    asm volatile("cli");
    outb(0x64, 0xFE);
    for (;;)
    {
        asm volatile("hlt");
    }
}

void command_do_shutdown(void)
{
    outw(0x604, 0x2000);
    asm volatile("cli");
    for (;;)
    {
        asm volatile("hlt");
    }
}

void command_delay(uint32_t ticks)
{
    for (volatile uint32_t i = 0; i < ticks * 100000; i++)
        __asm__ volatile("nop");
}

bool command_dispatch(const char *name)
{
    if (command_strcmp(input_buffer, "help") == 0)
    {
        vga_print("help     - Show this help\n");
        vga_print("whoami   - Show your name\n");
        vga_print("delay <ms> - Delay by ms\n");
        vga_print("clear    - Clear screen\n");
        vga_print("lsblk    - List Partitions\n");
        vga_print("mount    - Mount partition\n");
        vga_print("ls       - File list\n");
        vga_print("cat      - Show source file\n");
        vga_print("cd       - Change directory\n");
        vga_print("reboot   - Reboot system\n");
        vga_print("shutdown - Shutdown system\n");
        vga_print("exit     - Exit from user\n");
        return false;
    }
    else if (command_strcmp(input_buffer, "whoami") == 0)
    {
        vga_print(name);
        vga_print("\n");
        return false;
    }
    else if (command_strncmp(input_buffer, "delay", 5) == 0)
    {
        const char *arg = input_buffer + 5;
        while (*arg == ' ')
            arg++;

        if (*arg)
        {
            uint32_t ticks = 0;
            while (*arg >= '0' && *arg <= '9')
            {
                ticks = ticks * 10 + (*arg - '0');
                arg++;
            }
            command_delay(ticks);
        }
        else
        {
            command_delay(1);
        }

        return false;
    }
    else if (command_strcmp(input_buffer, "clear") == 0)
    {
        vga_clear_screen();
        return false;
    }
    else if (command_strcmp(input_buffer, "reboot") == 0)
    {
        command_do_reboot();
        return true;
    }
    else if (command_strcmp(input_buffer, "shutdown") == 0)
    {
        command_do_shutdown();
        return true;
    }
    else if (command_strcmp(input_buffer, "exit") == 0)
    {
        return true;
    }
    else if (command_strcmp(input_buffer, "lsblk") == 0)
    {
        ata_lsblk();
        return false;
    }
    else if (command_strcmp(input_buffer, "mount") == 0)
    {
        fat32_mount(0, &fs);
        return false;
    }
    else if (command_strcmp(input_buffer, "ls") == 0)
    {
        fat32_list_dir(0, &fs, &current_dir);
        return false;
    }
    else if (command_strncmp(input_buffer, "cat ", 4) == 0)
    {
        fat32_cat(0, &fs, input_buffer + 4);
        return false;
    }
    else if (command_strncmp(input_buffer, "cd ", 3) == 0)
    {
        if (!fat32_cd(0, &fs, &current_dir, input_buffer + 3))
            vga_print("No such directory\n");
        else
            update_current_path(current_path, input_buffer + 3);
        return false;
    }
    else
    {
        vga_print("Unknown command: ");
        vga_print(input_buffer);
        vga_print("\n");
        return false;
    }
    return true;
}

void command_prompt_and_readline(const char *prompt)
{
    vga_print(prompt);
    uint8_t prompt_row = terminal_row;
    uint8_t prompt_col = terminal_col;

    input_len = 0;
    input_buffer[0] = 0;
    uint16_t cursor_pos = 0;
    ctrl_c_pressed = false;

    while (1)
    {
        KeyEvent event = keyboard_read_key();

        if (event.ctrl_c)
        {
            vga_print("^C\n");
            ctrl_c_pressed = true;
            input_len = 0;
            input_buffer[0] = 0;
            return;
        }

        if (event.ctrl_d && input_len == 0)
        {
            vga_print("exit\n");
            ctrl_d_pressed = true;
            input_len = 0;
            input_buffer[0] = 0;
            return;
        }

        if (event.ctrl_l)
        {
            vga_clear_screen();
            vga_print(prompt);
            for (uint16_t i = 0; i < input_len; i++)
            {
                vga_putchar(input_buffer[i]);
            }
            prompt_row = terminal_row;
            prompt_col = terminal_col - input_len;
            continue;
        }

        if (event.is_left_arrow)
        {
            if (cursor_pos > 0)
            {
                cursor_pos--;
                vga_update_cursor(prompt_row, prompt_col + cursor_pos);
            }
            continue;
        }

        if (event.is_right_arrow)
        {
            if (cursor_pos < input_len)
            {
                cursor_pos++;
                vga_update_cursor(prompt_row, prompt_col + cursor_pos);
            }
            continue;
        }

        if (event.character == '\n')
        {
            vga_putchar('\n');
            input_buffer[input_len] = 0;
            return;
        }

        if (event.character == '\b')
        {
            if (cursor_pos > 0 && input_len > 0)
            {
                for (uint16_t i = cursor_pos - 1; i < input_len - 1; i++)
                {
                    input_buffer[i] = input_buffer[i + 1];
                }
                input_len--;
                cursor_pos--;
                input_buffer[input_len] = 0;

                terminal_row = prompt_row;
                terminal_col = prompt_col;
                vga_print(input_buffer);
                vga_putchar(' ');

                terminal_row = prompt_row;
                terminal_col = prompt_col + cursor_pos;
                vga_update_cursor(terminal_row, terminal_col);
            }
            continue;
        }

        if (event.character && input_len < INPUT_BUFFER_SIZE - 1)
        {
            if (terminal_col >= VGA_COLS - 1)
            {
                continue;
            }

            for (uint16_t i = input_len; i > cursor_pos; i--)
            {
                input_buffer[i] = input_buffer[i - 1];
            }

            input_buffer[cursor_pos] = event.character;
            input_len++;
            cursor_pos++;
            input_buffer[input_len] = 0;

            terminal_row = prompt_row;
            terminal_col = prompt_col;
            vga_print(input_buffer);

            terminal_row = prompt_row;
            terminal_col = prompt_col + cursor_pos;
            vga_update_cursor(terminal_row, terminal_col);
        }
    }
}

void update_current_path(char *path, const char *newdir)
{
    if (command_strcmp(newdir, "/") == 0)
    {
        command_strcpy(path, "/");
        return;
    }
    if (command_strcmp(newdir, "..") == 0)
    {
        char *last = command_strrchr(path, '/');
        if (last && last != path)
            *last = '\0';
        else
            command_strcpy(path, "/");
        return;
    }

    if (command_strcmp(path, "/") != 0)
        command_strcat(path, "/");
    command_strcat(path, newdir);
}