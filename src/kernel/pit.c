#include <pit.h>
#include <idt.h>
#include <screen.h>
#include <rtc.h>

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

void PITInterruptHandler()
{
	pit_ticks++;
	update_clock();
	cursor_blink();
	PIC_sendEOI(32);
}
