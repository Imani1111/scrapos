#ifndef RTC_H
#define RTC_H

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;

#define CMOS_ADDRESS 0x70
#define CMOS_DATA 0x71

#define UPDATE_CLEAR 0
#define IN_PROGRESS 1

#define SECONDS_REG 0x00
#define MINUTES_REG 0x02
#define HOUR_REG 0x04
#define WEEKDAY_REG 0x06
#define DAY_OF_MONTH_REG 0x07
#define MONTH_REG 0x08
#define YEAR_REG 0x09
#define CENTURY_REG 0x32
#define STATUS_A_REG 0x0A
#define STATUS_B_REG 0x0B

#define CURRENT_YEAR 2026

typedef struct {
	uint8_t second;
	uint8_t minute;
	uint8_t hour;
	uint8_t day;
	uint8_t month;
	uint8_t year;
}realtime_t;

void read_rtc(void);
realtime_t* get_current_timestamp(void);
void format_time(realtime_t* t, char* b);
void update_clock(void);

#endif
