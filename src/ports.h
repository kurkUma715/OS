#ifndef PORTS_H
#define PORTS_H

#include <stdint.h>

uint8_t inb(uint16_t port);
uint16_t inw(uint16_t port); // Добавьте эту строку
void outb(uint16_t port, uint8_t value);
void outw(uint16_t port, uint16_t value);

#endif