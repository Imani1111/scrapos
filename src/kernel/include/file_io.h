#ifndef FILEIO_H
#define FILEIO_H

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;

int fopen(char* filename);
int fwrite(int file_no, uint8_t* buffer);

#endif
