#include <shell.h>
#include <screen.h>
#include <ui.h>
#include <keyboard.h>
#include <task_mgr.h>
#include <fs.h>
#include <rtc.h>
#include <string.h>
#include <disk_mgr.h>
#include <ted.h>

extern int cursor_x;
extern int cursor_y;

char* cmd_toks[64];
char buf[256];

void test1(){
	Window_t* win = open_window(48, 80, 160, 40, TOS_COLOR_DARK_GRAY, 0x0, 0x00ffffff, "Test-1");
	reset_cursor_bounds();
	set_cursor(208, 116);
	set_cursor_bounds(208, 480, 116, 200);
	while(1){
		char c = read_key();
		if (c == 27){
			close_window(win);
			awake_parent(NULL);
			KillTask(NULL);
		}
	}
}

void test(){
	Window_t* win = open_window(200, 100, 400, 150, TOS_COLOR_CYAN, 0x0, 0x00ffffff, "Test");
	reset_cursor_bounds();
	set_cursor(208, 116);
	set_cursor_bounds(208, 480, 116, 200);
	while(1){
		char c = read_key();
		if (c == 27){
			close_window(win);
			awake_parent(NULL);
			KillTask(NULL);
		}else if (c == 's'){
			SpawnTask(test1, "SHELL");
			set_task_state(-1, TASK_BLOCKED);
		}
	}
}

void shell_main()
{	
	Window_t* win = open_window(80, 20, 480, 360, 0x00ffffff, 0x000000ff, 0x0, "Shell");
	reset_cursor_bounds();	
	set_cursor(88, 36);
	set_cursor_bounds(84, 556, 36, 376);

	print_shell_prompt();
	int ptr = 0;
	while (1){
		char c = read_key();
		if (c != 0){
			switch (c){
				case '\n':
					draw_char(c, 0);
					kstrtok(' ', buf, cmd_toks, 64);
					if (kstrcmp((uint8_t*)cmd_toks[0], (uint8_t*)"ls") == 0){
						ls();
					}else if (kstrcmp((uint8_t*)cmd_toks[0], (uint8_t*)"touch") == 0){
						if (!cmd_toks[1] || !cmd_toks[2]){
							print_string("Usage: touch -file <name> or -dir <name>\n", 0x00ff0000);
						}
						else if (kstrcmp((uint8_t*)cmd_toks[1], (uint8_t*)"-dir") == 0){
							int c = create_entry(cmd_toks[2], ATTR_DIRECTORY);
							if (c == FS_FULL){
								print_string("Error: File system full!\n", 0x00ff0000);
							}else if(c == DIR_FULL){
								print_string("Error: Current directory is full!\n", 0x00ff0000);
							}
						}else if (kstrcmp((uint8_t*)cmd_toks[1], (uint8_t*)"-file") == 0){
							int f = create_entry(cmd_toks[2], ATTR_FILE);
							if (f == FS_FULL){
								print_string("Error: File system full!\n", 0x00ff0000);
							}else if (f == DIR_FULL){
								print_string("Error: Current directory is full!\n", 0x00ff0000);
							}
						}
					}else if (kstrcmp((uint8_t*)cmd_toks[0], (uint8_t*)"cd") == 0){
						if (!cmd_toks[1]){
							print_string("Usage: cd <directory>\n", 0x00ff0000);
						}else{
							char path[64] = {0};
							kstrcpy(path, cmd_toks[1]);
							int cd_t = cd(path);
							if (cd_t == CACHE_ERR){
								print_string("CD failed!: cache error\n", 0x00ff0000);
							}else if (cd_t == CD_ENF){
								print_string("CD failed!: entry not found\n", 0x00ff0000);
							}else if(cd_t == -3){
								print_string("Specify search start directory", 0x00ff0000);
							}
						}
					}else if (kstrcmp((uint8_t*)cmd_toks[0], (uint8_t*)"ps") == 0){
						ls_tasks();
					}
					else if (kstrcmp((uint8_t*)cmd_toks[0], (uint8_t*)"ted") == 0){
						SpawnTask(ted_main, "TED");
						set_task_state(-1, TASK_BLOCKED);
					}
					else if (kstrcmp((uint8_t*)cmd_toks[0], (uint8_t*)"exit") == 0){
						close_window(win);
						awake_parent(NULL);
						KillTask(NULL);
					}
					kmemset((uint8_t*)cmd_toks, 0, sizeof(char*) * 64);
					kmemset(buf, 0, 256);
					ptr = 0;
					print_shell_prompt();
					break;
				case '\b':
					if (ptr > 0){
						ptr--;
						buf[ptr] = '\0';
					}
					break;
				default:
					buf[ptr++] = c;	
					draw_char(c, 0x0);
					break;
			}
		}
	}
}
