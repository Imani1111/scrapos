#include <stdio.h>
#include <stdint.h>
#include <string.h>

typedef struct {
	uint32_t magic;
	uint32_t root_inode;
	uint32_t sector_bitmap;
	uint32_t inode_bitmap;
	uint32_t data_sea;
}SuperBlock;

typedef struct {
	uint32_t logical_block;
	uint32_t physical_block;
	uint32_t length;
}__attribute__((packed)) extent_t;

#define EXTENT_COUNT 8
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

#define ATTR_FILE (0 << 0)
#define ATTR_DIRECTORY (1 << 0)
#define ATTR_READONLY (1 << 1)
#define ATTR_HIDDEN (1 << 2)
#define ATTR_READABLE (1 << 3)
#define ATTR_WRITABLE (1 << 4)
#define ATTR_EXECUTABLE (1 << 5)

inode_t inode_table[512] = {0};

int main()
{
	uint8_t sector[512] = {0};

	SuperBlock* sb = (SuperBlock*)sector;
	sb->magic = 0xEEEEEEEE;
	sb->root_inode = 1;
	sb->sector_bitmap = 131;
	sb->inode_bitmap = 135;
	sb->data_sea = 137;

	FILE* f = fopen("sb.bin", "wb");
	fwrite(sector, 1, 512, f);
	fclose(f);
	printf("==> sb created\n");
	
	inode_table[0].inode_no = 0;
	inode_table[0].file_size = 512;
	uint16_t entry_count = 1;
	inode_table[0].attributes = ATTR_DIRECTORY | ATTR_READABLE | ATTR_WRITABLE;
	inode_table[0].ctime = 0;
	inode_table[0].mtime = 0;
	inode_table[0].active_extents = 1;
	extent_t rr;
	rr.logical_block = 0;
	rr.physical_block = 137;
	rr.length = 1;
	inode_table[0].exts[0] = rr;
	
	printf("Size of inode_t: %lu bytes\n", sizeof(inode_t));
	FILE* root = fopen("inodet.bin", "wb");
	fwrite(inode_table, 1, sizeof(inode_t) * 512, root);
	fclose(root);

	uint8_t rt[512] = {0};
	dirent_t* r = (dirent_t*)rt;
	char* name = ".";
	strcpy(r->name, name);
	r->inode_no = 0;
	FILE* root_dir = fopen("root.bin", "wb");
	fwrite(rt, 1, 512, root_dir);
	fclose(root_dir);

	uint8_t sbm[2560] = {0};
	for (int i = 0; i < 17; i++){
		sbm[i] = 0xff;
	}
	sbm[17] = 0b00000011;
	FILE* stbm = fopen("sbm.bin", "wb");
	fwrite(sbm, 1, 2560, stbm);
	fclose(stbm);

	uint8_t idt[512] = {0};
	idt[0] = 0b00000001;
	FILE* itbm = fopen("itbm.bin", "wb");
	fwrite(idt, 1, 512, itbm);
	fclose(itbm);


	return 0;
}
