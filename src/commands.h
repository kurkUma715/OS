#ifndef COMMANDS_H
#define COMMANDS_H

#include <stdint.h>

#define INPUT_BUFFER_SIZE 128

extern char input_buffer[INPUT_BUFFER_SIZE];
extern uint16_t input_len;

void command_prompt_and_readline(const char* prompt);
int command_strcmp(const char* a, const char* b);
void command_do_reboot(void);
void command_do_shutdown(void);

#endif