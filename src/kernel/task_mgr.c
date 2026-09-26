#include <task_mgr.h>
#include <mem_mgr.h>
#include <screen.h>
#include <idt.h>
#include <string.h>
#include <ui.h>

TaskControlBlock_t* head_task = NULL;
TaskControlBlock_t* tail_task = NULL;
TaskControlBlock_t* current_task = NULL;
int next_pid = 1;

void kernel_idle_loop(){
	while(1){
		asm volatile("hlt");
	}
}


void init_multitasking()
{
	TaskControlBlock_t* kernel_task = (TaskControlBlock_t*)my_malloc(sizeof(TaskControlBlock_t));
	if (kernel_task == NULL){
		print_string_at("Task Init Failed!", 0, 0, 0x00FF0000);
		while (1){
			asm volatile("cli; hlt");
		}
	}
	uint8_t* stack_base = (uint8_t*)my_malloc(TASK_STACK_SPACE);	
	uint32_t* stack_pointer = (uint32_t*)((uint8_t*)stack_base + TASK_STACK_SPACE);
	
	*(--stack_pointer) = EFLAGS;
	*(--stack_pointer) = GDT_CODE_SEGMENT;
	*(--stack_pointer) = (uint32_t)kernel_idle_loop;

	*(--stack_pointer) = 0;
	*(--stack_pointer) = 0;
	*(--stack_pointer) = 0;
	*(--stack_pointer) = 0;
	*(--stack_pointer) = 0;
	*(--stack_pointer) = 0;
	*(--stack_pointer) = 0;
	*(--stack_pointer) = 0;
	
	kernel_task->esp = (uint32_t)stack_pointer;
	kernel_task->base = (uint32_t*)stack_base;
	kernel_task->pid = 0;
	char* name = "Kernel_Task";
	kstrcpy(kernel_task->name, name);
	kernel_task->state = TASK_READY;
	kernel_task->parent_pid = 0;
	kernel_task->child_count = 0;
	kmemset(kernel_task->children, 0, sizeof(int) * 16);
	
	head_task = kernel_task;
	tail_task = kernel_task;
	head_task->next = tail_task;
	head_task->prev = tail_task;
	tail_task->next = head_task;
	tail_task->prev = head_task;
	current_task = kernel_task;
}

TaskControlBlock_t* SpawnTask(void(*entry_point)(void), const char* name)
{
	TaskControlBlock_t* new_task = (TaskControlBlock_t*)my_malloc(sizeof(TaskControlBlock_t));
	if (new_task == NULL){
		return TASK_CREAT_FAILED;
	}
	uint8_t* stack_base = (uint8_t*)my_malloc(TASK_STACK_SPACE);
	uint32_t* stack_pointer = (uint32_t*)((uint8_t*)stack_base + TASK_STACK_SPACE);
	
	*(--stack_pointer) = EFLAGS;
	*(--stack_pointer) = GDT_CODE_SEGMENT;
	*(--stack_pointer) = (uint32_t)entry_point;

	*(--stack_pointer) = 0;
	*(--stack_pointer) = 0;
	*(--stack_pointer) = 0;
	*(--stack_pointer) = 0;
	*(--stack_pointer) = 0;
	*(--stack_pointer) = 0;
	*(--stack_pointer) = 0;
	*(--stack_pointer) = 0;

	new_task->base = (uint32_t*)stack_base;
	new_task->esp = (uint32_t)stack_pointer;
	new_task->pid = next_pid++;
	new_task->state = TASK_READY;
	kstrcpy(new_task->name, name);
	new_task->parent_pid = current_task->pid;
	new_task->child_count = 0;
	kmemset(new_task->children, 0, sizeof(int) * 16);
	
	if (current_task->child_count >= 4){
		my_free(new_task);
		my_free(stack_base);
		return TASK_CREAT_FAILED;
	}

	current_task->child_count++;
	for (int i = 0; i < 16; i++){
		if (current_task->children[i] == 0){
			current_task->children[i] = new_task->pid;
			break;
		}
	}

	new_task->next = head_task;
	new_task->prev = tail_task;
	tail_task->next = new_task;
	head_task->prev = new_task;
	tail_task = new_task;

	return new_task;	
}

uint32_t Schedule(uint32_t current_esp)
{
	if (!current_task) return current_esp;
	current_task->esp = current_esp;

	if (current_task->state == TASK_RUNNING){
		current_task->state = TASK_READY;
	}

	TaskControlBlock_t* start_task = current_task;	
	TaskControlBlock_t* next_task = current_task->next;
	int found = 0;
	do {
		if (next_task->state == TASK_READY){
			found = 1;
			break;
		}
		next_task = next_task->next;
	}while (next_task != start_task);
	
	if (!found){
		next_task = current_task;
	}
		
	next_task->state = TASK_RUNNING;
	current_task = next_task;
	return next_task->esp;
}

void KillTask(TaskControlBlock_t* task)
{
	if (task == NULL){
		int parent_pid = current_task->parent_pid;
		TaskControlBlock_t* parent_task = current_task->next;
		while (parent_task != current_task){
			if (parent_task->pid == parent_pid){
				parent_task->state = TASK_READY;
				parent_task->child_count--;
				for (int i = 0; i < 16; i++){
					if (parent_task->children[i] == current_task->pid){
						parent_task->children[i] = 0;
						break;
					}
				}
				break;
			}
			parent_task = parent_task->next;
		}
		current_task->state = TASK_ZOMBIE;
	}
	else if (task){
		TaskControlBlock_t* target_task = task;

		int parent_pid = target_task->parent_pid;
		TaskControlBlock_t* parent_task = head_task;
		do {
			if (parent_task->pid== parent_pid){
				parent_task->state = TASK_READY;
				parent_task->child_count--;
				for (int i = 0; i < 16; i++){
					if (parent_task->children[i] == target_task->pid){
						parent_task->children[i] = 0;
						break;
					}
				}
				break;
			}
			parent_task = parent_task->next;
		}while(parent_task != head_task);
		
		target_task->state = TASK_ZOMBIE;
	}

	asm volatile("int $0x20");
}

void reaper()
{
	while (1){
		TaskControlBlock_t* task = head_task;
		do {
			if (task->state == TASK_ZOMBIE){
				asm volatile("cli");

				// unlink the tcb and free its stack and tcb
				TaskControlBlock_t* next_task = task->next;
				task->next->prev = task->prev;
				task->prev->next = task->next;
				if (task == tail_task){
					tail_task = task->prev;
				}
				my_free(task->base);
				my_free(task);

				task = next_task;
				asm volatile("sti");
				break;
			}

			task = task->next;
		}while (task != NULL && task != head_task);
	}
}

void set_task_state(int pid, TaskState state)
{
	TaskControlBlock_t* temp = head_task;
	int task_pid;
	if (pid == -1){
		task_pid = current_task->pid;
	}
	else{
		task_pid = pid;
	}

	do {
		if (temp->pid == task_pid){
			temp->state = state;
			break;
		}
		temp = temp->next;
	}while(temp != head_task);
}

void awake_parent(TaskControlBlock_t* task)
{
	int parent_pid;
	if (task == NULL){
		parent_pid = current_task->parent_pid;
	}else{
		parent_pid = task->parent_pid;
	}
	set_task_state(parent_pid, TASK_READY);
}

void ls_tasks()
{
	print_string("PID ", 0x00800080);
	print_string("ESP_VAL  ", 0x00800080);
	print_string("CC ", 0x00800080);
	print_string("STATE ", 0x00800080);
	print_string("TASK_NAME", 0x00800080);
	draw_char('\n', 0);

	TaskControlBlock_t* t = head_task;
	char buf[4];
	do{
		itoa(t->pid, buf);
		print_string(buf, 0x0);
		print_string(" ", 0);
		print_hex(t->esp, TOS_COLOR_RED);
		print_string(" ", 0);
		itoa(t->child_count, buf);
		print_string(buf, 0x00ffff00);
		print_string("  ", 0);
		if (t->state == TASK_RUNNING){
			print_string("t_run  ", 0x0000ff00);
		}else if (t->state == TASK_READY){
			print_string("t_rdy  ", 0x00ff);
		}else if (t->state == TASK_BLOCKED){
			print_string("t_blkd ", 0x00ffff00);
		}else if (t->state == TASK_ZOMBIE){
			print_string("t_zomb ", 0x00ff0000);
		}
		print_string(t->name, 0x00008000);
		print_string("\n", 0);

		t = t->next;
	}while (t != NULL && t != head_task);
}


