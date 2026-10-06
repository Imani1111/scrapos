#include <pit.h>
#include <idt.h>
#include <screen.h>
#include <rtc.h>
#include <string.h>
#include <ui.h>

uint32_t pit_ticks = 0;

void init_pit(uint32_t preferred_freq)
{
	uint16_t divisor = PIT_FREQUENCY / preferred_freq;
	send_byte_to_port(0x43, 0x36);
	uint8_t low_divisor_byte = divisor & 0xFF;
	uint8_t high_divisor_byte = (divisor >> 8) & 0xFF;
	send_byte_to_port(0x40, low_divisor_byte);
	send_byte_to_port(0x40, high_divisor_byte);
}

void display_pit_ticks()
{
	draw_rect(10, 0, 32 * 8, 10, TOS_COLOR_CYAN);
	char buf[32];
	itoa(pit_ticks, buf);
	print_string_at("IRQ0(PIT): ", 10, 1, 0x00ff);
	print_string_at(buf, 98, 1, 0x00ff0000);
}

void PITInterruptHandler()
{
	pit_ticks++;
	update_clock();
	display_pit_ticks();
	cursor_blink();
	PIC_sendEOI(32);
}

void yield(){
	asm volatile("int $0x20");
}

void log_cpu_state(cpu_state_t* cpu_state)
{
	if ((pit_ticks % 50) == 0){
		draw_rect(12, 450, 616, 28, 0x00ffffff);

		print_string_at("eip->", 12, 450, 0x00ff0000);
		print_string_at("[", 52, 450, 0x00ff);
		print_hex_at(cpu_state->eip, 60, 450, 0x00cc0000);
		print_string_at("]", 140, 450, 0x00ff);
		print_string_at("esp->", 12, 458, 0x00ff0000);
		print_string_at("[", 52, 458, 0x00ff);
		print_hex_at(cpu_state->esp, 60, 458, 0x00cc0000);
		print_string_at("]", 140, 458, 0x00ff);
		print_string_at("eax->", 12, 466, 0x0);
		print_string_at("[", 52, 466, 0x00ff);
		print_hex_at(cpu_state->eax, 60, 466, 0x0);
		print_string_at("]", 140, 466, 0x00ff);
	
		print_string_at("ebx->", 156, 450, 0x0);
		print_string_at("[", 196, 450, 0x00ff);
		print_hex_at(cpu_state->ebx, 204, 450, 0x0);
		print_string_at("]", 284, 450, 0x00ff);

		print_string_at("eax->", 156, 458, 0x0);
		print_string_at("[", 196, 458, 0x00ff);
		print_hex_at(cpu_state->ecx, 204, 458, 0x0);
		print_string_at("]", 284, 458, 0x00ff);

		print_string_at("edx->", 156, 466, 0x0);
		print_string_at("[", 196, 466, 0x00ff);
		print_hex_at(cpu_state->edx, 204, 466, 0x0);
		print_string_at("]", 284, 466, 0x00ff);

		print_string_at("ebp->", 300, 450, 0x0);
		print_string_at("[", 340, 450, 0x00ff);
		print_hex_at(cpu_state->ebp, 348, 450, 0x0);
		print_string_at("]", 428, 450, 0x00ff);

		print_string_at("edi->", 300, 458, 0x0);
		print_string_at("[", 340, 458, 0x00ff);
		print_hex_at(cpu_state->edi, 348, 458, 0x0);
		print_string_at("]", 428, 458, 0x00ff);

		print_string_at("esi->", 300, 466, 0x0);
		print_string_at("[", 340, 466, 0x00ff);
		print_hex_at(cpu_state->esi, 348, 466, 0x0);
		print_string_at("]", 428, 466, 0x00ff);
	}
}
