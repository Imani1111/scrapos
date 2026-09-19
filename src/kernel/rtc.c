#include <rtc.h>
#include <idt.h>
#include <screen.h>
#include <string.h>

realtime_t systime = {0};

uint8_t check_update_in_progress_flag(void)
{
	send_byte_to_port(CMOS_ADDRESS, STATUS_A_REG);
	if (read_byte_from_port(CMOS_DATA) & 0x80){
		return IN_PROGRESS;
	}
	return UPDATE_CLEAR;
}

uint8_t read_RTC_reg(uint8_t rtc_reg)
{
	send_byte_to_port(CMOS_ADDRESS, rtc_reg);
	return (read_byte_from_port(CMOS_DATA));
}

void read_rtc()
{
	uint8_t last_second;
	uint8_t last_minute;
	uint8_t last_hour;
	uint8_t last_day;
	uint8_t last_month;
	uint8_t last_year;
	uint8_t registerB;

	while(check_update_in_progress_flag());
	systime.second = read_RTC_reg(SECONDS_REG);
	systime.minute = read_RTC_reg(MINUTES_REG);
	systime.hour = read_RTC_reg(HOUR_REG);
	systime.day = read_RTC_reg(DAY_OF_MONTH_REG);
	systime.month = read_RTC_reg(MONTH_REG);
	systime.year = read_RTC_reg(YEAR_REG);

	do {
		last_second = systime.second;
		last_minute = systime.minute;
		last_hour = systime.hour;
		last_day = systime.day;
		last_month = systime.month;
		last_year = systime.year;

		while (check_update_in_progress_flag());
		systime.second = read_RTC_reg(SECONDS_REG);
		systime.minute = read_RTC_reg(MINUTES_REG);
		systime.hour = read_RTC_reg(HOUR_REG);
		systime.day = read_RTC_reg(DAY_OF_MONTH_REG);
		systime.month = read_RTC_reg(MONTH_REG);
		systime.year = read_RTC_reg(YEAR_REG);

	}while(last_second != systime.second || last_minute != systime.minute || last_hour != systime.hour ||
			last_day != systime.day || last_month != systime.month || last_year != systime.year);

	registerB = read_RTC_reg(STATUS_B_REG);
	systime.second = ((systime.second >> 4) * 10) + (systime.second & 0xF);
	systime.minute = ((systime.minute >> 4) * 10) + (systime.minute & 0xF);
	systime.hour = ((systime.hour >> 4) * 10) + (systime.hour & 0xF) + 3;
	if (systime.hour > 23){
		systime.hour -= 24;
	}
	systime.day = ((systime.day >> 4) * 10) + (systime.day & 0xF);
	systime.month = ((systime.month >> 4) * 10) + (systime.month & 0xF);
	systime.year = (uint8_t)(CURRENT_YEAR - 2000);
	print_string("rtc read successfully......\n", 0x00ff);
}

realtime_t* get_current_timestamp()
{
	return &systime;
}

void format_time(realtime_t* t, char* b)
{
	char c[3];
	itoa(t->hour, c);
	if (c[0] == '0'){
		c[1] = '0';
	}else if (c[1] == '\0'){
		char t = c[0];
		c[0] = '0';
		c[1] = t;
	}
	kmemcpy(&b[0], c, 2);
	b[2] = ':';

	itoa(t->minute, c);
	if (c[0] == '0'){
		c[1] = '0';
	}else if (c[1] == '\0'){
		char t = c[0];
		c[0] = '0';
		c[1] = t;
	}
	kmemcpy(&b[3], c, 2);
	b[5] = ':';

	itoa(t->second, c);
	if (c[0] == 0){
		c[1] = '0';
	}else if (c[1] == '\0'){
		char t = c[0];
		c[0] = '0';
		c[1] = t;
	}
	kmemcpy(&b[6], c, 2);
	b[8] = ' ';

	itoa(t->day, c);
	if (c[1] == '\0'){
		char t = c[0];
		c[0] = '0';
		c[1] = t;
	}
	kmemcpy(&b[9], c, 2);
	b[11] = '/';
	itoa(t->month, c);
	if (c[1] == '\0'){
		char t = c[0];
		c[0] = '0';
		c[1] = t;
	}
	kmemcpy(&b[12], c, 2);
	b[14] = '/';
	itoa(t->year, c);
	kmemcpy(&b[15], c, 2);
	b[17] = '\0';
}

