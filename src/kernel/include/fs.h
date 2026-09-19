#ifndef FS_H
#define FS_H

#include <stddef.h>

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;

#define FS_MAGIC 0xEEEEEEEE
#define BITMAP (20480 / 8)
#define INODES 512

typedef struct {
	uint32_t magic;
	uint32_t root_inode;
	uint32_t sector_bitmap;
	uint32_t inode_bitmap;
	uint32_t data_sea;
}superblock_t;

typedef struct {
	uint32_t logical_block;
	uint32_t physical_block;
	uint32_t length;
}__attribute__((packed)) extent_t;

#define EXTENT_COUNT 8
#define MAX_DIR_ENTRIES 128
#define MAX_LENGTH 28
typedef struct {
	uint32_t inode_no;
	uint32_t file_size;
	uint32_t attributes;
	uint64_t ctime;
	uint64_t mtime;
	uint32_t active_extents;
	extent_t exts[EXTENT_COUNT];
}__attribute__((packed)) inode_t;

typedef struct {
	char name[MAX_LENGTH];
	uint32_t inode_no;
}dirent_t;

typedef struct {
	uint32_t active_extents;
	uint32_t dir_inode_no;
	dirent_t entries[MAX_DIR_ENTRIES];
}cwd_cache;

#define ATTR_FILE (0 << 0)
#define ATTR_DIRECTORY (1 << 0)
#define ATTR_READONLY (1 << 1)
#define ATTR_HIDDEN (1 << 2)
#define ATTR_READABLE (1 << 3)
#define ATTR_WRITABLE (1 << 4)
#define ATTR_EXECUTABLE (1 << 5)

#define FS_FULL -2
#define DIR_FULL -1
#define INVALID_PATH -1
#define ENTRY_NOT_FOUND ((inode_t*)-1)
#define MAX_TOKENS 32

void init_fs(void);
int create_entry(const char* name, uint32_t attributes);
void cache_dir(inode_t* dir);
void grab_dir(inode_t* dir);
int fd_dir(const char* name);
int cd(char* path);
inode_t* resolve_dir_path(char* path);
void print_shell_prompt(void);
#endif
