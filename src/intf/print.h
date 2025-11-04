#ifndef PRINT_H
#define PRINT_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define PRINT_COLOR_BLACK 0
#define PRINT_COLOR_BLUE 1
#define PRINT_COLOR_GREEN 2
#define PRINT_COLOR_CYAN 3
#define PRINT_COLOR_RED 4
#define PRINT_COLOR_MAGENTA 5
#define PRINT_COLOR_BROWN 6
#define PRINT_COLOR_LIGHT_GRAY 7
#define PRINT_COLOR_DARK_GRAY 8
#define PRINT_COLOR_LIGHT_BLUE 9
#define PRINT_COLOR_LIGHT_GREEN 10
#define PRINT_COLOR_LIGHT_CYAN 11
#define PRINT_COLOR_LIGHT_RED 12
#define PRINT_COLOR_PINK 13
#define PRINT_COLOR_YELLOW 14
#define PRINT_COLOR_WHITE 15

extern size_t col;
extern size_t row;
extern uint8_t color;

void print_clear();
void print_newline();
void print_char(char character);
void print_str(char *str);
void print_set_color(uint8_t foreground, uint8_t background);

void update_cursor();

void keyboard_handler();
void process_input(char c);
void execute_command(char *command);
void show_prompt();
void move_cursor_left();
void move_cursor_right();
void clear_input_line();

#endif