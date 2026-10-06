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
#include <launcher.h>

extern uint32_t first_allocatable_addr;

void kernel_main()
{
	asm volatile("cli");
	clear_screen(0x00ffffff);
	reset_cursor_bounds();
	
	outline_rect(0, 0, 640, 480, 10, TOS_COLOR_CYAN);
	draw_rect(322, 10, 2, 438, 0x0);
	draw_rect(250, 440, 88, 10, 0x00ff);
	outline_rect(10, 448, 620, 32, 2, 0x00ff);
	print_string_at("STATUS_BAR", 254, 442, 0x00ffffff);
	
	draw_rect(326, 12, 300, 80, 0x00ffffff);
	outline_rect(326, 12, 300, 80, 3, 0x00ff);
	draw_rect(440, 10, 64, 10, 0x00ff);
	print_string_at("MEM_INSP", 440, 12, 0x00ffffff);

	outline_rect(326, 96, 300, 344, 3, 0x00ff);
	draw_rect(424, 94, 94, 14, 0x00ff);
	print_string_at("APPLAUNCHER", 426, 98, 0x00ffffff);

	read_rtc();
	realtime_t* time = get_current_timestamp();
	char disp[20];
	format_time(time, disp);

	print_string_at(disp, 484, 1, 0);

	set_cursor(16, 16);
	set_cursor_bounds(16, 624, 16, 464);
	init_fs();
	init_pmm();
	init_vmm();
	
	init_multitasking();
	SpawnTask(reaper, "TaskReaper");
	SpawnTask(display_heap_data, "MEM_INSP");
	SpawnTask(app_launcher, "AppLauncher");
		
	init_pit(100);
	init_idt();
	send_byte_to_port(0x21, 0b11111100); // Unmask PIT(bit 0) and Keyboard(bit 1)
	
	//shell_main();	
	while(1);
}

