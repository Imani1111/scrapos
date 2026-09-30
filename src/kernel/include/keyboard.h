#ifndef KEYBOARD_H
#define KEYBOARD_H

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;

#define KEYBOARD_DATA_PORT 0x60
#define MAX_KEYS 64

#define LEFT_SHIFT 0x2A
#define RIGHT_SHIFT 0x36
#define CAPS_LOCK 0x3A
#define UP 0x48
#define DOWN 0x50
#define LEFT 0x4B
#define RIGHT 0x4D
#define WINDOWS_BUTTON 0x5B
#define LEFT_CTRL 0x1D

typedef struct {
	char key_buffer[MAX_KEYS];
	uint8_t writing_index;
	uint8_t reading_index;
}KeyboardKeyCollector;

void HandleKeyboardInterrupt(void);
char read_key(void);

#endif
