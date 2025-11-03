#include "fat32.h"
#include "ata.h"
#include "vga.h"

static mbr_t mbr;

static void print_hex(uint8_t value)
{
    char buf[3];
    buf[0] = "0123456789ABCDEF"[value >> 4];
    buf[1] = "0123456789ABCDEF"[value & 0xF];
    buf[2] = 0;
    vga_print(buf);
}

static uint16_t string_length(const char *str)
{
    uint16_t len = 0;
    while (str[len] != '\0')
        len++;
    return len;
}
void read_mbr(void)
{
    // vga_print("Reading MBR from sector 0...\n");

    // Читаем MBR (сектор 0)
    int sectors_read = ata_read_sectors(0, (uint8_t *)&mbr, 1);

    if (sectors_read != 1)
    {
        // vga_print("Error: Could not read MBR sector\n");
        return;
    }

    // Проверяем сигнатуру
    if (mbr.signature != 0xAA55)
    {
        // vga_print("Invalid MBR signature: 0x");
        // print_hex((mbr.signature >> 8) & 0xFF);
        // print_hex(mbr.signature & 0xFF);
        // vga_print("\n");

        // Для отладки: выведем первые 64 байта MBR
        // vga_print("First 64 bytes of MBR:\n");
        uint8_t *mbr_bytes = (uint8_t *)&mbr;
        for (int i = 0; i < 64; i++)
        {
            // if (i % 16 == 0)
            // vga_print("\n");
            // print_hex(mbr_bytes[i]);
            // vga_print(" ");
        }
        // vga_print("\n");
        return;
    }

    // vga_print("Valid MBR found\n");
}

void fat32_list_partitions(void)
{
    read_mbr();

    vga_print("\nDisk Partitions:\n");
    vga_print("Num Type        Start     Size      Active\n");
    vga_print("--- ----------- --------- --------- ------\n");

    int found = 0;
    for (int i = 0; i < 4; i++)
    {
        partition_entry_t *p = &mbr.partitions[i];

        if (p->type == 0x00)
            continue;
        found = 1;

        // Partition number
        vga_putchar(' ');
        vga_putchar('0' + i);
        vga_print("  ");

        // Type
        const char *type_name = get_partition_type_name(p->type);
        vga_print(type_name);

        uint16_t type_len = string_length(type_name);
        for (int j = 0; j < 11 - type_len; j++)
            vga_putchar(' ');

        // Start LBA
        char buf[10];
        uint32_t value = p->lba_start;
        int pos = 9;
        buf[pos] = 0;
        while (pos > 0)
        {
            buf[--pos] = '0' + (value % 10);
            value /= 10;
            if (value == 0)
                break;
        }
        vga_print(&buf[pos]);
        vga_print(" ");

        // Size in MB
        value = (p->sector_count * 512) / (1024 * 1024);
        pos = 9;
        buf[pos] = 0;
        while (pos > 0)
        {
            buf[--pos] = '0' + (value % 10);
            value /= 10;
            if (value == 0)
                break;
        }
        vga_print(&buf[pos]);
        vga_print("MB ");

        // Active
        vga_print(p->status == 0x80 ? "Yes" : "No");
        vga_print("\n");
    }

    if (!found)
    {
        vga_print("No partitions found\n");
    }
    vga_print("\n");
}

const char *get_partition_type_name(uint8_t type)
{
    switch (type)
    {
    case 0x00:
        return "Empty";
    case 0x01:
        return "FAT12";
    case 0x04:
        return "FAT16 <32M";
    case 0x05:
        return "Extended";
    case 0x06:
        return "FAT16";
    case 0x07:
        return "NTFS/HPFS";
    case 0x0B:
        return "FAT32";
    case 0x0C:
        return "FAT32 LBA";
    case 0x0E:
        return "FAT16 LBA";
    case 0x0F:
        return "Extended LBA";
    case 0x82:
        return "Linux Swap";
    case 0x83:
        return "Linux";
    case 0x8E:
        return "Linux LVM";
    default:
        return "Unknown";
    }
}