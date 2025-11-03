#include "commands.h"
#include "vga.h"
#include "keyboard.h"
#include "ports.h"
#include "fat32.h"
#include "ata.h"

char input_buffer[INPUT_BUFFER_SIZE] = {0};
uint16_t input_len = 0;
bool ctrl_c_pressed = false;

extern uint8_t terminal_row;
extern uint8_t terminal_col;

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
            command_do_shutdown();
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
