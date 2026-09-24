#include <ui.h>
#include <screen.h>
#include <string.h>
#include <task_mgr.h>
#include <keyboard.h>
#include <shell.h>

extern TaskControlBlock_t* current_task;
extern int next_pid;
extern TaskControlBlock_t* current_task;
extern int cursor_x;
extern int cursor_y;

int start_menu_open = 0;

Window_t windows[MAX_WINDOW_DEPTH];
int z_order[MAX_WINDOW_DEPTH];
int z_count = 0;

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
	draw_rect(win->x + 2, win->y + 2, win->w - 4, 12, ACTIVE_TITLE_BAR);
	print_string_at(win->name, win->x + 6, win->y + 3, 0x0000FF00);
}

void draw_inactive_title_bar(Window_t* win)
{
	draw_rect(win->x + 2, win->y + 2, win->w - 4, 12, INACTIVE_TITLE_BAR);
	print_string_at(win->name, win->x + 6, win->y + 3, TOS_COLOR_YELLOW);
}
/*
void handle_bounds_error(){
	asm volatile("cli");
	draw_window(150, 150, 300, 40, TOS_COLOR_YELLOW, TOS_COLOR_RED, "Window manager");
	print_string_at("Out of Bounds window detected!", 158, 170, TOS_COLOR_RED);
	while(1){
		char c = read_key();
		if (c == 27){
			redraw_previous_snapshot(150, 150, 300, 40);
			KillTask(NULL);
			asm volatile("sti");
			return;
		}
	}
}
*/
Window_t* create_window(uint32_t x, uint32_t y, uint16_t width, uint16_t height, uint32_t window_color, uint32_t frame_color, const char* title){
	for (int i = 0; i < MAX_WINDOW_DEPTH; i++){
		if (windows[i].winspace != OCCUPIED){
			windows[i].x = x;
			windows[i].y = y;
			windows[i].w = width;
			windows[i].h = height;
			windows[i].name = title;
			windows[i].pid = current_task->pid;
			windows[i].bg = window_color;
			windows[i].fc = frame_color;
			windows[i].win_idx = i;
			windows[i].winspace = OCCUPIED;

			z_order[z_count++] = i;
			return (&windows[i]);	
		}
	}
	return NULL;
}

void draw_window(Window_t* win)
{
	draw_rect(win->x, win->y, win->w, win->h, win->bg);
	outline_rect(win->x, win->y, win->w, win->h, 2, win->fc);
	draw_active_title_bar(win);
}

void take_snapshot(Window_t* win)
{
	asm volatile("cli");
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
	asm volatile("sti");
}

void redraw_snapshot(Window_t* win)
{
	asm volatile("cli");
	
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

	asm volatile("sti");
}

Window_t* open_window(uint32_t x, uint32_t y, uint16_t w, uint16_t h, uint32_t bg, uint32_t fc, const char* t)
{
	asm volatile("cli");
	Window_t* win = create_window(x, y, w, h, bg, fc, t);
	if (win == NULL){
		print_string("Window Limit Reached!\n", 0x00ff0000);
		asm volatile("sti");
		return NULL;;
	}
	
	take_snapshot(win);
	draw_window(win);
	focus_window(win);
	asm volatile("sti");
	return win;
}

void bring_to_top(Window_t* win)
{
	int idx;
	for (int i = 0; i < z_count - 1; i++){
		if (z_order[i] == win->win_idx){
			idx = i;
			break;
		}
	}
	for (int i = idx; i < z_count - 1; i++){
		z_order[i] = z_order[i + 1];
	}
	z_order[z_count - 1] = win->win_idx;
}

void focus_window(Window_t* win)
{
	for (int i = 0; i < z_count; i++){
		int win_idx = z_order[i];
		Window_t* current_win = &windows[win_idx];
		int win_pid = current_win->pid;

		if (win_pid == win->pid){
			set_task_state(win_pid, TASK_READY);
		}else{
			set_task_state(win_pid, TASK_BLOCKED);
		}	
	}
}

void draw_reordered()
{
	for (int i = 0; i < z_count; i++){
		redraw_snapshot(&windows[z_order[i]]);
		take_snapshot(&windows[z_order[i]]);
		draw_window(&windows[z_order[i]]);
	}
}

void cycle_win(){
	if (z_count < 1) return;
	Window_t* next = &windows[z_order[0]];
	bring_to_top(next);
	draw_reordered();
	focus_window(&windows[z_order[z_count - 1]]);
}

void close_window(Window_t* win)
{
	asm volatile("cli");
	redraw_snapshot(win);
	bring_to_top(win);
	z_count--;
	focus_window(&windows[z_order[z_count - 1]]);
	//draw_reordered();
	kmemset((void*)win, 0, sizeof(Window_t));
	asm volatile("sti");
}

void test(){
	Window_t* win = open_window(150, 100, 200, 100, 0x00000000, 0x00ffffff, "TEST");
	while(1){
		char c = read_key();
		if (c == 27){
			close_window(win);
			KillTask(NULL);
		}
	}
}

void start_menu()
{
	cursor_pos_t cpos;
	Cursor_bounds cb;
	store_cursor_attributes(&cb, &cpos);

	set_task_state(1, TASK_BLOCKED);
	Window_t* win = open_window(0, 380, 200, 70, TOS_COLOR_DARK_GRAY, TOS_COLOR_YELLOW, "START");
	draw_rect(16, 410, 168, 24, 0x00ffffff);
	print_string_at("Type task name to run", 8, 396, 0x00ff0000);
	set_cursor(16, 418);
	set_cursor_bounds(16, 184, 418, 426);

	draw_rect(8, 458, 40, 14, TOS_COLOR_BLUE);
	outline_rect(8, 458, 40, 14, 2, TOS_COLOR_RED);
	print_string_at("START", 10, 461, TOS_COLOR_YELLOW);
	
	uint8_t char_buffer[32];
	int ptr = 0;
	
	while(1){
		char c = read_key();
		if (c == 27){
			set_task_state(1, TASK_READY);
			close_window(win);
			restore_cursor(&cb, &cpos);
			draw_rect(8, 458, 40, 14, TOS_COLOR_YELLOW);
			print_string_at("START", 10, 461, TOS_COLOR_BLUE);
			KillTask(NULL);	
		}
		else if (c == '\b'){
			if (ptr == 0) continue;
			char_buffer[--ptr] = '\0';
			draw_block((cursor_x - 8), cursor_y, 0x00ffffff);
			move_cursor(-1, 0);
		}
		else if (c == '\n'){
			if (kstrcmp(char_buffer, (uint8_t*)"shell") == 0){
				SpawnTask(shell_main, "shell");
			}
			set_task_state(1, TASK_READY);
			close_window(win);
			draw_rect(8, 458, 40, 14, TOS_COLOR_YELLOW);
			print_string_at("START", 10, 461, TOS_COLOR_BLUE);
			KillTask(NULL);	
		}
		else if (c >= 32){
			if (ptr > 31) continue;
			draw_char(c, 0x00000000);
			char_buffer[ptr++] = c;
		}
	}
}

void InitDesktop()
{
	clear_screen(TOS_COLOR_CYAN);
	draw_rect(0, 448, 640, 32, TOS_COLOR_WHITE);
	draw_rect(0, 448, 640, 2, TOS_COLOR_DARK_GRAY);

	draw_rect(8, 458, 40, 14, TOS_COLOR_YELLOW);
	print_string_at("START", 10, 461, TOS_COLOR_BLUE);

	while(1){
		char c = read_key();
		if (c == 0x5B){
			SpawnTask(start_menu, "start");
		}
	}
}

