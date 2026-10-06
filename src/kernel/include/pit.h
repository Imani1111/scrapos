#ifndef PIT_H
#define PIT_H

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;

#define PIT_FREQUENCY 1193180

typedef struct {
	uint32_t eax;
	uint32_t ecx;
	uint32_t edx;
	uint32_t ebx;
	uint32_t esp;
	uint32_t ebp;
	uint32_t esi;
	uint32_t edi;
	uint32_t eip;
}__attribute__((packed)) cpu_state_t;

extern uint32_t pit_ticks;

void init_pit(uint32_t preferred_freq);
void PITInterruptHandler(void);
void log_cpu_state(cpu_state_t* cpu_state);
void yield(void);

#endif
