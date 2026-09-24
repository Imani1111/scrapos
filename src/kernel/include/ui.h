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
#define WINDOW_BORDER       0x404040
#define WINDOW_BACKGROUND   0xE0E0E0

#define MAX_WINDOW_DEPTH 4
#define SIZE_OF_MAX_WINDOW (400*300*3)

#define OCCUPIED 1

typedef struct Window{
	uint32_t x;
	uint32_t y;
	uint16_t w;
	uint16_t h;
	uint32_t bg;
	uint32_t fc;
	const char* name;
	int pid;
	uint8_t vault[SIZE_OF_MAX_WINDOW];
	int win_idx;
	int winspace;
}Window_t;

extern Window_t windows[MAX_WINDOW_DEPTH];

void draw_rect(uint32_t x, uint32_t y, uint16_t w, uint16_t h, uint32_t color);
void outline_rect(uint32_t x, uint32_t y, uint16_t w, uint16_t h, uint8_t outline_thickness, uint32_t color);
void draw_active_title_bar(Window_t* win);
void draw_inactive_title_bar(Window_t* win);

Window_t* create_window(uint32_t x, uint32_t y, uint16_t width, uint16_t height, uint32_t window_color, uint32_t frame_color, const char* title);
void take_snapshot(Window_t* win);
void redraw_snapshot(Window_t* win);
Window_t* open_window(uint32_t x, uint32_t y, uint16_t w, uint16_t h, uint32_t bg, uint32_t fc, const char* t);
void close_window(Window_t* win);
void bring_to_top(Window_t* win);
void draw_reordered(void);
void focus_window(Window_t* win);
void cycle_win();
void InitDesktop(void);
void start_menu(void);
#endif
