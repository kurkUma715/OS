#!/bin/bash
set -e

KERNEL="kernel.elf"
IMAGE="huesos.img"
MOUNT_DIR="mnt"
SIZE_MB=64

if [ ! -f "$KERNEL" ]; then
    echo "> ERROR: $KERNEL not found. Run 'make' first."
    exit 1
fi

echo "> Create $SIZE_MB MB image ($IMAGE)..."
dd if=/dev/zero of=$IMAGE bs=1M count=$SIZE_MB status=none

echo "> Label disk..."
parted -s $IMAGE mklabel msdos
parted -s $IMAGE mkpart primary fat32 1MiB 100%
parted -s $IMAGE set 1 boot on

LOOP=$(sudo losetup -Pf --show $IMAGE)
PART=${LOOP}p1

echo "> Create FAT32 filesystem..."
sudo mkfs.vfat -F32 $PART > /dev/null

mkdir -p $MOUNT_DIR
sudo mount $PART $MOUNT_DIR

echo "> Copy kernel and boot files..."
sudo mkdir -p $MOUNT_DIR/boot/grub
sudo cp -r img/* $MOUNT_DIR/ 2>/dev/null || true
sudo cp "$KERNEL" $MOUNT_DIR/boot/
sudo cp grub.cfg $MOUNT_DIR/boot/grub/

echo "> Install GRUB..."
sudo grub-install --target=i386-pc \
    --boot-directory=$MOUNT_DIR/boot \
    --modules="normal part_msdos ext2 fat multiboot" \
    --no-floppy --recheck $LOOP > /dev/null

echo "> Unmount and cleanup..."
sync
sudo umount -l $MOUNT_DIR || true
sudo losetup -d $LOOP || true
sleep 1
rmdir $MOUNT_DIR 2>/dev/null || true

echo "> Done: $IMAGE"
