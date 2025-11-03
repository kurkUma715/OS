#ifndef FAT32_H
#define FAT32_H
#include <stdbool.h>
#include <stdint.h>

typedef struct
{
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t fat_count;
    uint32_t sectors_per_fat;
    uint32_t root_cluster;
    uint32_t fat_begin_lba;
    uint32_t cluster_begin_lba;
} fat32_t;

typedef struct
{
    char name[11];
    uint8_t attr;
    uint8_t reserved;
    uint8_t create_time_tenths;
    uint16_t create_time;
    uint16_t create_date;
    uint16_t last_access_date;
    uint16_t high_cluster;
    uint16_t write_time;
    uint16_t write_date;
    uint16_t low_cluster;
    uint32_t size;
} __attribute__((packed)) fat_dir_entry_t;

typedef struct
{
    uint32_t cluster;
    char path[128];
} fat32_dir_t;

bool fat32_cd(uint8_t drive, fat32_t *fs, fat32_dir_t *dir, const char *dirname);
void fat32_cat(uint8_t drive, fat32_t *fs, const char *filename);
void fat32_mount(uint8_t drive, fat32_t *fs);
void fat32_list_dir(uint8_t drive, fat32_t *fs, fat32_dir_t *dir);

#endif