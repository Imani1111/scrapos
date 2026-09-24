#include <fs.h>
#include <disk_mgr.h>
#include <screen.h>
#include <string.h>
#include <rtc.h>
#include <ui.h>

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

int cache_dir(inode_t* dir)
{
	if (!(dir->attributes & ATTR_DIRECTORY)) return -1;

	current_dir_cache.active_extents = dir->active_extents;
	current_dir_cache.dir_inode_no = dir->inode_no;

	int curr_ext_lba;
	uint8_t ext_block[512];

	for (int i = 0; i < (int)current_dir_cache.active_extents; i++){
		curr_ext_lba = dir->exts[i].physical_block;
		dskrs2(curr_ext_lba, ext_block);

		dirent_t* dirents = (dirent_t*)ext_block;
		dirent_t* tc = current_dir_cache.entries + (i * 16);

		for (int j = 0; j < 16; j++){
			*tc++ = dirents[j];
		}
	}
	return 0;
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

void add_exts(inode_t* entry, int ext_count)
{
	for (int i = 0; i < ext_count; i++){
		uint32_t new = fds();
		use(new);
		if (new == (entry->exts[entry->active_extents - 1].physical_block + entry->exts[entry->active_extents - 1].length)){
			entry->exts[entry->active_extents - 1].length += 1;
		}
		else{
			uint32_t old_extents = entry->active_extents;
			entry->active_extents++;
			current_dir_cache.active_extents++;

			extent_t new_ext;
			new_ext.physical_block = new;
			new_ext.logical_block = old_extents;
			new_ext.length = 1;
			entry->exts[entry->active_extents - 1] = new_ext;
		}
	}
}

int find_free_dir_slot()
{
	dirent_t* temp = current_dir_cache.entries;
	int i = 2;
	int found = 0;
	while (i < MAX_DIR_ENTRIES){
		if (temp[i].name[0] == '\0'){
			found = 1;
			break;
		}
		i++;
	}
	if (!found) return DIR_FULL;
	return i;
}

void populate_inode_metadata(inode_t* inode, uint32_t inode_no, uint32_t attributes)
{
	inode->inode_no = inode_no;
	inode->attributes = attributes;
	inode->active_extents = 0;
	inode->file_size = 0;

	realtime_t* time = get_current_timestamp();
	uint64_t second = (uint64_t)time->second;
	uint64_t minute = (uint64_t)time->minute << 8;
	uint64_t hour = (uint64_t)time->hour << 16;
	uint64_t day = (uint64_t)time->day << 24;
	uint64_t month = (uint64_t)time->month << 32;
	uint64_t year = (uint64_t)time->year << 40;
	
	inode->ctime = second | minute | hour | day | month | year;
	inode->mtime = inode->ctime;
}

void init_directory(inode_t* direntry)
{
	if (!(direntry->attributes & ATTR_DIRECTORY)) return;
	int new_ext = fds();
	use(new_ext);

	extent_t e;
	e.physical_block = new_ext;
	e.logical_block = 0;
	e.length = 1;
	kmemcpy(&direntry->exts[0], &e, sizeof(extent_t));
	
	direntry->active_extents = 1;
	direntry->file_size = 512;

	dirent_t* dir = (dirent_t*)temp_dir_buf;

	dir[0].inode_no = direntry->inode_no;
	kstrcpy(dir[0].name, ".");

	dir[1].inode_no = current_dir_cache.entries[0].inode_no;
	kstrcpy(dir[1].name, "..");

	dskws2(direntry->exts[0].physical_block, (uint8_t*)temp_dir_buf);
}

int create_entry(const char* name, uint32_t attributes)
{
	inode_t* inodes = (inode_t*)inode_table;

	int i = find_free_dir_slot();
	if (i == -1) return -1;

	if (i >= (int)(current_dir_cache.active_extents * 16)){
		inode_t* parent_dir = &inodes[current_dir_cache.dir_inode_no];
		add_exts(parent_dir, 1);
	}
	
	int j = fdi();
	if (j == -1) return FS_FULL;
	iuse(j);

	inode_t* new_entry_inode = &inodes[j];
	populate_inode_metadata(new_entry_inode, j, attributes);
	if (attributes & ATTR_DIRECTORY){
		init_directory(new_entry_inode);
	}
	
	dirent_t* new_direntry = &current_dir_cache.entries[i];
	new_direntry->inode_no = j;
	kstrcpy(new_direntry->name, name);

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

inode_t* fd_dir(const char* path)
{
	char path_str[256];
	kstrcpy(path_str, path);
	
	int start_dir = 0;
	int i = 0;

	char* path_toks[32];
	int tc = kstrtok('/', path_str, path_toks, 32);
	if (kstrcmp((uint8_t*)path_toks[0], (uint8_t*)".") == 0){
		i = 1;
	}
	if (kstrcmp((uint8_t*)path_toks[0], (uint8_t*)"..") == 0){
		start_dir = 1;
		i = 1;
	}
		
	inode_t* inodes = (inode_t*)inode_table;
	dirent_t* curr_dir;

	if (start_dir){
		inode_t* parent = &inodes[current_dir_cache.entries[1].inode_no];
		grab_dir(parent);
		curr_dir = temp_dir_buf;
	}else{
		curr_dir = current_dir_cache.entries;
	}

	inode_t* target_inode;
	
	while(i < tc){
		int found = 0;
		int k = 0;
		while (k < MAX_DIR_ENTRIES){
			if (curr_dir[k].name[0] != '\0' && (kstrcmp((uint8_t*)curr_dir[k].name, (uint8_t*)path_toks[i]) == 0))
			{
				inode_t* ci = &inodes[curr_dir[k].inode_no];

				if (ci->attributes & ATTR_DIRECTORY){
					found = 1;
					grab_dir(ci);
					curr_dir = temp_dir_buf;
					target_inode = ci;
					break;
				}
			}
			k++;
		}
		if (!found){
			return ENTRY_NOT_FOUND;
		}
		i++;
	}
	return target_inode;
}

void update_cwd_str(char* path)
{
	char temp[256] = {0};
	kstrcpy(temp, path);
	char* ptr = temp;

	int in_curr_dir;
	if (*ptr == '.'){
		if (*(ptr + 1) == '.'){
			in_curr_dir = -1;
			ptr += 2;
		}else{
			in_curr_dir = 1;
			ptr++;
		}
	}else{
		in_curr_dir = 0;
	}

	if (in_curr_dir == 1){
		kstrcpy(&cwd_path[path_ptr], ptr);
		path_ptr += kstrlen(ptr);
		cwd_path[path_ptr] = '>';
	}else if (in_curr_dir == -1){
		while(cwd_path[path_ptr] && cwd_path[path_ptr] != '/'){
			cwd_path[path_ptr] = '\0';
			path_ptr--;
		}
		kstrcpy(&cwd_path[path_ptr], ptr);
		path_ptr += kstrlen(ptr);
		cwd_path[path_ptr] = '>';
	}else{
		cwd_path[path_ptr++] = '/';
		kstrcpy(&cwd_path[path_ptr], ptr);
		path_ptr += kstrlen(ptr);
		cwd_path[path_ptr] = '>';
	}
}
/*
inode_t* resolve_dir_path(char* path)
{
}
*/
int cd(char* path)
{
	inode_t* dir = fd_dir(path);
	if (dir == ENTRY_NOT_FOUND) return -1;
	if (cache_dir(dir) == -1) return -1;
	
	update_cwd_str(path);
	return 0;
}

void ls(void)
{
	dirent_t* ptr = current_dir_cache.entries;
	int i = 0;
	int total_entries = 0;
	inode_t* inode_base = (inode_t*)inode_table;
	char buf[32];
	char timebuf[20];
	while (i < MAX_DIR_ENTRIES){
		if (ptr->name[0] == '\0'){
			i++;
			ptr++;
		       	continue;
		}
		inode_t* curr = &inode_base[ptr->inode_no];
		if (curr->attributes & ATTR_DIRECTORY){
			print_string("DIR  ", TOS_COLOR_RED);
		}else{
			print_string("FILE ", 0x00ff1dce);
		}
		print_string(ptr->name, 0x00ff);
		print_string(" ", 0x0);

		realtime_t time;
		time.second = curr->ctime & 0xFF;
		time.minute = (curr->ctime >> 8) & 0xFF;
		time.hour = (curr->ctime >> 16) & 0xFF;
		time.day = (curr->ctime >> 24) & 0xFF;
		time.month = (curr->ctime >> 32) & 0xFF;
		time.year = (curr->ctime >> 40) & 0xFF;

		format_time(&time, timebuf);
		print_string(timebuf, 0x00ff);
		print_string(" ", 0);
		itoa(curr->file_size, buf);
		print_string(buf, TOS_COLOR_RED);
		print_string("Bytes", 0x00228b22);
		draw_char('\n', 0);
		
		kmemset(buf, 0, 32);
		ptr++;
		i++;
		total_entries++;
	}
	print_string("Total: ", 0x0);
	itoa(total_entries, buf);
	print_string(buf, 0x00ff0000);
	draw_char('\n', 0);
}

void print_shell_prompt()
{
	print_string(cwd_path, 0x00ff);
}
