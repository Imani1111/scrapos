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
	Window_t* win = open_window(300, 40, 320, 200, 0x0, TOS_COLOR_RED, 0x00ffffff, "APPLAUNCHER");
	set_cursor(308, 56);
	set_cursor_bounds(308, 620, 48, 240);

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
					else if (kstrcmp((uint8_t*)buf, (uint8_t*)"test") == 0){
						SpawnTask(test, "TEST");
						set_task_state(-1, TASK_BLOCKED);
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

