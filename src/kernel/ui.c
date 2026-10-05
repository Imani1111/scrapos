#include <ui.h>
#include <screen.h>
#include <string.h>
#include <task_mgr.h>
#include <keyboard.h>
#include <mem_mgr.h>
#include <shell.h>

extern TaskControlBlock_t* current_task;
extern int next_pid;

Window_t* bottom_window = NULL;
Window_t* top_window = NULL;

void draw_rect(uint32_t x, uint32_t y, uint16_t w, uint16_t h, uint32_t color)
{
	for(uint16_t r = 0; r < h; r++){
		for (uint16_t c = 0; c < w; c++){
			draw_pixel(x+c, y+r, color);
		}
	}
}

void outline_rect(uint32_t x, uint32_t y, uint16_t w, uint16_t h, uint8_t outline_thickness, uint32_t color)
{
	for (uint16_t r = 0; r < h; r++){
		for (uint16_t c = 0; c < w; c++){
			if (c < outline_thickness || c > (w - outline_thickness) || r < outline_thickness || r > (h - outline_thickness)){
				draw_pixel(x + c, y + r, color);
			}
		}
	}
}

void draw_active_title_bar(Window_t* win)
{
	draw_rect(win->x + 2, win->y + 2, win->w - 4, 12, win->fc);
	print_string_at(win->name, win->x + 6, win->y + 3, win->tc);
	print_string_at("[X]", win->x + (win->w - 32), win->y + 3, 0x00ff0000);
}

void draw_inactive_title_bar(Window_t* win)
{
	draw_rect(win->x + 2, win->y + 2, win->w - 4, 12, INACTIVE_TITLE_BAR);
	print_string_at(win->name, win->x + 6, win->y + 3, TOS_COLOR_YELLOW);
}

Window_t* create_window(uint32_t x, uint32_t y, uint16_t width, uint16_t height, uint32_t window_color, uint32_t frame_color, uint32_t title_color, const char* title)
{
	Window_t* new_win = my_malloc(sizeof(Window_t));
	if (new_win == NULL) return NULL;
	new_win->x = x;
	new_win->y = y;
	new_win->w = width;
	new_win->h = height;
	new_win->bg = window_color;
	new_win->fc = frame_color;
	new_win->tc = title_color;
	kstrcpy(new_win->name, title);
	new_win->pid = current_task->pid;
	new_win->vault = my_malloc(new_win->w * new_win->h * 3);
	if (new_win->vault == NULL) return NULL;
	store_cursor_attributes(&new_win->cb, &new_win->cpos);
	
	if (bottom_window == NULL){
		bottom_window = new_win;
		top_window = new_win;
		bottom_window->next = new_win;
		bottom_window->prev = new_win;
		top_window->next = bottom_window;
		top_window->prev = bottom_window;
	}else{
		new_win->next = bottom_window;
		bottom_window->prev = new_win;
		new_win->prev = top_window;
		top_window->next = new_win;
		top_window = new_win;
	}	
	return new_win;
}

void draw_window(Window_t* win)
{
	draw_rect(win->x, win->y, win->w, win->h, win->bg);
	outline_rect(win->x, win->y, win->w, win->h, 4, win->fc);
	int len = kstrlen(win->name);
	draw_rect(win->x, win->y, len * 8, 16, win->fc);
	print_string_at(win->name, win->x, win->y + 4, 0x0000ff00);
}

void take_snapshot(Window_t* win)
{
	volatile uint8_t* fb = (volatile uint8_t*)VBE->fb_phys_addr;
	uint16_t p = (uint16_t)VBE->p;
	uint32_t rb = win->w * 3;
	
	uint8_t* dest = win->vault;
		
	for (uint16_t r = 0; r < win->h; r++){
		uint32_t ri = ((win->y + r) * p) + (win->x * 3);
		volatile uint8_t* src = &fb[ri];
		kmemcpy(dest, (const void*)src, rb);
		dest += rb;
	}
}

void redraw_snapshot(Window_t* win)
{
	uint32_t rb = win->w * 3;
	volatile uint8_t* fb = (volatile uint8_t*)VBE->fb_phys_addr;
	uint16_t p = (uint16_t)VBE->p;

	uint8_t* src = win->vault;
	for (uint32_t r = 0; r < win->h; r++){
		uint32_t ri = ((win->y + r) * p) + (win->x * 3);
		volatile uint8_t* dest = &fb[ri];
		kmemcpy((void*)dest, (const void*)src, rb);
		src += rb;
	}
}

Window_t* open_window(uint32_t x, uint32_t y, uint16_t w, uint16_t h, uint32_t bg, uint32_t fc, uint32_t tc, const char* t)
{
	Window_t* win = create_window(x, y, w, h, bg, fc, tc, t);
	if (win == NULL){
		print_string("Window creation failed!\n", 0x00ff0000);
		asm volatile("sti");
		return NULL;
	}

	take_snapshot(win);
	draw_window(win);
	TaskControlBlock_t* window_task = get_task_tcb(win->pid);
	create_task_bar_entry(window_task);
	return win;
}

void close_window(Window_t* win)
{
	redraw_snapshot(win);
	TaskControlBlock_t* window_task = get_task_tcb(win->pid);
	update_task_bar_entry(window_task, -1);
	restore_cursor(&win->cb, &win->cpos);

	my_free(win->vault);
	my_free(win);
}
