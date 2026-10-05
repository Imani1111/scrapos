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

	draw_rect(0, 448, 640, 32, TOS_COLOR_DARK_GRAY);
	draw_rect(0, 448, 640, 2, TOS_COLOR_DARK_GRAY);
	outline_rect(0, 0, 640, 480, 10, TOS_COLOR_CYAN);
	read_rtc();
	realtime_t* time = get_current_timestamp();
	char disp[20];
	format_time(time, disp);

	print_string_at("SCRAP_OPERATING_SYSTEM", 240, 1, 0x0);
	print_string_at(disp, 484, 1, 0);

	set_cursor(16, 16);
	set_cursor_bounds(16, 624, 16, 464);
	init_fs();
	init_pmm();
	init_vmm();
	
	init_multitasking();
	SpawnTask(reaper, "TaskReaper");
	SpawnTask(app_launcher, "AppLauncher");
		
	init_pit(100);
	init_idt();
	send_byte_to_port(0x21, 0b11111100); // Unmask PIT(bit 0) and Keyboard(bit 1)
	
	//shell_main();	
	while(1);
}

