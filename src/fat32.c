#include "ata.h"
#include "vga.h"
#include "fat32.h"
#include "commands.h"
#include "string.h"

#define FAT_ENTRY_SIZE 4

static uint32_t cluster_to_lba(fat32_t *fs, uint32_t cluster)
{
    return fs->cluster_begin_lba + (cluster - 2) * fs->sectors_per_cluster;
}

static uint32_t fat32_next_cluster(uint8_t drive, fat32_t *fs, uint32_t cluster)
{
    uint8_t sector[512];
    uint32_t fat_offset = cluster * FAT_ENTRY_SIZE;
    uint32_t fat_sector = fs->fat_begin_lba + (fat_offset / 512);
    uint32_t ent_offset = fat_offset % 512;

    ata_read_sector(drive, fat_sector, sector);
    uint32_t next = *(uint32_t *)(sector + ent_offset);
    return next & 0x0FFFFFFF;
}

bool fat32_cd(uint8_t drive, fat32_t *fs, fat32_dir_t *dir, const char *dirname)
{
    if (command_strcmp(dirname, "/") == 0 && dirname[1] == '\0')
    {
        dir->cluster = fs->root_cluster;
        command_strcpy(dir->path, "/");
        return true;
    }

    if (command_strcmp(dirname, "..") == 0)
    {
        char *last = command_strrchr(dir->path, '/');
        if (last && last != dir->path)
            *last = '\0';
        else
            command_strcpy(dir->path, "/");
        dir->cluster = fs->root_cluster;
        return true;
    }

    uint8_t sector[512];
    uint32_t lba = fs->cluster_begin_lba + (dir->cluster - 2) * fs->sectors_per_cluster;
    ata_read_sector(drive, lba, sector);
    fat_dir_entry_t *entries = (fat_dir_entry_t *)sector;
    size_t entries_per_sector = 512 / sizeof(fat_dir_entry_t);

    char target[11];
    memset(target, ' ', 11);
    int pos = 0;
    for (int i = 0; dirname[i] && pos < 11; i++)
    {
        if (dirname[i] == '.')
            pos = 8;
        else
            target[pos++] = (dirname[i] >= 'a' && dirname[i] <= 'z')
                                ? dirname[i] - 'a' + 'A'
                                : dirname[i];
    }

    for (size_t i = 0; i < entries_per_sector; i++)
    {
        if (entries[i].name[0] == 0x00)
            break;
        if (entries[i].attr == 0x0F)
            continue;
        if (memcmp(entries[i].name, target, 11) == 0)
        {
            if (!(entries[i].attr & 0x10))
            {
                vga_print("Not a directory\n");
                return false;
            }

            uint32_t new_cluster = ((uint32_t)entries[i].high_cluster << 16) | entries[i].low_cluster;
            dir->cluster = new_cluster;

            if (command_strcmp(dir->path, "/") != 0)
                command_strcat(dir->path, "/");
            command_strcat(dir->path, dirname);

            return true;
        }
    }

    vga_print("Directory not found\n");
    return false;
}

void fat32_cat(uint8_t drive, fat32_t *fs, const char *filename)
{
    uint8_t sector[512];
    uint32_t lba = cluster_to_lba(fs, fs->root_cluster);
    ata_read_sector(drive, lba, sector);
    fat_dir_entry_t *entries = (fat_dir_entry_t *)sector;

    char target[11];
    memset(target, ' ', 11);
    int pos = 0;
    for (int i = 0; filename[i] && pos < 11; i++)
    {
        if (filename[i] == '.')
        {
            pos = 8;
        }
        else
        {
            target[pos++] = (filename[i] >= 'a' && filename[i] <= 'z')
                                ? filename[i] - 'a' + 'A'
                                : filename[i];
        }
    }

    fat_dir_entry_t *entry = NULL;
    uint16_t entries_per_sector = 512 / sizeof(fat_dir_entry_t);
    for (uint16_t i = 0; i < entries_per_sector; i++)
    {
        if (entries[i].name[0] == 0x00)
            break;
        if (entries[i].attr == 0x0F)
            continue;
        if (memcmp(entries[i].name, target, 11) == 0)
        {
            entry = &entries[i];
            break;
        }
    }

    if (!entry)
    {
        vga_print("File not found\n");
        return;
    }

    uint32_t cluster = ((uint32_t)entry->high_cluster << 16) | entry->low_cluster;
    uint32_t remaining = entry->size;
    uint8_t buffer[512];

    while (cluster < 0x0FFFFFF8)
    {
        uint32_t lba_data = cluster_to_lba(fs, cluster);
        for (uint32_t s = 0; s < fs->sectors_per_cluster && remaining > 0; s++)
        {
            ata_read_sector(drive, lba_data + s, buffer);
            uint32_t to_print = (remaining > 512) ? 512 : remaining;
            for (uint32_t j = 0; j < to_print; j++)
            {
                vga_putchar(buffer[j]);
            }
            remaining -= to_print;
        }
        cluster = fat32_next_cluster(drive, fs, cluster);
    }

    vga_print("\n");
}

void fat32_mount(uint8_t drive, fat32_t *fs)
{
    static uint8_t sector[512];
    vga_print("Mounting FAT32...\n");
    uint32_t lba_boot = 2048;

    ata_read_sector(drive, lba_boot, sector);
    vga_print("Sector read OK\n");

    fs->bytes_per_sector = *(uint16_t *)(sector + 11);
    fs->sectors_per_cluster = *(uint8_t *)(sector + 13);
    fs->reserved_sectors = *(uint16_t *)(sector + 14);
    fs->fat_count = *(uint8_t *)(sector + 16);
    fs->sectors_per_fat = *(uint32_t *)(sector + 36);
    fs->root_cluster = *(uint32_t *)(sector + 44);

    fs->fat_begin_lba = lba_boot + fs->reserved_sectors;
    fs->cluster_begin_lba = fs->fat_begin_lba + fs->fat_count * fs->sectors_per_fat;

    vga_print("FAT32 mounted:\n");
    vga_print("Bytes per sector: ");
    vga_print_num(fs->bytes_per_sector);
    vga_putchar('\n');
    vga_print("Sectors per cluster: ");
    vga_print_num(fs->sectors_per_cluster);
    vga_putchar('\n');
    vga_print("Root cluster: ");
    vga_print_num(fs->root_cluster);
    vga_putchar('\n');
}

void fat32_list_dir(uint8_t drive, fat32_t *fs, fat32_dir_t *dir)
{
    uint8_t sector[512];
    uint32_t lba = cluster_to_lba(fs, dir->cluster);
    ata_read_sector(drive, lba, sector);

    fat_dir_entry_t *entries = (fat_dir_entry_t *)sector;
    uint16_t entries_per_sector = 512 / sizeof(fat_dir_entry_t);

    for (uint16_t i = 0; i < entries_per_sector; i++)
    {
        if (entries[i].name[0] == 0x00)
            break;
        if ((uint8_t)entries[i].name[0] == 0xE5)
            continue;
        if (entries[i].attr == 0x0F)
            continue;

        char formatted[13];
        int pos = 0;
        for (int j = 0; j < 8; j++)
        {
            if (entries[i].name[j] != ' ')
                formatted[pos++] = entries[i].name[j];
        }

        if (entries[i].name[8] != ' ')
        {
            formatted[pos++] = '.';
            for (int j = 8; j < 11; j++)
            {
                if (entries[i].name[j] != ' ')
                    formatted[pos++] = entries[i].name[j];
            }
        }

        formatted[pos] = '\0';

        vga_print(formatted);

        if (entries[i].attr & 0x10)
            vga_print("/");
        else
        {
            vga_putchar(' ');
            vga_print_num(entries[i].size);
        }

        vga_print("\n");
    }
}