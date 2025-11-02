#include "commands.h"
#include "vga.h"
#include "keyboard.h"
#include "ports.h"

char input_buffer[INPUT_BUFFER_SIZE];
uint16_t input_len = 0;

int command_strcmp(const char* a, const char* b) {
    for (;; a++, b++) {
        if (*a != *b) return (int)(uint8_t)*a - (int)(uint8_t)*b;
        if (!*a) return 0;
    }
}

void command_do_reboot(void) {
    asm volatile ("cli");
    outb(0x64, 0xFE);
    for (;; ) { asm volatile ("hlt"); }
}

void command_do_shutdown(void) {
    outw(0x604, 0x2000);
    asm volatile ("cli");
    for (;; ) { asm volatile ("hlt"); }
}

void command_prompt_and_readline(const char* prompt) {
    vga_print(prompt);
    uint8_t prompt_row = terminal_row;
    uint8_t prompt_col = terminal_col;

    input_len = 0;
    input_buffer[0] = 0;
    uint16_t cursor_pos = 0;

    vga_update_cursor(prompt_row, prompt_col);

    while (1) {
        bool left = false, right = false;
        char c = keyboard_read_key_char(&left, &right);

        if (left) {
            if (cursor_pos > 0) {
                cursor_pos--;
                vga_update_cursor(prompt_row, prompt_col + cursor_pos);
            }
            continue;
        }

        if (right) {
            if (cursor_pos < input_len) {
                cursor_pos++;
                vga_update_cursor(prompt_row, prompt_col + cursor_pos);
            }
            continue;
        }

        if (c == '\n') {
            vga_putchar('\n');
            input_buffer[input_len] = 0;
            return;
        }

        if (c == '\b') {
            if (cursor_pos > 0 && input_len > 0) {
                for (uint16_t i = cursor_pos - 1; i < input_len - 1; i++) {
                    input_buffer[i] = input_buffer[i + 1];
                }
                input_len--;
                cursor_pos--;
                input_buffer[input_len] = 0;

                for (uint16_t i = 0; i < input_len; i++) {
                    vga_write_char_at(prompt_row, prompt_col + i, input_buffer[i]);
                }
                vga_write_char_at(prompt_row, prompt_col + input_len, ' ');
                
                vga_update_cursor(prompt_row, prompt_col + cursor_pos);
            }
            continue;
        }

        if (c && input_len < INPUT_BUFFER_SIZE - 1) {
            if (prompt_col + input_len >= VGA_COLS - 1) {
                continue;
            }
            
            for (uint16_t i = input_len; i > cursor_pos; i--) {
                input_buffer[i] = input_buffer[i - 1];
            }

            input_buffer[cursor_pos] = c;
            input_len++;
            cursor_pos++;
            input_buffer[input_len] = 0;

            for (uint16_t i = 0; i < input_len; i++) {
                vga_write_char_at(prompt_row, prompt_col + i, input_buffer[i]);
            }

            vga_update_cursor(prompt_row, prompt_col + cursor_pos);
        }
    }
}