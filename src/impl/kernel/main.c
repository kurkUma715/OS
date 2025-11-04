#include "print.h"

void kmain(void)
{
    print_clear();
    print_set_color(PRINT_COLOR_WHITE, PRINT_COLOR_BLACK);
    print_str("Welcome to HuesOS!\n");

    show_prompt();

    update_cursor();

    while (1)
    {
        keyboard_handler();
    }
}