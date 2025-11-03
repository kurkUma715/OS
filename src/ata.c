#include "ports.h"
#include "vga.h"
#include "ata.h"

#define ATA_PRIMARY_IO 0x1F0
#define ATA_PRIMARY_CTRL 0x3F6

#define ATA_SECONDARY_IO 0x170
#define ATA_SECONDARY_CTRL 0x376

#define ATA_REG_DATA 0x00
#define ATA_REG_ERROR 0x01
#define ATA_REG_SECCOUNT0 0x02
#define ATA_REG_LBA0 0x03
#define ATA_REG_LBA1 0x04
#define ATA_REG_LBA2 0x05
#define ATA_REG_HDDEVSEL 0x06
#define ATA_REG_COMMAND 0x07
#define ATA_REG_STATUS 0x07

#define ATA_CMD_IDENTIFY 0xEC
#define ATA_SR_BSY 0x80
#define ATA_SR_DRDY 0x40
#define ATA_SR_DRQ 0x08
#define ATA_SR_ERR 0x01

#define ATA_CMD_READ_SECTORS 0x20

static ata_device_t devices[4];

static uint8_t ata_read_status(uint16_t io)
{
    return inb(io + ATA_REG_STATUS);
}

static void ata_wait_busy(uint16_t io)
{
    while (ata_read_status(io) & ATA_SR_BSY)
        ;
}

void ata_identify(ata_device_t *dev)
{
    uint16_t io = dev->io_base;
    outb(io + ATA_REG_HDDEVSEL, 0xA0 | (dev->slave << 4));
    outb(io + ATA_REG_SECCOUNT0, 0);
    outb(io + ATA_REG_LBA0, 0);
    outb(io + ATA_REG_LBA1, 0);
    outb(io + ATA_REG_LBA2, 0);
    outb(io + ATA_REG_COMMAND, ATA_CMD_IDENTIFY);

    uint8_t status = inb(io + ATA_REG_STATUS);
    if (status == 0)
    {
        dev->present = 0;
        return;
    }

    ata_wait_busy(io);

    status = ata_read_status(io);
    if (status & ATA_SR_ERR)
    {
        dev->present = 0;
        return;
    }

    while (!(status & ATA_SR_DRQ))
    {
        status = ata_read_status(io);
        if (status & ATA_SR_ERR)
        {
            dev->present = 0;
            return;
        }
    }

    uint16_t data[256];
    for (int i = 0; i < 256; i++)
        data[i] = inw(io + ATA_REG_DATA);

    uint32_t sectors = (uint32_t)data[60] | ((uint32_t)data[61] << 16);
    dev->size_in_sectors = sectors;
    dev->present = 1;
}

void ata_read_sector(uint8_t drive, uint32_t lba, uint8_t *buffer)
{
    ata_device_t *dev = &devices[drive];
    uint16_t io = dev->io_base;

    while (inb(io + ATA_REG_STATUS) & ATA_SR_BSY)
        ;

    outb(io + ATA_REG_HDDEVSEL, 0xE0 | (dev->slave << 4) | ((lba >> 24) & 0x0F));
    outb(io + ATA_REG_SECCOUNT0, 1);
    outb(io + ATA_REG_LBA0, (uint8_t)lba);
    outb(io + ATA_REG_LBA1, (uint8_t)(lba >> 8));
    outb(io + ATA_REG_LBA2, (uint8_t)(lba >> 16));
    outb(io + ATA_REG_COMMAND, ATA_CMD_READ_SECTORS);

    uint8_t status;
    do
    {
        status = inb(io + ATA_REG_STATUS);
    } while (!(status & ATA_SR_DRQ));

    for (int i = 0; i < 256; i++)
    {
        uint16_t data = inw(io + ATA_REG_DATA);
        buffer[i * 2] = data & 0xFF;
        buffer[i * 2 + 1] = data >> 8;
    }
}

void ata_init()
{
    vga_print("Scanning ATA channels...\n");

    // Primary channel
    devices[0].io_base = ATA_PRIMARY_IO;
    devices[0].slave = 0;
    devices[1].io_base = ATA_PRIMARY_IO;
    devices[1].slave = 1;

    // Secondary channel
    devices[2].io_base = ATA_SECONDARY_IO;
    devices[2].slave = 0;
    devices[3].io_base = ATA_SECONDARY_IO;
    devices[3].slave = 1;

    for (int i = 0; i < 4; i++)
    {
        ata_identify(&devices[i]);
    }

    int found = 0;
    for (int i = 0; i < 4; i++)
    {
        if (devices[i].present)
            found = 1;
    }

    if (!found)
    {
        vga_print("No ATA disks detected.\n");
    }
    else
    {
        vga_print("ATA scan complete.\n");
    }
}

static void itoa(uint32_t value, char *str, int base)
{
    char *p = str;
    char *p1, tmp;
    uint32_t n = value;

    if (n == 0)
    {
        *p++ = '0';
        *p = '\0';
        return;
    }

    while (n > 0)
    {
        int rem = n % base;
        *p++ = (rem < 10) ? ('0' + rem) : ('A' + rem - 10);
        n /= base;
    }
    *p = '\0';

    p1 = str;
    p--;
    while (p1 < p)
    {
        tmp = *p1;
        *p1 = *p;
        *p = tmp;
        p1++;
        p--;
    }
}

void ata_lsblk()
{
    int found = 0;
    for (int i = 0; i < 4; i++)
    {
        if (devices[i].present)
        {
            found = 1;
            char buf[32];
            itoa(devices[i].size_in_sectors, buf, 10);

            vga_print("sd");
            char letter[2] = {'a' + i, 0};
            vga_print(letter);
            vga_print(" ");
            vga_print(buf);
            vga_print(" sectors\n");
        }
        else
        {
            vga_print("No disk on ");
            if (i < 2)
            {
                vga_print("primary ");
            }
            else
            {
                vga_print("secondary ");
            }
            vga_print((i % 2) ? "slave\n" : "master\n");
        }
    }

    if (!found)
    {
        vga_print("No disks present.\n");
    }
}
