CC = gcc
AS = nasm
LD = ld

CFLAGS = -std=gnu99 -ffreestanding -fno-builtin -nostdlib -nostartfiles -nodefaultlibs -Wall -Wextra -O2 -m32 -Isrc -fno-stack-protector -fno-pie -fno-pic
ASFLAGS = -f elf32
LDFLAGS = -T linker.ld -nostdlib -static -m elf_i386

SRC = src
OBJDIR = build
ISO_DIR = iso

KERNEL_OBJS = $(OBJDIR)/boot.o $(OBJDIR)/kernel.o $(OBJDIR)/vga.o \
              $(OBJDIR)/keyboard.o $(OBJDIR)/commands.o $(OBJDIR)/ports.o $(OBJDIR)/fs.o $(OBJDIR)/fat32.o $(OBJDIR)/ata.o

all: $(ISO_DIR)/huesos.iso

$(OBJDIR)/boot.o: $(SRC)/boot.asm
	mkdir -p $(OBJDIR)
	$(AS) $(ASFLAGS) $< -o $@

$(OBJDIR)/%.o: $(SRC)/%.c
	mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

kernel.elf: $(KERNEL_OBJS)
	$(LD) $(LDFLAGS) $^ -o kernel.elf

$(ISO_DIR)/boot/kernel.elf: kernel.elf
	mkdir -p $(ISO_DIR)/boot
	cp kernel.elf $@

$(ISO_DIR)/boot/grub/grub.cfg: grub.cfg
	mkdir -p $(ISO_DIR)/boot/grub
	cp $< $@

# Create ISO with proper GRUB setup
$(ISO_DIR)/huesos.iso: $(ISO_DIR)/boot/kernel.elf $(ISO_DIR)/boot/grub/grub.cfg
	@echo "Creating ISO..."
	grub-mkrescue -o $@ $(ISO_DIR) 2>/dev/null || true

run: $(ISO_DIR)/huesos.iso
	qemu-system-i386 -m 1G -cdrom $(ISO_DIR)/huesos.iso

debug: $(ISO_DIR)/huesos.iso
	qemu-system-i386 -m 1G -no-reboot -no-shutdown -serial stdio -cdrom $(ISO_DIR)/huesos.iso

clean:
	rm -rf $(OBJDIR) kernel.elf $(ISO_DIR)

.PHONY: all run debug clean