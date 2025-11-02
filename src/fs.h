#ifndef FS_H
#define FS_H

#include <stdint.h>
#include <stdbool.h>

#define MAX_FILES 8196
#define MAX_NAME_LEN 64
#define MAX_CONTENT_LEN 512
#define MAX_PATH_LEN 512

typedef struct {
    char name[MAX_NAME_LEN];
    char content[MAX_CONTENT_LEN];
    uint16_t size;
    bool is_directory;
} FileEntry;

extern FileEntry file_system[MAX_FILES];
extern uint16_t file_count;
extern char current_path[MAX_PATH_LEN];

void fs_init(void);
bool fs_create_file(const char* name, const char* content);
bool fs_create_directory(const char* name);
FileEntry* fs_find_entry(const char* name);
void fs_list_directory(const char* path);
bool fs_read_file(const char* name);
bool fs_remove_file(const char* name);
bool fs_remove_directory(const char* name);
bool fs_change_directory(const char* path);
const char* fs_get_current_path(void);
void vga_print_int(uint16_t num);

#endif