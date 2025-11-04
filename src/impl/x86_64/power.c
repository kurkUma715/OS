#include "ports.h"

#define SHUTDOWN_PORT 0x604
#define REBOOT_PORT 0x64

#define ACPI_SHUTDOWN 0x2000
#define ACPI_REBOOT 0x2001

void shutdown()
{
    outw(0x4004, 0x3400);
    outw(SHUTDOWN_PORT, ACPI_SHUTDOWN);

    uint8_t temp;
    do
    {
        temp = inb(REBOOT_PORT);
        if (temp & 0x02)
        {
            continue;
        }
        outb(REBOOT_PORT, 0xFE);
    } while (1);
}

void reboot()
{
    uint8_t status;

    do
    {
        status = inb(REBOOT_PORT);
    } while (status & 0x02);

    outb(REBOOT_PORT, 0xFE);
}

void reboot_triple_fault()
{
    asm volatile(
        "cli\n"
        "lidt 0\n"
        "int $0\n");
}