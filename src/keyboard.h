#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdbool.h>

#define PORT_KEYBOARD 0x60
#define PORT_STATUS   0x64

char keyboard_read_key_char(bool *is_left_arrow, bool *is_right_arrow);

#endif