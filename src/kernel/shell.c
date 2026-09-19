#include <shell.h>
#include <screen.h>
#include <ui.h>
#include <keyboard.h>
#include <task_mgr.h>
#include <fs.h>
#include <rtc.h>

extern int cursor_x;
extern int cursor_y;

void shell_main()
{
	//set_cursor_bounds(108, 492, 116, 392);
	//set_cursor(108, 96);
	
	//Window_t* win = open_window(100, 80, 400, 300, 0x00ffffff, 0x000000ff, "Shell");
	print_shell_prompt();
	while (1){
		char c = read_key();
		if (c == '\n'){
			print_string("\n", 0);
			print_shell_prompt();
		}else{
			draw_char(c, 0x0);
		}
	}
}
