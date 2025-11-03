# Toolchain
CC      := gcc
AS      := nasm
LD      := ld
OBJS    := build/boot.o build/kernel.o build/vga.o build/ports.o build/commands.o build/keyboard.o

# Flags
CFLAGS  := -std=gnu99 -ffreestanding -fno-builtin -nostdlib -nostartfiles -nodefaultlibs \
           -Wall -Wextra -O2 -m32 -Isrc -fno-stack-protector -fno-pie -fno-pic
ASFLAGS := -f elf32
LDFLAGS := -T linker.ld -nostdlib -static -m elf_i386

# Values
RAM 	:= 1G
NAME 	:= huesos

# Paths
SRC_DIR := src
BUILD   := build
ISO_DIR := iso
ISO     := $(ISO_DIR)/${NAME}.iso
IMG     := ${NAME}.img


# Targets
all: $(ISO)

$(BUILD)/boot.o: $(SRC_DIR)/boot.asm
	@mkdir -p $(BUILD)
	$(AS) $(ASFLAGS) $< -o $@

$(BUILD)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

kernel.elf: $(OBJS) linker.ld
	$(LD) $(LDFLAGS) $(OBJS) -o $@

$(ISO_DIR)/boot/kernel.elf: kernel.elf
	@mkdir -p $(ISO_DIR)/boot
	cp $< $@

$(ISO_DIR)/boot/grub/grub.cfg: grub.cfg
	@mkdir -p $(ISO_DIR)/boot/grub
	cp $< $@

$(ISO): $(ISO_DIR)/boot/kernel.elf $(ISO_DIR)/boot/grub/grub.cfg
	grub-mkrescue -o $(ISO) $(ISO_DIR) >/dev/null 2>&1 || true

# Create bootable IMG ===
$(IMG): kernel.elf grub.cfg create_image.sh
	./create_image.sh

# Run QEMU
run: $(IMG)
	qemu-system-i386 -m ${RAM} -hda $(IMG)

debug: $(IMG)
	qemu-system-i386 -m ${RAM} -hda $(IMG) -no-reboot -no-shutdown -serial stdio

clean:
	rm -rf $(BUILD) kernel.elf $(ISO_DIR) $(IMG)

.PHONY: all run debug clean
