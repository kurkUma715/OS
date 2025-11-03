#include "fs.h"
#include "fat32.h"

void list_partitions(void)
{
    fat32_list_partitions();
}