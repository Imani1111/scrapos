#include <launcher.h>
#include <ui.h>
#include <screen.h>
#include <keyboard.h>
#include <fs.h>
#include <mem_mgr.h>
#include <shell.h>
#include <rtc.h>
#include <string.h>
#include <task_mgr.h>


void app_launcher(void)
{	
	//clear_screen(TOS_COLOR_CYAN);
	//draw_rect(0, 448, 640, 32, TOS_COLOR_WHITE);
	//draw_rect(0, 448, 640, 2, TOS_COLOR_DARK_GRAY);
	//
	outline_rect(0, 0, 640, 480, 10, TOS_COLOR_CYAN);
	read_rtc();
	realtime_t* time = get_current_timestamp();
	char disp[20];
	format_time(time, disp);
	
	print_string_at("SCRAP_OPERATING_SYSTEM", 240, 1, 0);
	print_string_at(disp, 484, 1, 0);

	Window_t* win = open_window(24, 40, 320, 200, 0x0, TOS_COLOR_RED, 0x00ffffff, "APPLAUNCHER");
	set_cursor(32, 56);
	set_cursor_bounds(32, 320, 48, 240);

	print_string("ALCH$", 0x0000ffff);
	char buf[32] = {0};
	int i = 0;
	while(1){
		char c = read_key();
		if (c != 0){
			switch(c){
				case '\n':
					draw_char(c, 0);
					if (kstrcmp((uint8_t*)buf, (uint8_t*)"shell") == 0){
						TaskControlBlock_t* shell = SpawnTask(shell_main, "Shell");
						if (shell == TASK_CREAT_FAILED){
							print_string("Task creation failed!\n", 0x00ff0000);
						}else{
							set_task_state(-1, TASK_BLOCKED);
						}
					}
					i = 0;
					kmemset(buf, 0, sizeof(buf));
					print_string("ALCH$", 0x0000ffff);
					break;
				case '\b':
					if (i == 0) continue;
					break;
				default:
					if (i > 31) continue;
					draw_char(c, 0x00ffffff);
					buf[i++] = c;
					break;
			}
		}
	}
}

