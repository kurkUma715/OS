#!/bin/bash
set -e

KERNEL="kernel.elf"
IMAGE="huesos.img"
MOUNT_DIR="mnt"
SIZE_MB=32

# Проверяем, что ядро существует
if [ ! -f "$KERNEL" ]; then
    echo "Ошибка: $KERNEL не найден. Сначала собери проект (make)."
    exit 1
fi

echo "===> Создаём $SIZE_MB MB образ ($IMAGE)..."
dd if=/dev/zero of=$IMAGE bs=1M count=$SIZE_MB status=none

# Размечаем диск (MBR, один раздел FAT32)
echo "===> Размечаем диск..."
parted -s $IMAGE mklabel msdos
parted -s $IMAGE mkpart primary fat32 1MiB 100%
parted -s $IMAGE set 1 boot on

# Подключаем loop-устройство
LOOP=$(sudo losetup -Pf --show $IMAGE)
PART=${LOOP}p1

echo "===> Создаём файловую систему FAT32..."
sudo mkfs.vfat -F32 $PART > /dev/null

# Монтируем раздел и копируем файлы
mkdir -p $MOUNT_DIR
sudo mount $PART $MOUNT_DIR

echo "===> Копируем ядро и GRUB..."
sudo mkdir -p $MOUNT_DIR/boot/grub
sudo cp kernel.elf $MOUNT_DIR/boot/
sudo cp grub.cfg $MOUNT_DIR/boot/grub/

# Устанавливаем GRUB в образ
echo "===> Устанавливаем GRUB..."
sudo grub-install --target=i386-pc --boot-directory=$MOUNT_DIR/boot --modules="normal part_msdos ext2 fat multiboot" --no-floppy --recheck $LOOP > /dev/null

# Отмонтируем и освободим loop
sudo umount $MOUNT_DIR
sudo losetup -d $LOOP
rmdir $MOUNT_DIR

echo "===> Готово: $IMAGE"
