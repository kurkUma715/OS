#include "fs.h"
#include "vga.h"
#include "commands.h"

FileEntry file_system[MAX_FILES] = {0};
uint16_t file_count = 0;
char current_path[MAX_PATH_LEN] = "/";

#define NULL ((void *)0)

void fs_init(void)
{
    fs_create_file("readme.txt", "Welcome to HuesOS File System!\nThis is a simple file system.");
    fs_create_file("kernel.bin", "Kernel binary data");
    fs_create_directory("home");
    fs_create_directory("etc");
    fs_create_directory("bin");
}

bool fs_create_file(const char *name, const char *content)
{
    if (file_count >= MAX_FILES)
    {
        vga_print("Error: File system full\n");
        return false;
    }

    for (uint16_t i = 0; i < file_count; i++)
    {
        if (command_strcmp(file_system[i].name, name) == 0)
        {
            vga_print("Error: File already exists\n");
            return false;
        }
    }

    FileEntry *file = &file_system[file_count];

    uint16_t name_len = 0;
    while (name[name_len] && name_len < MAX_NAME_LEN - 1)
    {
        file->name[name_len] = name[name_len];
        name_len++;
    }
    file->name[name_len] = 0;

    uint16_t content_len = 0;
    if (content)
    {
        while (content[content_len] && content_len < MAX_CONTENT_LEN - 1)
        {
            file->content[content_len] = content[content_len];
            content_len++;
        }
    }
    file->content[content_len] = 0;
    file->size = content_len;
    file->is_directory = false;

    file_count++;
    return true;
}

bool fs_create_directory(const char *name)
{
    if (file_count >= MAX_FILES)
    {
        vga_print("Error: File system full\n");
        return false;
    }

    for (uint16_t i = 0; i < file_count; i++)
    {
        if (command_strcmp(file_system[i].name, name) == 0)
        {
            vga_print("Error: Directory already exists\n");
            return false;
        }
    }

    FileEntry *dir = &file_system[file_count];

    uint16_t name_len = 0;
    while (name[name_len] && name_len < MAX_NAME_LEN - 1)
    {
        dir->name[name_len] = name[name_len];
        name_len++;
    }
    dir->name[name_len] = 0;

    dir->content[0] = 0;
    dir->size = 0;
    dir->is_directory = true;

    file_count++;
    return true;
}

FileEntry *fs_find_entry(const char *name)
{
    for (uint16_t i = 0; i < file_count; i++)
    {
        if (command_strcmp(file_system[i].name, name) == 0)
        {
            return &file_system[i];
        }
    }
    return NULL;
}

void fs_list_directory(const char *path)
{
    if (path[0] != '/' || command_strcmp(path, "/") != 0)
    {
        vga_print("Error: Only root directory supported in this version\n");
        return;
    }

    vga_print("Type    Size    Name\n");
    vga_print("-------------------\n");

    for (uint16_t i = 0; i < file_count; i++)
    {
        if (file_system[i].is_directory)
        {
            vga_print("dir            ");
        }
        else
        {
            vga_print("file     ");

            if (file_system[i].size < 10)
                vga_print(" ");
            if (file_system[i].size < 100)
                vga_print(" ");
            if (file_system[i].size < 1000)
                vga_print(" ");
            vga_print_int(file_system[i].size);
            vga_print(" ");
        }

        vga_print(file_system[i].name);
        vga_print("\n");
    }
}

bool fs_read_file(const char *name)
{
    FileEntry *file = fs_find_entry(name);
    if (!file)
    {
        vga_print("Error: File not found\n");
        return false;
    }

    if (file->is_directory)
    {
        vga_print("Error: Is a directory\n");
        return false;
    }

    vga_print(file->content);
    vga_print("\n");
    return true;
}

bool fs_remove_file(const char *name)
{
    for (uint16_t i = 0; i < file_count; i++)
    {
        if (command_strcmp(file_system[i].name, name) == 0)
        {
            if (file_system[i].is_directory)
            {
                vga_print("Error: Use rmdir for directories\n");
                return false;
            }

            for (uint16_t j = i; j < file_count - 1; j++)
            {
                file_system[j] = file_system[j + 1];
            }
            file_count--;
            return true;
        }
    }

    vga_print("Error: File not found\n");
    return false;
}

bool fs_remove_directory(const char *name)
{
    for (uint16_t i = 0; i < file_count; i++)
    {
        if (command_strcmp(file_system[i].name, name) == 0)
        {
            if (!file_system[i].is_directory)
            {
                vga_print("Error: Use rm for files\n");
                return false;
            }

            for (uint16_t j = i; j < file_count - 1; j++)
            {
                file_system[j] = file_system[j + 1];
            }
            file_count--;
            return true;
        }
    }

    vga_print("Error: Directory not found\n");
    return false;
}

bool fs_change_directory(const char *path)
{
    if (command_strcmp(path, "/") == 0)
    {
        current_path[0] = '/';
        current_path[1] = 0;
        return true;
    }

    if (command_strcmp(path, "..") == 0)
    {
        current_path[0] = '/';
        current_path[1] = 0;
        return true;
    }

    FileEntry *dir = fs_find_entry(path);
    if (dir && dir->is_directory)
    {
        current_path[0] = '/';
        uint16_t i = 0;
        while (path[i] && i < MAX_PATH_LEN - 2)
        {
            current_path[i + 1] = path[i];
            i++;
        }
        current_path[i + 1] = 0;
        return true;
    }

    vga_print("Error: Directory not found: ");
    vga_print(path);
    vga_print("\n");
    return false;
}

const char *fs_get_current_path(void)
{
    return current_path;
}

void vga_print_int(uint16_t num)
{
    if (num == 0)
    {
        vga_putchar('0');
        return;
    }

    char buffer[6];
    uint8_t i = 0;

    while (num > 0)
    {
        buffer[i++] = '0' + (num % 10);
        num /= 10;
    }

    for (uint8_t j = i; j > 0; j--)
    {
        vga_putchar(buffer[j - 1]);
    }
}