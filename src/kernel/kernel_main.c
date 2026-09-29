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

