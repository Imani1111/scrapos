#ifndef SCREEN_H
#define SCREEN_H

#include <stddef.h>

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;


#define C64_FONT_COUNT  96
#define C64_FONT_HEIGHT 8

typedef struct __attribute__((packed)){
	uint32_t fb_phys_addr;
	uint16_t w;
	uint16_t h;
	uint8_t bpp;
	uint16_t p;
}vbemodeinfo;
#define VBE ((vbemodeinfo*)0x9500)

typedef struct {
	int min_x;
	int max_x;
	int min_y;
	int max_y;
}Cursor_bounds;

typedef struct {
	int cx;
	int cy;
}cursor_pos_t;
extern int cursor_x;
extern int cursor_y;

void clear_screen(uint32_t color);
void draw_char_at(char c, uint32_t x, uint32_t y, uint32_t color);
void draw_char(char c, uint32_t color);
void print_string_at(const char* str, uint32_t startx, uint32_t starty, uint32_t color);
void print_string(const char* str, uint32_t color);
void print_hex(uint32_t val, int start_x, int y);
void draw_block(uint32_t x, uint32_t y, uint32_t color);
void draw_cursor(void);
void toggle_cursor(uint32_t x, uint32_t y);
void move_cursor(int horizontal, int vertical);
void cursor_blink(void);
void set_cursor(int x, int y);
void draw_pixel(uint32_t x, uint32_t y, uint32_t color);
void set_cursor_bounds(int minx, int maxx, int miny, int maxy);
void store_cursor_attributes(Cursor_bounds* c, cursor_pos_t* cpos);
void restore_cursor(Cursor_bounds* c, cursor_pos_t* cpos);

#endif
