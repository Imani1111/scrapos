#include <screen.h>
#include <idt.h>
#include <keyboard.h>
#include <pit.h>
#include <ui.h>
#include <task_mgr.h>
#include <mem_mgr.h>
#include <disk_mgr.h>
#include <string.h>
#include <shell.h>
#include <fs.h>
#include <mmap.h>
#include <rtc.h>

extern uint32_t first_allocatable_addr;
void kernel_main()
{
	asm volatile("cli");
	clear_screen(0x00ffffff);
	set_cursor_bounds(0, 640, 0, 480);
		
	init_fs();
	init_pmm();
	init_vmm();
		
	read_rtc();
	realtime_t* time = get_current_timestamp();
	char disp[20];
	format_time(time, disp);	
	print_string(disp, 0xff);
	print_string("\n\n", 0);	
	
	//init_multitasking();
	//SpawnTask(InitDesktop, "Desktop");
		
	init_pit(100);
	init_idt();
	send_byte_to_port(0x21, 0b11111100); // Unmask PIT(bit 0) and Keyboard(bit 1)
	
	//shell_main();	
	print_shell_prompt();
	while(1){
		char c = read_key();
		if (c == '\n'){
			draw_char(c, 0);
			print_shell_prompt();
		}
		else {
			draw_char(c, 0x0);
		}
	}
}

