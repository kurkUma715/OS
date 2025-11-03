#ifndef COMMANDS_H
#define COMMANDS_H

#include <stdint.h>
#include <stdbool.h>

#define INPUT_BUFFER_SIZE 128
#define NULL ((void *)0)

extern char input_buffer[INPUT_BUFFER_SIZE];
extern uint16_t input_len;
extern bool ctrl_c_pressed;

void command_prompt_and_readline(const char *prompt);
int command_strcmp(const char *a, const char *b);
int command_strncmp(const char *a, const char *b, uint16_t n);
void command_strcpy(char *dest, const char *src);
uint16_t command_strlen(const char *s);
void command_do_reboot(void);
void command_do_shutdown(void);

#endif