#include <idt.h>
#include <keyboard.h>
#include <screen.h>

IDTEntry_t idt[IDT_ENTRIES];
IDTPtr_t idt_addr;

extern void LoadIDT(uint32_t idtptr);
extern void isr13();
extern void isr32();
extern void isr33();

void send_byte_to_port(uint16_t port, uint8_t val){
	asm volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

uint8_t read_byte_from_port(uint16_t port){
	uint8_t result;
	asm volatile("inb %1, %0" : "=a"(result) : "Nd"(port));
	return result;
}

uint16_t read_word_from_port(uint16_t port){
	uint16_t result;
	asm volatile("inw %1, %0" : "=a"(result) : "d"(port));
	return result;
}

void send_word_to_port(uint16_t port, uint16_t val){
	asm volatile("outw %0, %1" : : "a"(val), "Nd"(port));
}

static inline void io_wait(void){
	send_byte_to_port(0x80, 0);
}

static inline void RemapPIC(void){
	send_byte_to_port(PIC1_COMMAND, 0x11);
	io_wait();
	send_byte_to_port(PIC2_COMMAND, 0x11);
	io_wait();
	send_byte_to_port(PIC1_DATA, 0x20);
	io_wait();
	send_byte_to_port(PIC2_DATA, 0x28);
	io_wait();
	send_byte_to_port(PIC1_DATA, 0x04);
	io_wait();
	send_byte_to_port(PIC2_DATA, 0x02);
	io_wait();
	send_byte_to_port(PIC1_DATA, 0x01);
	io_wait();
	send_byte_to_port(PIC2_DATA, 0x01);
	io_wait();
	send_byte_to_port(PIC1_DATA, 0);
	send_byte_to_port(PIC2_DATA, 0);
}

void PIC_sendEOI(uint8_t irqno){
	if (irqno >= 40){
		send_byte_to_port(PIC2_COMMAND, PIC_EOI);
	}
	send_byte_to_port(PIC1_COMMAND, PIC_EOI);
}

void CreateIDTEntry(IDTEntry_t* table, uint32_t israddr, uint16_t segselector, uint8_t flags)
{
	table->israddr_low = (israddr & 0xFFFF);
	table->segment_selector = segselector;
	table->reserved = 0;
	table->typeattr = flags;
	table->israddr_high = (israddr >> 16) & 0xFFFF;
}

void gp_fault()
{
	clear_screen(0x00ff0000);
	while (1){
		asm volatile("cli; hlt");
	}
}

void init_idt(){
	idt_addr.limit = (sizeof(IDTEntry_t) * IDT_ENTRIES) - 1;
	idt_addr.idtaddr = (uint32_t)&idt;

	for (uint16_t i = 0; i < IDT_ENTRIES; i++){
		CreateIDTEntry(&idt[i], 0, 0, 0);
	}
	
	//CreateIDTEntry(&idt[13], (uint32_t)isr13, GDT_CODE_SEGMENT, IDT_FLAG_INTERRUPT_GATE);	
	CreateIDTEntry(&idt[33], (uint32_t)isr33, GDT_CODE_SEGMENT, IDT_FLAG_INTERRUPT_GATE);
	CreateIDTEntry(&idt[32], (uint32_t)isr32, GDT_CODE_SEGMENT, IDT_FLAG_INTERRUPT_GATE);

	RemapPIC();
	LoadIDT((uint32_t)&idt_addr);
	asm volatile("sti");
}
