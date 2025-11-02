#include "vga.h"
#include "ports.h"

#define USED __attribute__((used))

static volatile uint16_t *const VGA_BUFFER = (uint16_t *)VGA_ADDRESS;
uint8_t terminal_row = 0;
uint8_t terminal_col = 0;
static uint8_t terminal_color = 0x0F;

void vga_init(void)
{
    terminal_row = 0;
    terminal_col = 0;
    terminal_color = 0x0F;
}

void vga_write_char_at(uint8_t row, uint8_t col, char c)
{
    if (row >= VGA_ROWS || col >= VGA_COLS)
        return;
    const uint64_t index = (uint64_t)row * VGA_COLS + col;
    VGA_BUFFER[index] = ((uint16_t)terminal_color << 8) | (uint8_t)c;
}

void vga_clear_screen(void)
{
    for (uint64_t i = 0; i < VGA_COLS * VGA_ROWS; i++)
    {
        VGA_BUFFER[i] = 0x0F00;
    }
    terminal_row = 0;
    terminal_col = 0;
}

void vga_enable_cursor(uint8_t cursor_start, uint8_t cursor_end)
{
    outb(0x3D4, 0x0A);
    outb(0x3D5, (inb(0x3D5) & 0xC0) | cursor_start);

    outb(0x3D4, 0x0B);
    outb(0x3D5, (inb(0x3D5) & 0xE0) | cursor_end);
}

void vga_update_cursor(uint8_t row, uint8_t col)
{
    uint16_t pos = row * VGA_COLS + col;

    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

void vga_putchar(char c)
{
    if (c == '\n')
    {
        terminal_col = 0;
        terminal_row++;
        if (terminal_row >= VGA_ROWS)
        {
            for (uint64_t y = 1; y < VGA_ROWS; y++)
            {
                for (uint64_t x = 0; x < VGA_COLS; x++)
                {
                    VGA_BUFFER[(y - 1) * VGA_COLS + x] = VGA_BUFFER[y * VGA_COLS + x];
                }
            }
            for (uint64_t x = 0; x < VGA_COLS; x++)
                VGA_BUFFER[(VGA_ROWS - 1) * VGA_COLS + x] = ((uint16_t)terminal_color << 8) | ' ';
            terminal_row = VGA_ROWS - 1;
        }
        vga_update_cursor(terminal_row, terminal_col);
        return;
    }

    if (c == '\b')
    {
        if (terminal_col > 0)
        {
            terminal_col--;
        }
        else if (terminal_row > 0)
        {
            terminal_row--;
            terminal_col = VGA_COLS - 1;
        }
        // Записываем пробел в текущую позицию
        vga_write_char_at(terminal_row, terminal_col, ' ');
        vga_update_cursor(terminal_row, terminal_col);
        return;
    }

    vga_write_char_at(terminal_row, terminal_col, c);
    terminal_col++;
    if (terminal_col >= VGA_COLS)
    {
        terminal_col = 0;
        terminal_row++;
    }
    if (terminal_row >= VGA_ROWS)
    {
        for (uint64_t y = 1; y < VGA_ROWS; y++)
        {
            for (uint64_t x = 0; x < VGA_COLS; x++)
            {
                VGA_BUFFER[(y - 1) * VGA_COLS + x] = VGA_BUFFER[y * VGA_COLS + x];
            }
        }
        for (uint64_t x = 0; x < VGA_COLS; x++)
            VGA_BUFFER[(VGA_ROWS - 1) * VGA_COLS + x] = ((uint16_t)terminal_color << 8) | ' ';
        terminal_row = VGA_ROWS - 1;
    }

    vga_update_cursor(terminal_row, terminal_col);
}

void vga_print(const char *s)
{
    for (uint64_t i = 0; s[i]; ++i)
        vga_putchar(s[i]);
}