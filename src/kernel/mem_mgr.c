#include <mem_mgr.h>
#include <mmap.h>
#include <screen.h>
#include <string.h>
#include <ui.h>
#include <pit.h>

static HeapBlockMetaData* heap_start = NULL;
static HeapBlockMetaData* heap_tail = NULL;
extern uint32_t first_allocatable_addr;
HeapBlockMetaData* heap_pointer = NULL;
uint32_t heap_current = HEAP_START;

int wait_time = 0;

void* my_malloc(uint32_t size)
{
	if (size == 0) return NULL;
	uint32_t aligned = (size + 3) & ~3;
	
	if (heap_start == NULL){
		heap_start = (HeapBlockMetaData*)sbrk(PAGE);
		heap_start->size = PAGE - sizeof(HeapBlockMetaData);
		heap_start->is_free = BLOCK_FREE;
		heap_start->next = heap_start;
		heap_start->prev = heap_start;

		heap_tail = heap_start;
		heap_pointer = heap_start;
	}

	HeapBlockMetaData* curr = heap_pointer;
	do {
		if (curr->is_free && (curr->size >= aligned + sizeof(HeapBlockMetaData))){
			HeapBlockMetaData* new = (HeapBlockMetaData*)((uint8_t*)curr + aligned + sizeof(HeapBlockMetaData));
			new->size = curr->size - aligned - sizeof(HeapBlockMetaData);
			new->is_free = BLOCK_FREE;
			
			new->next = curr->next;
			if (new->next == heap_start) heap_tail = new;		
			new->prev = curr;
			
			curr->next = new;
			curr->size = aligned;
			curr->is_free = BLOCK_OCCUPIED;

			heap_pointer = new;

			void* ptr = (uint8_t*)curr + sizeof(HeapBlockMetaData);
			return ptr;
		}
		curr = curr->next;
	}while (curr != heap_pointer);

	//TODO: Implement heap expansion using sbrk when no block fits
	if (heap_tail->is_free){
		uint32_t no_of_pages = ((aligned - heap_tail->size) + PAGE - 1) / PAGE;
		
		for (uint32_t i = 0; i < no_of_pages; i++){
			void* ptr = sbrk(PAGE);
			if (ptr == SBRK_FAIL){
				print_string("SBRK FAILED!\n", 0x00ff0000);
				return NULL;
			}
			heap_tail->size += PAGE;
		}
		
	}else{
		uint32_t no_of_pages = ((aligned + sizeof(HeapBlockMetaData)) + PAGE - 1) / PAGE;	
		HeapBlockMetaData* expansion = NULL;
		
		for (uint32_t i = 0; i < no_of_pages; i++){
			void* ptr = sbrk(PAGE);
			if (ptr == SBRK_FAIL){	
				print_string("SBRK FAILED!\n", 0x00ff0000);
				return NULL;
			}
		}
		expansion = (HeapBlockMetaData*)((uint8_t*)heap_tail + sizeof(HeapBlockMetaData) + heap_tail->size);
		expansion->size = (no_of_pages * PAGE) - sizeof(HeapBlockMetaData);
		expansion->is_free = BLOCK_FREE;
		expansion->next = heap_start;
		expansion->prev = heap_tail;
		heap_start->prev = expansion;
		heap_tail = expansion;
	}
	return my_malloc(aligned);
}

void my_free(void* ptr)
{
	if (!ptr) return;
		
	HeapBlockMetaData* block = (HeapBlockMetaData*)((uint8_t*)ptr - sizeof(HeapBlockMetaData));
	if (block->is_free) return; // Double free detected!
	
	block->is_free = BLOCK_FREE;
	
	if (block->next != heap_start && block->next->is_free){
		block->size += block->next->size + sizeof(HeapBlockMetaData);
		block->next = block->next->next;
		block->next->prev = block;

		heap_pointer = block;
	}
	if (block->prev != heap_tail && block->prev->is_free){
		heap_pointer = block->prev;
		block->prev->size += block->size + sizeof(HeapBlockMetaData);
		block->prev->next = block->next;
		block->next->prev = block->prev;
	}
}

void* sbrk(int32_t increment)
{
	if (increment == 0) return (void*)heap_current;
	uint32_t old = heap_current;
	uint32_t new = heap_current + increment;
	if (increment > 0){
		uint32_t current_block_end = (old + PAGE - 1) & ~(PAGE - 1);
		uint32_t target_block_end = (new + PAGE - 1) & ~(PAGE - 1);
		
		while (current_block_end < target_block_end){
			uint32_t phys_addr = pmm_alloc_page();
			
			uint32_t* addr = (uint32_t*)vmm_map_page(current_block_end, phys_addr, READ_WRITE);
			if (addr == MAP_FAILED){
			       	return SBRK_FAIL;
			}
			
			current_block_end += PAGE;
		}
	}
	heap_current = new;
	return (void*)old;
}

void draw_current_heap_data(){
	draw_rect(330, 20, 292, 70, 0x00ffffff);
	print_string_at("SBRK position: ", 330, 20, 0x00ff);
	print_hex_at(heap_current, 450, 20, 0x0000aa00);

	int total_space = heap_current - HEAP_START;
	char buf[10];
	itoa(total_space, buf);
	int len = kstrlen(buf);
	print_string_at("Current heap size: ", 330, 28, 0x00ff);
	print_string_at(buf, 482, 28, 0x00ff0000);
	print_string_at("Bytes", 482 + (len * 8), 28, 0);
			
	int free_chunks_total_bytes = 0;
	HeapBlockMetaData* t = heap_start;
	do {
		if (t->is_free){
			free_chunks_total_bytes += t->size;
		}
		t = t->next;
	}while(t != heap_start);
			
	itoa(free_chunks_total_bytes, buf);
	len = kstrlen(buf);
	print_string_at("Free: ", 330, 36, 0x00ff);
	print_string_at(buf, 378, 36, 0x0000aa00);
	print_string_at("Bytes", 378 + (len * 8), 36, 0x0);
	print_string_at("Heap pointer: ", 330, 44, 0x00ff);
	print_hex_at((uint32_t)heap_pointer, 442, 44, 0x0000aa00);
}

void display_heap_data(){
	draw_rect(326, 12, 300, 80, 0x00ffffff);
	outline_rect(326, 12, 300, 80, 3, 0x00ff);
	draw_rect(440, 10, 64, 10, 0x00ff);
	print_string_at("MEM_INSP", 440, 12, 0x00ffffff);
	
	draw_current_heap_data();
	while(1){
		wait_time++;
		if ((wait_time % 50) == 0){ 
			draw_current_heap_data();
		}else{
			yield();
		}
	}
}
