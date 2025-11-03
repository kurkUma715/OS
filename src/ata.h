#ifndef ATA_H
#define ATA_H

#include <stdint.h>

typedef struct
{
    char name[8];
    uint16_t io_base;
    uint8_t slave;
    uint32_t size_in_sectors;
    uint8_t present;
} ata_device_t;

void ata_init();
void ata_identify(ata_device_t *dev);
void ata_read_sector(uint8_t drive, uint32_t lba, uint8_t *buffer);
void ata_lsblk();

#endif