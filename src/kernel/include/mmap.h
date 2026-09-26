#ifndef MMAP_H
#define MMAP_H

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;

typedef struct {
	uint64_t base_addr;
	uint64_t length;
	uint32_t type;
	uint32_t acpi_extended;
}__attribute__((packed)) MemoryMapEntry;

#define ENTRY_COUNT_PTR 0x500
#define MMAP_ENTRY_BUFFER 0x502

#define NO_OF_ENTRIES 1024
#define PAGE 4096
#define MAX_PAGES 262144
#define PAGE_PRESENT 0x1
#define READ_WRITE 0x2
#define USER_SUPERVISOR 0x4
#define CACHE_DISABLE 0x10

#define MAP_FAILED ((void*)-1)
#define MASTER_DIR_VIRTUAL_ADDR (0x3FF << 22)

typedef struct {
	uint32_t page_addresses[MAX_PAGES];
	uint32_t capacity;
	uint32_t stack_pointer;
}PMM_stack_alloc;

extern MemoryMapEntry* mmap;
extern uint16_t total_entries;
extern uint32_t first_allocatable_addr;

typedef struct {
	uint32_t page_table_entries[1024];
}__attribute__((aligned(4096))) page_table_t;

typedef struct {
	uint32_t page_directory_entries[1024];
}__attribute__((aligned(4096))) page_directory_t;


void ReadMemoryMap(void);
void init_pmm(void);
uint32_t pmm_alloc_page(void);
void pmm_free_page(uint32_t page_address);
uint32_t pmm_get_free_page_count(void);

void* vmm_map_page(uint32_t v_addr, uint32_t phys_addr, uint32_t flags);
void enable_paging(uint32_t page_dir_addr);
void init_identity_mapping(void);
void init_vmm(void);
#endif
