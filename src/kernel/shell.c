#include <shell.h>
#include <screen.h>
#include <ui.h>
#include <keyboard.h>
#include <task_mgr.h>
#include <fs.h>
#include <rtc.h>
#include <string.h>

extern int cursor_x;
extern int cursor_y;

char* cmd_toks[64];
char buf[256];

void shell_main()
{	
	Window_t* win = open_window(100, 80, 400, 300, 0x00A9A9A9, 0x000000ff, "Shell");

	set_cursor_bounds(108, 492, 88, 372);
	set_cursor(108, 96);
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
							}
						}
					}else if (kstrcmp((uint8_t*)cmd_toks[0], (uint8_t*)"exit") == 0){
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
