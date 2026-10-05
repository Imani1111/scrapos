#include <ted.h>
#include <file_io.h>
#include <ui.h>
#include <screen.h>
#include <keyboard.h>
#include <task_mgr.h>

Window_t* ted_win = NULL;

void ted_win_man(){
	ted_win = open_window(80, 80, 400, 300, 0xffffff, 0x0, 0x00ff, "TED");
	reset_cursor_bounds();
	set_cursor(88, 96);
	set_cursor_bounds(88, 492, 96, 380);
}

void handle_input(){
	char c = read_key();
	if (c == 0) return;
	if (c == 27){
		close_window(ted_win);
		awake_parent(NULL);
		KillTask(NULL);
	}else{
		draw_char(c, 0x00ff);
	}
}

void ted_main()
{
	//ted_win_man();
	while (1){
		handle_input();
	}
}
