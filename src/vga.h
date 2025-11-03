#ifndef VGA_H
#define VGA_H

#include <stdint.h>

#define VGA_ADDRESS 0xB8000
#define VGA_COLS 80
#define VGA_ROWS 25

void vga_init(void);
void vga_clear_screen(void);
void vga_putchar(char c);
void vga_print(const char *s);
void vga_enable_cursor();
void vga_update_cursor(uint8_t row, uint8_t col);
void vga_write_char_at(uint8_t row, uint8_t col, char c);

extern uint8_t terminal_row;
extern uint8_t terminal_col;

#endif