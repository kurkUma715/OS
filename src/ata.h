#ifndef ATA_H
#define ATA_H

#include <stdint.h>

// Primary ATA Channel
#define ATA_PRIMARY_BASE 0x1F0
#define ATA_PRIMARY_CTRL 0x3F6

// Secondary ATA Channel (для жесткого диска)
#define ATA_SECONDARY_BASE 0x170
#define ATA_SECONDARY_CTRL 0x376

// ATA Commands
#define ATA_CMD_READ_SECTORS 0x20
#define ATA_CMD_IDENTIFY 0xEC

// ATA Status Register Bits
#define ATA_SR_BSY 0x80
#define ATA_SR_DRDY 0x40
#define ATA_SR_DF 0x20
#define ATA_SR_DSC 0x10
#define ATA_SR_DRQ 0x08
#define ATA_SR_CORR 0x04
#define ATA_SR_IDX 0x02
#define ATA_SR_ERR 0x01

void ata_init(void);
int ata_read_sectors(uint32_t lba, uint8_t *buffer, uint32_t count);
int ata_identify(void);

#endif