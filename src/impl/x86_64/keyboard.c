#include "print.h"
#include "ports.h"
#include "string.h"
#include "power.h"

#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64
#define MAX_INPUT_LENGTH 256
#define MAX_HISTORY 10

static const char scancode_lower_case[128] = {
    0,
    0,
    '1',
    '2',
    '3',
    '4',
    '5',
    '6',
    '7',
    '8',
    '9',
    '0',
    '-',
    '=',
    0,
    0,
    'q',
    'w',
    'e',
    'r',
    't',
    'y',
    'u',
    'i',
    'o',
    'p',
    '[',
    ']',
    0,
    0,
    'a',
    's',
    'd',
    'f',
    'g',
    'h',
    'j',
    'k',
    'l',
    ';',
    '\'',
    '`',
    0,
    '\\',
    'z',
    'x',
    'c',
    'v',
    'b',
    'n',
    'm',
    ',',
    '.',
    '/',
    0,
    '*',
    0,
    ' ',
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
};

static const char scancode_upper_case[128] = {
    0,
    0,
    '!',
    '@',
    '#',
    '$',
    '%',
    '^',
    '&',
    '*',
    '(',
    ')',
    '_',
    '+',
    0,
    0,
    'Q',
    'W',
    'E',
    'R',
    'T',
    'Y',
    'U',
    'I',
    'O',
    'P',
    '{',
    '}',
    0,
    0,
    'A',
    'S',
    'D',
    'F',
    'G',
    'H',
    'J',
    'K',
    'L',
    ':',
    '"',
    '~',
    0,
    '|',
    'Z',
    'X',
    'C',
    'V',
    'B',
    'N',
    'M',
    '<',
    '>',
    '?',
    0,
    '*',
    0,
    ' ',
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
};

static int shift_pressed = 0;
static int caps_lock = 0;
static int extended_scancode = 0;

extern size_t col;
extern size_t row;
extern uint8_t color;

static char input_buffer[MAX_INPUT_LENGTH];
static uint16_t input_index = 0;
static uint16_t cursor_position = 0;
static bool input_mode = true;

static char command_history[MAX_HISTORY][MAX_INPUT_LENGTH];
static int history_index = 0;
static int history_size = 0;
static int current_history_pos = -1;

static char user[32] = "root";
static char host[32] = "HuesOS";
static char current_path[256] = "/";

size_t get_prompt_start_col()
{
    size_t prompt_length = 1;

    const char *n = user;
    while (*n)
    {
        prompt_length++;
        n++;
    }

    prompt_length += 1;

    const char *h = host;
    while (*h)
    {
        prompt_length++;
        h++;
    }

    prompt_length += 1;

    const char *p = current_path;
    while (*p)
    {
        prompt_length++;
        p++;
    }

    prompt_length += 2;

    return prompt_length;
}

void redraw_input_line()
{
    size_t prompt_start = get_prompt_start_col();

    size_t current_row = row;

    col = prompt_start;
    update_cursor();

    for (int i = 0; i < input_index; i++)
    {
        print_char(input_buffer[i]);
    }

    int remaining = 80 - prompt_start - input_index;
    for (int i = 0; i < remaining; i++)
    {
        print_char(' ');
    }

    col = prompt_start + cursor_position;
    update_cursor();
}

void process_input(char c)
{
    if (!input_mode || input_index >= MAX_INPUT_LENGTH - 1)
        return;

    if (cursor_position < input_index)
    {
        for (int i = input_index; i > cursor_position; i--)
        {
            input_buffer[i] = input_buffer[i - 1];
        }
    }

    input_buffer[cursor_position] = c;
    input_index++;
    cursor_position++;
    input_buffer[input_index] = '\0';

    redraw_input_line();
}

void move_cursor_left()
{
    if (cursor_position > 0)
    {
        cursor_position--;
        size_t prompt_start = get_prompt_start_col();
        col = prompt_start + cursor_position;
        update_cursor();
    }
}

void move_cursor_right()
{
    if (cursor_position < input_index)
    {
        cursor_position++;
        size_t prompt_start = get_prompt_start_col();
        col = prompt_start + cursor_position;
        update_cursor();
    }
}

void clear_input_line()
{
    col = get_prompt_start_col();
    update_cursor();

    for (int i = 0; i < 80 - col; i++)
    {
        print_char(' ');
    }

    col = get_prompt_start_col();
    update_cursor();
}

void add_to_history(char *command)
{
    if (command[0] == '\0')
        return;

    for (int i = MAX_HISTORY - 1; i > 0; i--)
    {
        strcpy(command_history[i], command_history[i - 1]);
    }

    strcpy(command_history[0], command);

    if (history_size < MAX_HISTORY)
    {
        history_size++;
    }
}

void show_history_prev()
{
    if (current_history_pos < history_size - 1)
    {
        current_history_pos++;
        strcpy(input_buffer, command_history[current_history_pos]);
        input_index = strlen(input_buffer);
        cursor_position = input_index;

        redraw_input_line();
    }
}

void show_history_next()
{
    if (current_history_pos > 0)
    {
        current_history_pos--;
        strcpy(input_buffer, command_history[current_history_pos]);
        input_index = strlen(input_buffer);
        cursor_position = input_index;

        redraw_input_line();
    }
    else if (current_history_pos == 0)
    {
        current_history_pos = -1;
        input_buffer[0] = '\0';
        input_index = 0;
        cursor_position = 0;
        clear_input_line();
    }
}

void execute_command(char *command)
{
    if (command[0] == 0)
    {
        print_newline();
        show_prompt();
        return;
    }

    print_newline();

    add_to_history(command);

    if (strcmp(command, "help") == 0)
    {
        print_str("Available commands:\n");
        print_str("  help    - Show this help\n");
        print_str("  clear   - Clear screen\n");
        print_str("  about   - About HuesOS\n");
        print_str("  echo    - Echo text\n");
        print_str("  history - Show command history\n");
        print_str("  shutdown  - Shutdown system\n");
        print_str("  reboot    - Reboot system\n");
        print_str("  exit      - Exit to reboot\n");
    }
    else if (strcmp(command, "clear") == 0)
    {
        print_clear();
        show_prompt();
        return;
    }
    else if (strcmp(command, "about") == 0)
    {
        print_str("HuesOS\n");
        print_str("Status: RAW\n");
        print_str("Version: 0.0.1\n");
        print_str("Author: kurkUma\n");
    }
    else if (strcmp(command, "history") == 0)
    {
        print_str("Command history:\n");
        for (int i = 0; i < history_size; i++)
        {
            print_str("  ");
            print_char('0' + i);
            print_str(": ");
            print_str(command_history[i]);
            print_newline();
        }
    }
    else if (strncmp(command, "echo ", 5) == 0)
    {
        print_str(command + 5);
        print_newline();
    }
    else if (strcmp(command, "shutdown") == 0)
    {
        shutdown();
    }
    else if (strcmp(command, "reboot") == 0)
    {
        reboot();
    }
    else if (strcmp(command, "exit") == 0)
    {
        reboot_triple_fault();
    }
    else
    {
        print_str("Command not found: ");
        print_str(command);
        print_newline();
    }

    show_prompt();
}

void show_prompt()
{
    static char user[32] = "root";
    static char host[32] = "HuesOS";
    static char current_path[256] = "/";
    static char prompt[128];

    uint16_t i = 0;
    prompt[i++] = '[';

    const char *n = user;
    while (*n && i < sizeof(prompt) - 1)
        prompt[i++] = *n++;

    prompt[i++] = '@';

    const char *s = host;
    while (*s && i < sizeof(prompt) - 1)
        prompt[i++] = *s++;

    prompt[i++] = ' ';

    const char *path = current_path;
    while (*path && i < sizeof(prompt) - 1)
        prompt[i++] = *path++;

    prompt[i++] = ']';
    prompt[i++] = ' ';
    prompt[i] = 0;

    print_str(prompt);

    input_index = 0;
    cursor_position = 0;
    input_mode = true;
    current_history_pos = -1;

    for (int i = 0; i < MAX_INPUT_LENGTH; i++)
    {
        input_buffer[i] = 0;
    }
}

void keyboard_handler()
{
    uint8_t status = inb(KEYBOARD_STATUS_PORT);

    if (status & 0x01)
    {
        uint8_t scancode = inb(KEYBOARD_DATA_PORT);

        if (scancode == 0xE0)
        {
            extended_scancode = 1;
            return;
        }

        if (extended_scancode)
        {
            extended_scancode = 0;

            if (scancode < 128)
            {
                switch (scancode)
                {
                case 0x48: // Up
                    if (input_mode)
                    {
                        show_history_prev();
                    }
                    break;
                case 0x50: // Down
                    if (input_mode)
                    {
                        show_history_next();
                    }
                    break;
                case 0x4B: // Left
                    if (input_mode)
                    {
                        move_cursor_left();
                    }
                    break;
                case 0x4D: // Right
                    if (input_mode)
                    {
                        move_cursor_right();
                    }
                    break;
                }
            }
            return;
        }

        if (scancode < 128)
        {
            if (scancode == 0x2A || scancode == 0x36)
            {
                shift_pressed = 1;
            }
            else if (scancode == 0x3A)
            {
                caps_lock = !caps_lock;
            }
            else if (scancode == 0x0E) // Backspace
            {
                if (input_mode && cursor_position > 0)
                {
                    for (int i = cursor_position - 1; i < input_index - 1; i++)
                    {
                        input_buffer[i] = input_buffer[i + 1];
                    }
                    input_index--;
                    cursor_position--;
                    input_buffer[input_index] = '\0';

                    col = get_prompt_start_col();
                    redraw_input_line();

                    for (int i = 0; i < input_index; i++)
                    {
                        print_char(input_buffer[i]);
                    }

                    int chars_printed = input_index;
                    while (chars_printed < 80 - col)
                    {
                        print_char(' ');
                        chars_printed++;
                    }

                    col = cursor_position;
                    redraw_input_line();
                }
            }
            else if (scancode == 0x1C) // Enter
            {
                if (input_mode)
                {
                    input_buffer[input_index] = 0;
                    execute_command(input_buffer);
                }
            }
            else
            {
                char ascii;
                if ((shift_pressed && !caps_lock) || (!shift_pressed && caps_lock))
                {
                    ascii = scancode_upper_case[scancode];
                }
                else
                {
                    ascii = scancode_lower_case[scancode];
                }

                if (ascii != 0 && input_mode)
                {
                    process_input(ascii);
                }
            }
        }
        else
        {
            scancode &= 0x7F;
            if (scancode == 0x2A || scancode == 0x36)
            {
                shift_pressed = 0;
            }
        }
    }
}