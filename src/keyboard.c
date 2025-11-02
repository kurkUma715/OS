#include "keyboard.h"
#include "ports.h"

static bool shift = false;
static bool capslock = false;

static const char scancode_lower_case[128] = {
    0,  27, '1', '2', '3', '4', '5', '6',
    '7', '8', '9', '0', '-', '=', '\b', '\t',
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i',
    'o', 'p', '[', ']', '\n', 0, 'a', 's',
    'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',
    '\'', '`', 0, '\\', 'z', 'x', 'c', 'v',
    'b', 'n', 'm', ',', '.', '/', 0, '*',
    0, ' ', 0
};

static const char scancode_upper_case[128] = {
    0,  27, '!', '@', '#', '$', '%', '^',
    '&', '*', '(', ')', '_', '+', '\b', '\t',
    'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I',
    'O', 'P', '{', '}', '\n', 0, 'A', 'S',
    'D', 'F', 'G', 'H', 'J', 'K', 'L', ':',
    '"', '~', 0, '|', 'Z', 'X', 'C', 'V',
    'B', 'N', 'M', '<', '>', '?', 0, '*',
    0, ' ', 0
};

static uint8_t keyboard_read_scancode_byte(void) {
    while (!(inb(PORT_STATUS) & 0x01)) { }
    return inb(PORT_KEYBOARD);
}

char keyboard_read_key_char(bool *is_left_arrow, bool *is_right_arrow) {
    *is_left_arrow = false;
    *is_right_arrow = false;

    uint8_t b = keyboard_read_scancode_byte();

    if (b == 0xE0) {
        uint8_t e = keyboard_read_scancode_byte();
        if (e & 0x80) {
            return 0;
        }
        
        switch (e) {
            case 0x4B: *is_left_arrow = true; return 0;
            case 0x4D: *is_right_arrow = true; return 0;
        }
        return 0;
    }

    if (b & 0x80) {
        uint8_t code = b & 0x7F;
        switch (code) {
            case 42: case 54: shift = false; break;
        }
        return 0;
    }

    switch (b) {
        case 42: case 54: shift = true; return 0;
        case 58: capslock = !capslock; return 0;
    }

    char c = shift ? scancode_upper_case[b] : scancode_lower_case[b];
    if (!c) return 0;

    if (c >= 'a' && c <= 'z') {
        if (capslock && !shift) c = c - 'a' + 'A';
    } else if (c >= 'A' && c <= 'Z') {
        if (capslock && !shift) c = c - 'A' + 'a';
    }

    return c;
}