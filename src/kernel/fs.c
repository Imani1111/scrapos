#include <fs.h>
#include <disk_mgr.h>
#include <screen.h>
#include <string.h>
#include <rtc.h>

uint8_t b[BITMAP];
uint8_t inodebm[512];
uint8_t inode_table[INODES * 128];

dirent_t temp_dir_buf[MAX_DIR_ENTRIES];
cwd_cache current_dir_cache;
char cwd_path[1024] = {0};
int path_ptr = 0;

void init_fs()
{
	uint8_t s[512];
	uint32_t inodes;
	uint32_t sector_bitmap;
	uint32_t inode_bitmap;
	uint32_t data_sea;
	dskrs2(0, s);
	superblock_t* sb = (superblock_t*)s;
	if (sb->magic != FS_MAGIC){
		print_string("File system not found!\n", 0x00ff0000);
		return;
	}
	inodes = sb->root_inode;
	sector_bitmap = sb->sector_bitmap;
	inode_bitmap = sb->inode_bitmap;
	data_sea = sb->data_sea;

	uint32_t* it = (uint32_t*)inode_table;
	for (int i = 0; i < 128; i++){
		dskrs2(inodes + i, s);
		uint32_t* sector_words = (uint32_t*)s;
		for (int j = 0; j < 128; j++){
			*it++ = sector_words[j];
		}
	}
	
	uint32_t* sbm = (uint32_t*)b;
	for (int i = 0; i < 5; i++){
		dskrs2(sector_bitmap + i, s);
		uint32_t* sector_words = (uint32_t*)s;
		for (int j = 0; j < 128; j++){
			*sbm++ = sector_words[j];
		}
	}
	
	inode_t* root_dir = (inode_t*)inode_table;
	cache_dir(root_dir);
	kstrcpy(&cwd_path[path_ptr], "C:");
	path_ptr += kstrlen("C:");
	cwd_path[path_ptr] = '>';

	dskrs2(inode_bitmap, inodebm);
	print_string("Everything still works!\n", 0x00ff);
}

void cache_dir(inode_t* dir)
{
	if (!(dir->attributes & ATTR_DIRECTORY)) return;

	current_dir_cache.active_extents = dir->active_extents;
	current_dir_cache.dir_inode_no = dir->inode_no;

	int curr_ext_lba;
	uint8_t ext_block[512];
	for (int i = 0; i < (int)current_dir_cache.active_extents; i++){
		curr_ext_lba = dir->exts[i].physical_block;
		dskrs2(curr_ext_lba, ext_block);
		dirent_t* dirents = (dirent_t*)ext_block;
		dirent_t* tc = current_dir_cache.entries;
		for (int j = 0; j < (int)(512 / sizeof(dirent_t)); j++){
			*tc++ = dirents[j];
		}
	}
}

void use(int n){
	int x = n / 8;
	int y= n % 8;
	if (!(b[x] & (1 << y))){
		b[x] |= (1 << y);
	}
}

void freeup(int n){
	int x = n / 8;
	int y= n % 8;
	if (b[x] & (1 << y)){
		b[x] &= (0 << y);
	}
}

int fds()
{
	uint32_t* bm = (uint32_t*)b;
	for (int i = 0; i < 640; i++){
		if (bm[i] == 0xffffffff) continue;
		for (int j = 0; j < 32; j++){
			if (!(bm[i] & (1 << j))){
				int s = (i * 32) + j;
				return s;
			}
		}
	}
	return -1;
}

void iuse(int inum)
{
	int x = inum / 8;
	int y = inum % 8;
	if (!(inodebm[x] & (1 << y))){
		inodebm[x] |= (1 << y);
	}
}

void ifree(int inum)
{
	int x = inum / 8;
	int y = inum % 8;
	if (inodebm[x] & (1 << y)){
		inodebm[x] &= (0 << y);
	}
}

int fdi()
{
	uint32_t* it = (uint32_t*)inodebm;
	for (int i = 0; i < 16; i++){
		if (it[i] == 0xffffffff) continue;
		for (int j = 0; j < 32; j++){
			if (!(it[i] & (1 << j))){
				int inum = (i * 32) + j;
				return inum;
			}
		}
	}
	return -1;
}

int create_entry(const char* name, uint32_t attributes)
{
	int inode_no = fdi();
	if (!inode_no) return FS_FULL;
	inode_t* inodes = (inode_t*)inode_table;
	inode_t* new_dir = &inodes[inode_no];

	dirent_t* dir_entries = current_dir_cache.entries;
	int parent_dir_inode_no = dir_entries->inode_no;
	int found = 0;
	uint32_t active_extents = current_dir_cache.active_extents;
	dirent_t* new_dir_entry = &dir_entries[2];
	uint32_t i = 3;
	while (i < MAX_DIR_ENTRIES){
		if (new_dir_entry->name[0] == '\0'){
			found = 1;
			break;
		}
		new_dir_entry++;
		i++;
	}
	if (!found) return DIR_FULL;

	if (i > (active_extents * 16)){
		uint32_t new_extent = fds();
		inode_t* parent = &inodes[current_dir_cache.dir_inode_no];

		if (new_extent == (parent->exts[active_extents - 1].physical_block + parent->exts[active_extents - 1].length)){
			parent->exts[active_extents - 1].length += 1;
		}else{
			parent->active_extents += 1;
			extent_t new;
			new.physical_block = new_extent;
			new.logical_block = active_extents;
			new.length = 1;
			parent->exts[active_extents] = new;
			use(new_extent);
		}
	}

	new_dir->inode_no = inode_no;
	new_dir->file_size = 0;
	new_dir->attributes = attributes;
	realtime_t* time = get_current_timestamp();
	uint64_t second = (uint64_t)time->second;
	uint64_t minute = (uint64_t)time->minute << 8;
	uint64_t hour = (uint64_t)time->hour << 16;
	uint64_t day = (uint64_t)time->day << 24;
	uint64_t month = (uint64_t)time->month << 32;
	uint64_t year = (uint64_t)time->year << 40;
	new_dir->ctime = second | minute | hour | day | month | year;
	new_dir->mtime = new_dir->ctime;
	new_dir->active_extents = 0;

	kstrcpy(new_dir_entry->name, name);
	new_dir_entry->inode_no = inode_no;

	if (attributes & ATTR_DIRECTORY){
		grab_dir(new_dir);
		dirent_t* direntries = temp_dir_buf;
		char* names = ".";
		kmemcpy(direntries->name, names, kstrlen(names));
		direntries->inode_no = inode_no;
		names = "..";
		kmemcpy((direntries + 1)->name, names, kstrlen(names));
		(direntries + 1)->inode_no = parent_dir_inode_no;
	}

	return 0;
}

void grab_dir(inode_t* dir)
{
	if (!(dir->attributes & ATTR_DIRECTORY)) return;
	dirent_t* tmp = temp_dir_buf;
	uint8_t extent_block[512] = {0};
	for (int i = 0; i < (int)dir->active_extents; i++){
		if (dir->exts[i].length > 1){
			for (int j = 0; j < (int)dir->exts[i].length; j++){
				dskrs2(dir->exts[i].physical_block + j, extent_block);
				dirent_t* dirs = (dirent_t*)extent_block;
				for (int k = 0; k < 16; k++){
					*tmp++ = dirs[k];
				}
			}
		}
		else{
			dskrs2(dir->exts[i].physical_block, extent_block);
			dirent_t* dirs = (dirent_t*)extent_block;
			for (int l = 0; l < 16; l++){
				*tmp++ = dirs[l];
			}
		}
	}
}

inode_t* resolve_dir_path(char* path)
{
	if (*path == '\0') return ENTRY_NOT_FOUND;
	char* tokens[MAX_TOKENS];
	int tc = kstrtok('/', path, tokens, MAX_TOKENS);
	int found;
	inode_t* inodes = (inode_t*)inode_table;
	inode_t* target;
	dirent_t* curr_dir = current_dir_cache.entries;
	int start_dir = 0;
	for (int i = 0; i < tc; i++){
		if (i == 0 && (kstrcmp((uint8_t*)tokens[i], (uint8_t*)".") == 0)){
			start_dir = 1;
			continue;
		}
		else if (i == 0 && (kstrcmp((uint8_t*)tokens[i], (uint8_t*)"..") == 0)){
			start_dir = -1;
			continue;
		}
		found = 0;
		switch (start_dir){
			case 1: {
				int k = 0;
				while (k < MAX_DIR_ENTRIES){
					if (kstrcmp((uint8_t*)curr_dir->name, (uint8_t*)tokens[i]) == 0){
						inode_t* di = &inodes[curr_dir->inode_no];
						if (di->attributes & ATTR_DIRECTORY){
							found = 1;
							grab_dir(di);
							target = di;
							curr_dir = temp_dir_buf;
							break;
						}
					}
					curr_dir++;
					k++;
				}
				break;
			}
			case -1: {
				dirent_t* prev_dir = &curr_dir[1];
				inode_t* prev = &inodes[prev_dir->inode_no];
				grab_dir(prev);
				curr_dir = temp_dir_buf;
				int k = 0;
				while (k < MAX_DIR_ENTRIES){
					if (kstrcmp((uint8_t*)curr_dir->name, (uint8_t*)tokens[i]) == 0){
						inode_t* di = &inodes[curr_dir->inode_no];
						if (di->attributes & ATTR_DIRECTORY){
							found = 1;
							grab_dir(di);
							target = di;
							curr_dir = temp_dir_buf;
							break;
						}
					}
					curr_dir++;
					k++;
				}
				break;
			}
		}
		if (!found) return ENTRY_NOT_FOUND;
	}
	return target;
}

int fd_dir(const char* name)
{
	dirent_t* dir = current_dir_cache.entries;
	int i = 0;
	while (i < MAX_DIR_ENTRIES){
		if (kstrcmp((uint8_t*)dir->name, (uint8_t*)name) == 0){
			return 0;
		}
		dir++;
		i++;
	}
	return -1;
}

int cd(char* path)
{
	inode_t* dir = resolve_dir_path(path);
	if (dir == ENTRY_NOT_FOUND) return -1;
	cache_dir(dir);

	int is_curr_dir;
	char* ptr = path;
	while (*ptr){
		if (*ptr == '.'){
			if (*(ptr + 1) == '.'){
				is_curr_dir = 0;
				ptr += 2;
				break;
			}else{
				is_curr_dir = 1;
				ptr++;
				break;
			}
		}
	}
	if (is_curr_dir){
		kstrcpy(&cwd_path[path_ptr], ptr);
		path_ptr += kstrlen(ptr);
		cwd_path[path_ptr] = '>';
	}else{
		char* dirs[10];
		int dir_level = kstrtok('/', cwd_path, dirs, 10);
		if (dir_level == 1) return -1;

		while (cwd_path[path_ptr] && cwd_path[path_ptr] != '/'){
			cwd_path[path_ptr] = '\0';
			path_ptr--;
		}
		kstrcpy(&cwd_path[path_ptr], ptr);
		path_ptr += kstrlen(ptr);
		cwd_path[path_ptr] = '>';
	}
	return 0;
}

void print_shell_prompt()
{
	print_string(cwd_path, 0x00ff);
}
