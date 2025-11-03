#ifndef COMMANDS_H
#define COMMANDS_H

#include <stdint.h>
#include <stdbool.h>
#include "fat32.h"

#define INPUT_BUFFER_SIZE 128

extern char input_buffer[INPUT_BUFFER_SIZE];
extern uint16_t input_len;
extern bool ctrl_c_pressed;
extern bool ctrl_d_pressed;

void command_prompt_and_readline(const char *prompt);
int command_strcmp(const char *a, const char *b);
int command_strncmp(const char *a, const char *b, uint16_t n);
void command_strcpy(char *dest, const char *src);
void command_strcat(char *dest, const char *src);
uint16_t command_strlen(const char *s);
char *command_strrchr(const char *s, int c);
void command_do_reboot(void);
void command_do_shutdown(void);
void command_delay(uint32_t ticks);
bool command_dispatch(const char *name);

void update_current_path(char *path, const char *newdir);

#endif