#include "ata.h"
#include "ports.h"
#include "vga.h"

// Используем первый канал
#define ATA_BASE ATA_PRIMARY_BASE
#define ATA_CTRL ATA_PRIMARY_CTRL

static void print_hex(uint8_t value)
{
    char buf[3];
    buf[0] = "0123456789ABCDEF"[value >> 4];
    buf[1] = "0123456789ABCDEF"[value & 0xF];
    buf[2] = 0;
    vga_print(buf);
}

static void ata_delay(void)
{
    inb(ATA_CTRL);
    inb(ATA_CTRL);
    inb(ATA_CTRL);
    inb(ATA_CTRL);
}

static int ata_wait_not_busy(void)
{
    int timeout = 100000;
    while (timeout-- > 0)
    {
        if ((inb(ATA_BASE + 7) & ATA_SR_BSY) == 0)
        {
            return 1;
        }
        ata_delay();
    }
    return 0;
}

static int ata_wait_drq(void)
{
    int timeout = 100000;
    while (timeout-- > 0)
    {
        uint8_t status = inb(ATA_BASE + 7);
        if (status & ATA_SR_ERR)
            return 0;
        if (status & ATA_SR_DRQ)
            return 1;
        ata_delay();
    }
    return 0;
}

static int ata_probe_drive(uint16_t base, uint16_t ctrl)
{
    // Select master drive
    outb(base + 6, 0xA0);

    // Send IDENTIFY command
    outb(base + 7, ATA_CMD_IDENTIFY);

    // Check status
    uint8_t status = inb(base + 7);
    if (status == 0)
    {
        return 0; // No drive
    }

    // Wait for BSY to clear
    int timeout = 100000;
    while (timeout-- > 0)
    {
        status = inb(base + 7);
        if (!(status & ATA_SR_BSY))
        {
            break;
        }
    }

    if (timeout <= 0)
    {
        return 0;
    }

    // Check if drive is ready
    if (status & ATA_SR_DRQ)
    {
        return 1; // Drive found and ready
    }

    return 0;
}

void ata_init(void)
{
    vga_print("ATA: Probing for drives...\n");

    // Probe primary channel
    if (ata_probe_drive(ATA_PRIMARY_BASE, ATA_PRIMARY_CTRL))
    {
        vga_print("ATA: Drive found on primary channel\n");
        return;
    }

    // Probe secondary channel
    if (ata_probe_drive(ATA_SECONDARY_BASE, ATA_SECONDARY_CTRL))
    {
        vga_print("ATA: Drive found on secondary channel\n");
        return;
    }

    vga_print("ATA: No drives found\n");
}

int ata_read_sectors(uint32_t lba, uint8_t *buffer, uint32_t count)
{
    if (count == 0 || !buffer)
        return 0;

    if (!ata_wait_not_busy())
    {
        vga_print("ATA: Drive busy\n");
        return 0;
    }

    // Select drive (LBA mode, master)
    outb(ATA_BASE + 6, 0xE0 | ((lba >> 24) & 0x0F));

    // Send parameters
    outb(ATA_BASE + 2, (uint8_t)count);
    outb(ATA_BASE + 3, (uint8_t)lba);
    outb(ATA_BASE + 4, (uint8_t)(lba >> 8));
    outb(ATA_BASE + 5, (uint8_t)(lba >> 16));

    // Send read command
    outb(ATA_BASE + 7, ATA_CMD_READ_SECTORS);

    uint32_t sectors_read = 0;

    for (uint32_t i = 0; i < count; i++)
    {
        if (!ata_wait_drq())
        {
            vga_print("ATA: DRQ timeout\n");
            return sectors_read;
        }

        // Read 256 words (512 bytes) using inb instead of inw
        for (int j = 0; j < 256; j++)
        {
            uint16_t data = inb(ATA_BASE) | (inb(ATA_BASE) << 8);
            *((uint16_t *)buffer) = data;
            buffer += 2;
        }
        sectors_read++;
    }

    return sectors_read;
}

int ata_identify(void)
{
    vga_print("ATA: Identifying drive...\n");

    // Software reset
    outb(ATA_CTRL, 0x04);
    ata_delay();
    outb(ATA_CTRL, 0x00);
    ata_delay();

    if (!ata_wait_not_busy())
    {
        vga_print("ATA: Drive busy after reset\n");
        return 0;
    }

    // Select master drive
    outb(ATA_BASE + 6, 0xA0);

    // Send IDENTIFY command
    outb(ATA_BASE + 7, ATA_CMD_IDENTIFY);

    // Check if drive exists
    uint8_t status = inb(ATA_BASE + 7);
    if (status == 0)
    {
        vga_print("ATA: No drive (status=0)\n");
        return 0;
    }

    // Wait for BSY to clear and DRQ to set
    int timeout = 100000;
    while (timeout-- > 0)
    {
        status = inb(ATA_BASE + 7);
        if (status & ATA_SR_ERR)
        {
            vga_print("ATA: Error during identify\n");
            return 0;
        }
        if (!(status & ATA_SR_BSY) && (status & ATA_SR_DRQ))
        {
            break;
        }
        ata_delay();
    }

    if (timeout <= 0)
    {
        vga_print("ATA: Identify timeout\n");
        return 0;
    }

    // Read IDENTIFY data (512 bytes)
    for (int i = 0; i < 256; i++)
    {
        inw(ATA_BASE);
    }

    vga_print("ATA: Drive identified successfully\n");
    return 1;
}