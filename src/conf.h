#ifndef CONF_H
#define CONF_H

//  Kernel Settings
#define KERNEL_NAME "HuesOS"
#define KERNEL_VERSION "0.0.1"
#define KERNEL_AUTHOR "kurkUma"

// Kernel memory layout
#define KERNEL_BASE_ADDR 0x00100000 // 1 MB
#define KERNEL_STACK_SIZE 0x4000    // 16 KB

//  Hardware Configuration
#define VIDEO_TEXT_MODE 1
#define VGA_WIDTH 80
#define VGA_HEIGHT 25

#define PIT_FREQUENCY_HZ 100
#define KEYBOARD_ENABLED 1
#define SERIAL_ENABLED 0

//  Filesystem / Disk
#define ROOT_DEVICE "hd0"
#define FS_BLOCK_SIZE 512

//  User Configuration
#define ROOT_USER "root"

//  Debugging & Logging
#define ENABLE_DEBUG 1
#define ENABLE_ASSERTS 1

//  UI / Prompt
#define PROMPT_STRING "[root@HuesOS"

//  Feature Flags
#define FEATURE_SHELL 1
#define FEATURE_KEYBOARD 1
#define FEATURE_PAGING 0
#define FEATURE_MULTITASK 0

#endif
