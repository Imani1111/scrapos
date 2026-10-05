#ifndef UI_H
#define UI_H

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;

#include <screen.h>

#define TOS_COLOR_BLUE      0x0000AA  
#define TOS_COLOR_CYAN      0x00AAAA  
#define TOS_COLOR_WHITE     0xFFFFFF  
#define TOS_COLOR_DARK_GRAY 0x555555  
#define TOS_COLOR_YELLOW    0xFFFF55  
#define TOS_COLOR_RED       0xAA0000  

#define ACTIVE_TITLE_BAR    0x000080
#define INACTIVE_TITLE_BAR  0x808080

typedef struct Window{
	uint32_t x;
	uint32_t y;
	uint16_t w;
	uint16_t h;
	uint32_t bg;
	uint32_t fc;
	uint32_t tc;
	const char name[16];
	int pid;
	uint8_t* vault;
	Cursor_bounds cb;
	cursor_pos_t cpos;
	struct Window* next;
	struct Window* prev;
}Window_t;

void draw_rect(uint32_t x, uint32_t y, uint16_t w, uint16_t h, uint32_t color);
void outline_rect(uint32_t x, uint32_t y, uint16_t w, uint16_t h, uint8_t outline_thickness, uint32_t color);
void draw_active_title_bar(Window_t* win);
void draw_inactive_title_bar(Window_t* win);

Window_t* create_window(uint32_t x, uint32_t y, uint16_t width, uint16_t height, uint32_t window_color, uint32_t frame_color, uint32_t title_color, const char* title);
void draw_window(Window_t* win);
void take_snapshot(Window_t* win);
void redraw_snapshot(Window_t* win);
Window_t* open_window(uint32_t x, uint32_t y, uint16_t w, uint16_t h, uint32_t bg, uint32_t fc, uint32_t tc, const char* t);
void close_window(Window_t* win);


#endif
