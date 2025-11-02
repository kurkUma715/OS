#include "keyboard.h"
#include "ports.h"
#include "vga.h"

static bool shift = false;
static bool capslock = false;
static bool ctrl = false;

static uint8_t keyboard_read_scancode_byte(void) {
    while (!(inb(PORT_STATUS) & 0x01)) { }
    return inb(PORT_KEYBOARD);
}

KeyEvent keyboard_read_key(void) {
    KeyEvent event = {0};
    
    uint8_t b = keyboard_read_scancode_byte();
    
    

    if (b == 0xE0) {
        uint8_t e = keyboard_read_scancode_byte();
        if (e & 0x80) {
            return event;
        }
        
        switch (e) {
            case 0x4B: event.is_left_arrow = true; return event;
            case 0x4D: event.is_right_arrow = true; return event;
        }
        return event;
    }

    if (b & 0x80) {
        uint8_t code = b & 0x7F;
        switch (code) {
            case 29:  // Left Ctrl
            case 157: // Right Ctrl
                ctrl = false;
                break;
            case 42: 
            case 54: 
                shift = false; 
                break;
        }
        event.ctrl_pressed = ctrl;
        return event;
    }

    switch (b) {
        case 29:  // Left Ctrl
        case 157: // Right Ctrl
            ctrl = true;
            event.ctrl_pressed = true;
            return event;
            
        case 42: 
        case 54: 
            shift = true; 
            event.ctrl_pressed = ctrl;
            return event;
            
        case 58: 
            capslock = !capslock; 
            event.ctrl_pressed = ctrl;
            return event;
    }

    if (ctrl) {
        switch (b) {
            case 32:
                event.ctrl_d = true;
                return event;
            case 46:
                event.ctrl_c = true;
                return event;
            case 38: 
                event.ctrl_l = true;
                return event;
        }
    }

    char c = shift ? scancode_upper_case[b] : scancode_lower_case[b];
    if (!c) {
        event.ctrl_pressed = ctrl;
        return event;
    }

    if (c >= 'a' && c <= 'z') {
        if (capslock && !shift) c = c - 'a' + 'A';
    } else if (c >= 'A' && c <= 'Z') {
        if (capslock && !shift) c = c - 'A' + 'a';
    }

    event.character = c;
    event.ctrl_pressed = ctrl;
    return event;
}

char keyboard_read_key_char(bool *is_left_arrow, bool *is_right_arrow) {
    KeyEvent event = keyboard_read_key();
    if (is_left_arrow) *is_left_arrow = event.is_left_arrow;
    if (is_right_arrow) *is_right_arrow = event.is_right_arrow;
    return event.character;
}