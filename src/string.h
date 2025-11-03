#pragma once
#include <stddef.h>

void *memset(void *dest, int value, size_t size);
void *memcpy(void *dest, const void *src, size_t size);
int memcmp(const void *s1, const void *s2, size_t n);
size_t strlen(const char *s);
char *strcat(char *dest, const char *src);