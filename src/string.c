#include "string.h"

void *memset(void *dest, int value, size_t size)
{
    unsigned char *ptr = dest;
    while (size--)
        *ptr++ = (unsigned char)value;
    return dest;
}

void *memcpy(void *dest, const void *src, size_t size)
{
    unsigned char *d = dest;
    const unsigned char *s = src;
    while (size--)
        *d++ = *s++;
    return dest;
}

int memcmp(const void *s1, const void *s2, size_t n)
{
    const unsigned char *a = s1;
    const unsigned char *b = s2;
    for (size_t i = 0; i < n; i++)
    {
        if (a[i] != b[i])
            return a[i] - b[i];
    }
    return 0;
}

size_t strlen(const char *s)
{
    size_t len = 0;
    while (*s++)
        len++;
    return len;
}

char *strcpy(char *dest, const char *src)
{
    char *r = dest;
    while ((*dest++ = *src++))
        ;
    return r;
}

char *strrchr(const char *s, int c)
{
    const char *last = NULL;
    while (*s)
    {
        if (*s == (char)c)
            last = s;
        s++;
    }
    return (char *)last;
}

char *strcat(char *dest, const char *src)
{
    char *r = dest;
    while (*dest)
        dest++;
    while (*src)
        *dest++ = *src++;
    *dest = 0;
    return r;
}