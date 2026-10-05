#include <mem_mgr.h>
#include <mmap.h>
#include <screen.h>
#include <string.h>

static HeapBlockMetaData* heap_start = NULL;
static HeapBlockMetaData* heap_tail = NULL;
extern uint32_t first_allocatable_addr;
HeapBlockMetaData* heap_pointer = NULL;
uint32_t heap_current = HEAP_START;

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
	if ((uint32_t)block < HEAP_START || (uint32_t)block > HEAP_END) return; // Out of bound ram address detected!
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
