#include <mmap.h>
#include <screen.h>
#include <string.h>
#include <ui.h>

extern uint8_t _kernel_end[];
PMM_stack_alloc* pmm_stack = (PMM_stack_alloc*)0x00200000;

page_directory_t pd = {0};
extern int cursor_x;
extern int cursor_y;

page_directory_t boot_page_dir = {0};
page_table_t boot_page_t = {0};
page_table_t fb = {0};
page_table_t temp_heap = {0};
page_table_t pt = {0};

void init_pmm()
{
	MemoryMapEntry* mmap = (MemoryMapEntry*)MMAP_ENTRY_BUFFER;
	uint16_t* entry_count = (uint16_t*)ENTRY_COUNT_PTR;
	uint16_t total_entries = *entry_count;
	
	kmemset((uint8_t*)pmm_stack, 0, sizeof(uint32_t) * MAX_PAGES);
	pmm_stack->capacity = MAX_PAGES;
	pmm_stack->stack_pointer = 0;

	for (uint16_t i = 0; i < total_entries; i++){
		if (mmap[i].type != 1){
			continue;
		}

		uint32_t usable_memory_region = (uint32_t)mmap[i].base_addr;
		uint32_t size = mmap[i].length;
		uint32_t region_end = (uint32_t)usable_memory_region + size;

		print_string("Usable Ram: ", 0x00ff);
		print_hex(usable_memory_region, 0x00ff0000);
		print_string(" - ", 0x00ff0000);
		print_hex(region_end, 0x00ff0000);
		print_string("\n", 0);

		usable_memory_region = (usable_memory_region + (PAGE- 1)) & ~(PAGE - 1);
			
		while(usable_memory_region < region_end){
			if (usable_memory_region < 0x400000){
				usable_memory_region += PAGE;
			       	continue;
			}
			if (pmm_stack->stack_pointer >= pmm_stack->capacity) break;
			pmm_stack->page_addresses[pmm_stack->stack_pointer++] = usable_memory_region;
			usable_memory_region += PAGE;
		}
	}
	char buf[10];
	draw_rect(cursor_x, cursor_y, 32 * 8, 8, 0);
	print_string("<<PMM_STACK_PARTIAL_DEBUG_DUMP>>\n", 0x0000FF00);
	for (int i = 1; i < 50; i++){
		print_string("Physical Address: ", 0x00ff);
		print_hex(pmm_stack->page_addresses[pmm_stack->stack_pointer - i], 0x00ff0000);
		print_string("->", 0x00ff);
		draw_char('[', 0x00ff);
		itoa(pmm_stack->stack_pointer - i, buf);
		print_string(buf, 0x00ff0000);
		draw_char(']', 0x00ff);
		draw_char('\n', 0);
	}
}

uint32_t pmm_alloc_page()
{
	if (pmm_stack->stack_pointer == 0){
		return 0;
	}
	uint32_t page = pmm_stack->page_addresses[--pmm_stack->stack_pointer];
	return page; 
}

void pmm_free_page(uint32_t page_address)
{
	if (pmm_stack->stack_pointer <= pmm_stack->capacity){
		pmm_stack->page_addresses[pmm_stack->stack_pointer++] = page_address;
	}
}

uint32_t pmm_get_free_page_count()
{
	return pmm_stack->stack_pointer;
}

void* vmm_map_page(uint32_t virtual_addr, uint32_t phys_addr, uint32_t flags)
{
	uint32_t pd_index = (virtual_addr >> 22) & 0x3FF;
	uint32_t pt_index = (virtual_addr >> 12) & 0x3FF;

	uint32_t pd_entry = boot_page_dir.page_directory_entries[pd_index];
	uint32_t pt_phys;
	page_table_t* pt;

	if (!(pd_entry & PAGE_PRESENT)){
		pt_phys = pmm_alloc_page();
		boot_page_dir.page_directory_entries[pd_index] = (uint32_t)pt_phys | PAGE_PRESENT | flags;
		
		uint32_t recursive_pt_virt = MASTER_DIR_VIRTUAL_ADDR + (pd_index << 12);
		asm volatile("invlpg (%0)" : : "r" (recursive_pt_virt) : "memory");
			
		// The page directory entry index is added to the page table bits because im using 
		// page table 1023 as the map of my whole master directory so the page table entries of 1023 
		// are the physical addresses of the page directory entries
		pt = (page_table_t*)(MASTER_DIR_VIRTUAL_ADDR + (pd_index << 12));
		kmemset((void*)pt, 0, sizeof(page_table_t));
	}
	else{
		pt = (page_table_t*)(MASTER_DIR_VIRTUAL_ADDR + (pd_index << 12));
	}
	pt->page_table_entries[pt_index] = phys_addr | PAGE_PRESENT | flags;
	asm volatile ("invlpg (%0)" : : "r" (virtual_addr) : "memory");
	return (void*)virtual_addr;
}

void enable_paging(uint32_t page_directory_address)
{
	asm volatile(
		"mov %0, %%cr3\n\t"
		"mov %%cr0, %%eax\n\t"
		"or $0x80000000, %%eax\n\t"
		"mov %%eax, %%cr0\n\t"
		: : "r" (page_directory_address) : "eax", "memory"
	);
}

void init_identity_mapping()
{
	for (int i = 0; i < NO_OF_ENTRIES; i++){
		uint32_t physical_addr = 0x1000 * i;
		boot_page_t.page_table_entries[i] = physical_addr | PAGE_PRESENT | USER_SUPERVISOR;
	}
	boot_page_dir.page_directory_entries[0] = ((uint32_t)boot_page_t.page_table_entries) | PAGE_PRESENT | READ_WRITE | 
		USER_SUPERVISOR;

	uint32_t base_addr = 0xfd000000;
	uint32_t size = 640 * 480 * 3;
	uint32_t pd_index  = base_addr >> 22;
	
	uint32_t no_of_pages = (size + PAGE - 1) / PAGE;	
	for (uint32_t j = 0; j < no_of_pages; j++){
		uint32_t page_phys = base_addr + (j * PAGE);
		fb.page_table_entries[j] = page_phys | PAGE_PRESENT | READ_WRITE;
	}
	boot_page_dir.page_directory_entries[pd_index] = ((uint32_t)fb.page_table_entries) | PAGE_PRESENT | READ_WRITE;	
}

void init_recursive_mapping(){
	boot_page_dir.page_directory_entries[1023] = (uint32_t)&boot_page_dir | PAGE_PRESENT | READ_WRITE;
}

void load_master_dir()
{
	uint32_t master_dir = (uint32_t)&pd;
	asm volatile("mov %0, %%cr3" :: "r" (master_dir) : "memory");
}
void init_vmm()
{
	init_identity_mapping();
	init_recursive_mapping();
	enable_paging((uint32_t)&boot_page_dir);
	//load_master_dir();
}

