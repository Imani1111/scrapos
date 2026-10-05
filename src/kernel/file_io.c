#include <fs.h>
#include <file_io.h>

inode_t* file_pointers[10] = {0};
int file_count = 0;

int fopen(char* filename){
	inode_t* file = fd_file(filename);
	if (file == ENTRY_NOT_FOUND) return -1;
	int fd = file_count;
	file_pointers[file_count++] = file;
	return fd;
}

/*
void fwrite(int file_no, uint8_t* content_buffer)
{
}
*/
